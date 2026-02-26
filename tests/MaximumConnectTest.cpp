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
#include "MessageIdentifiers.h"
#include "RakSleep.h"

using namespace RakNet;
using Clock = std::chrono::steady_clock;

static constexpr int NUM_PEERS = 8;
static constexpr int MAX_CONNECTIONS = 4;

/**
 * @brief Tests that SetMaximumIncomingConnections is enforced when many peers
 * all try to connect to each other beyond the allowed limit.
 */
class MaximumConnectTest : public ::testing::Test {
protected:
    RakPeerInterface *peers[NUM_PEERS] = {};
    unsigned short ports[NUM_PEERS] = {};

    void SetUp() override {
        for (int i = 0; i < NUM_PEERS; i++) {
            peers[i] = RakPeerInterface::GetInstance();
            ASSERT_NE(peers[i], nullptr);

            SocketDescriptor sd(0, nullptr);
            ASSERT_EQ(peers[i]->Startup(MAX_CONNECTIONS, &sd, 1), RAKNET_STARTED);
            peers[i]->SetMaximumIncomingConnections(MAX_CONNECTIONS);
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

TEST_F(MaximumConnectTest, GetMaximumIncomingConnections) {
    for (int i = 0; i < NUM_PEERS; i++) {
        EXPECT_EQ(peers[i]->GetMaximumIncomingConnections(), MAX_CONNECTIONS)
            << "Peer " << i << " reports wrong max connections";
    }
}

TEST_F(MaximumConnectTest, ExcessConnectionsRefused) {
    // Every peer tries to connect to every other peer
    for (int i = 0; i < NUM_PEERS; i++) {
        for (int j = i + 1; j < NUM_PEERS; j++) {
            ASSERT_EQ(peers[i]->Connect("127.0.0.1", ports[j], nullptr, 0),
                      CONNECTION_ATTEMPT_STARTED)
                << "Connect call failed for peer " << i << " -> " << j;
        }
    }

    // Wait for connections to settle
    auto deadline = Clock::now() + std::chrono::seconds(10);
    while (Clock::now() < deadline) {
        for (int i = 0; i < NUM_PEERS; i++) {
            for (Packet *p = peers[i]->Receive(); p;
                 peers[i]->DeallocatePacket(p), p = peers[i]->Receive()) {
            }
        }
        RakSleep(30);
    }

    // Verify no peer has more connections than allowed
    DataStructures::List<SystemAddress> systemList;
    DataStructures::List<RakNetGUID> guidList;

    for (int i = 0; i < NUM_PEERS; i++) {
        peers[i]->GetSystemList(systemList, guidList);
        int connCount = guidList.Size();
        EXPECT_LE(connCount, MAX_CONNECTIONS)
            << "Peer " << i << " has " << connCount
            << " connections, max allowed is " << MAX_CONNECTIONS;
    }
}
