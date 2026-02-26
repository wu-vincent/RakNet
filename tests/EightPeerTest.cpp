/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <chrono>

#include <gtest/gtest.h>

#include "RakPeerInterface.h"
#include "BitStream.h"
#include "MessageIdentifiers.h"
#include "RakSleep.h"

using namespace RakNet;
using Clock = std::chrono::steady_clock;

static constexpr int NUM_PEERS = 8;
static constexpr int NUM_PACKETS = 100;

/**
 * @brief Tests 8 peers fully connected in a mesh, each sending 100 reliable
 * ordered packets to all others, verifying delivery and ordering.
 */
class EightPeerTest : public ::testing::Test {
protected:
    RakPeerInterface *peers[NUM_PEERS] = {};
    unsigned short ports[NUM_PEERS] = {};

    void SetUp() override {
        for (int i = 0; i < NUM_PEERS; i++) {
            peers[i] = RakPeerInterface::GetInstance();
            ASSERT_NE(peers[i], nullptr);

            SocketDescriptor sd(0, nullptr);
            ASSERT_EQ(peers[i]->Startup(NUM_PEERS * 2, &sd, 1), RAKNET_STARTED);
            peers[i]->SetMaximumIncomingConnections(NUM_PEERS);
            ports[i] = peers[i]->GetMyBoundAddress().GetPort();
        }
    }

    void TearDown() override {
        for (int i = 0; i < NUM_PEERS; i++) {
            if (peers[i])
                RakPeerInterface::DestroyInstance(peers[i]);
        }
    }
};

TEST_F(EightPeerTest, FullMeshReliableOrdered) {
    // Connect all peers to each other
    for (int i = 0; i < NUM_PEERS; i++) {
        for (int j = i + 1; j < NUM_PEERS; j++) {
            ASSERT_EQ(peers[i]->Connect("127.0.0.1", ports[j], nullptr, 0),
                      CONNECTION_ATTEMPT_STARTED)
                << "Connect failed for peer " << i << " -> " << j;
        }
    }

    // Wait for all peers to be fully connected (each should have NUM_PEERS-1 connections)
    int connectionCount[NUM_PEERS] = {};
    auto deadline = Clock::now() + std::chrono::seconds(20);
    bool allConnected = false;

    while (Clock::now() < deadline && !allConnected) {
        for (int i = 0; i < NUM_PEERS; i++) {
            for (Packet *p = peers[i]->Receive(); p;
                 peers[i]->DeallocatePacket(p), p = peers[i]->Receive()) {
                if (p->data[0] == ID_CONNECTION_REQUEST_ACCEPTED ||
                    p->data[0] == ID_NEW_INCOMING_CONNECTION)
                    connectionCount[i]++;
                else if (p->data[0] == ID_CONNECTION_ATTEMPT_FAILED)
                    FAIL() << "Peer " << i << " got CONNECTION_ATTEMPT_FAILED";
                else if (p->data[0] == ID_NO_FREE_INCOMING_CONNECTIONS)
                    FAIL() << "Peer " << i << " got NO_FREE_INCOMING_CONNECTIONS";
            }
        }

        allConnected = true;
        for (int i = 0; i < NUM_PEERS; i++) {
            if (connectionCount[i] < NUM_PEERS - 1)
                allConnected = false;
        }
        RakSleep(30);
    }
    ASSERT_TRUE(allConnected) << "Not all peers fully connected within timeout";

    // Each peer sends NUM_PACKETS messages to all others
    int receivedFrom[NUM_PEERS][NUM_PEERS] = {};
    int expectedSeq[NUM_PEERS][NUM_PEERS] = {};

    for (int k = 0; k < NUM_PACKETS; k++) {
        for (int i = 0; i < NUM_PEERS; i++) {
            BitStream bs;
            bs.Write(static_cast<unsigned char>(ID_USER_PACKET_ENUM + 1));
            bs.Write(k);
            bs.Write(i);
            peers[i]->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0,
                           UNASSIGNED_SYSTEM_ADDRESS, true);
        }
        RakSleep(5);

        // Drain while sending to avoid buffer buildup
        for (int i = 0; i < NUM_PEERS; i++) {
            for (Packet *p = peers[i]->Receive(); p;
                 peers[i]->DeallocatePacket(p), p = peers[i]->Receive()) {
                if (p->data[0] == ID_USER_PACKET_ENUM + 1) {
                    BitStream bs(p->data, p->length, false);
                    bs.IgnoreBytes(1);
                    int seq, sender;
                    bs.Read(seq);
                    bs.Read(sender);
                    if (sender >= 0 && sender < NUM_PEERS) {
                        EXPECT_EQ(seq, expectedSeq[i][sender])
                            << "Out of order: peer " << i << " from sender " << sender;
                        expectedSeq[i][sender] = seq + 1;
                        receivedFrom[i][sender]++;
                    }
                } else if (p->data[0] == ID_DISCONNECTION_NOTIFICATION ||
                           p->data[0] == ID_CONNECTION_LOST) {
                    FAIL() << "Peer " << i << " lost a connection during send phase";
                }
            }
        }
    }

    // Drain remaining packets
    auto recvDeadline = Clock::now() + std::chrono::seconds(5);
    while (Clock::now() < recvDeadline) {
        bool allDone = true;
        for (int i = 0; i < NUM_PEERS; i++) {
            for (Packet *p = peers[i]->Receive(); p;
                 peers[i]->DeallocatePacket(p), p = peers[i]->Receive()) {
                if (p->data[0] == ID_USER_PACKET_ENUM + 1) {
                    BitStream bs(p->data, p->length, false);
                    bs.IgnoreBytes(1);
                    int seq, sender;
                    bs.Read(seq);
                    bs.Read(sender);
                    if (sender >= 0 && sender < NUM_PEERS) {
                        EXPECT_EQ(seq, expectedSeq[i][sender])
                            << "Out of order: peer " << i << " from sender " << sender;
                        expectedSeq[i][sender] = seq + 1;
                        receivedFrom[i][sender]++;
                    }
                }
            }
            for (int j = 0; j < NUM_PEERS; j++) {
                if (i != j && receivedFrom[i][j] < NUM_PACKETS)
                    allDone = false;
            }
        }
        if (allDone) break;
        RakSleep(30);
    }

    // Verify all packets received
    for (int i = 0; i < NUM_PEERS; i++) {
        for (int j = 0; j < NUM_PEERS; j++) {
            if (i != j) {
                EXPECT_EQ(receivedFrom[i][j], NUM_PACKETS)
                    << "Peer " << i << " received " << receivedFrom[i][j]
                    << "/" << NUM_PACKETS << " from peer " << j;
            }
        }
    }
}
