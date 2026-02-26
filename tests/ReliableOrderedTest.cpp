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
#include <random>

#include <gtest/gtest.h>

#include "RakPeerInterface.h"
#include "BitStream.h"
#include "MessageIdentifiers.h"
#include "RakSleep.h"

using namespace RakNet;
using Clock = std::chrono::steady_clock;

static constexpr int NUM_CHANNELS = 32;
static constexpr int PACKETS_PER_CHANNEL = 50;

/**
 * @brief Tests sending a large volume of messages using RELIABLE_ORDERED
 * across multiple channels to verify ordering guarantees.
 */
class ReliableOrderedTest : public ::testing::Test {
protected:
    RakPeerInterface *sender = nullptr;
    RakPeerInterface *receiver = nullptr;

    void SetUp() override {
        sender = RakPeerInterface::GetInstance();
        ASSERT_NE(sender, nullptr);

        receiver = RakPeerInterface::GetInstance();
        ASSERT_NE(receiver, nullptr);

        receiver->SetMaximumIncomingConnections(8);

        SocketDescriptor sd1(0, nullptr), sd2(0, nullptr);
        ASSERT_EQ(receiver->Startup(8, &sd1, 1), RAKNET_STARTED);
        ASSERT_EQ(sender->Startup(8, &sd2, 1), RAKNET_STARTED);

        unsigned short receiverPort = receiver->GetMyBoundAddress().GetPort();
        ASSERT_EQ(sender->Connect("127.0.0.1", receiverPort, nullptr, 0),
                  CONNECTION_ATTEMPT_STARTED);

        // Wait for connection
        bool connected = false;
        auto deadline = Clock::now() + std::chrono::seconds(5);
        while (Clock::now() < deadline) {
            for (Packet *p = sender->Receive(); p;
                 sender->DeallocatePacket(p), p = sender->Receive()) {
                if (p->data[0] == ID_CONNECTION_REQUEST_ACCEPTED)
                    connected = true;
            }
            for (Packet *p = receiver->Receive(); p;
                 receiver->DeallocatePacket(p), p = receiver->Receive()) {
            }
            if (connected) break;
            RakSleep(30);
        }
        ASSERT_TRUE(connected) << "Connection was not established";
    }

    void TearDown() override {
        if (sender) { RakPeerInterface::DestroyInstance(sender); }
        if (receiver) { RakPeerInterface::DestroyInstance(receiver); }
    }
};

TEST_F(ReliableOrderedTest, PacketsArriveInOrderPerChannel) {
    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> padDist(1, 5000);

    // Paced sending: 30ms intervals, variable padding (1–5000 bytes)
    unsigned int sendSeq[NUM_CHANNELS] = {};
    unsigned int totalSent = 0;
    unsigned int totalExpected = NUM_CHANNELS * PACKETS_PER_CHANNEL;

    auto sendDeadline = Clock::now() + std::chrono::seconds(6);
    auto nextSend = Clock::now();

    while (Clock::now() < sendDeadline && totalSent < totalExpected) {
        if (Clock::now() >= nextSend) {
            for (unsigned char ch = 0; ch < NUM_CHANNELS; ch++) {
                if (sendSeq[ch] >= PACKETS_PER_CHANNEL)
                    continue;
                BitStream bs;
                bs.Write(static_cast<unsigned char>(ID_USER_PACKET_ENUM + 1));
                bs.Write(sendSeq[ch]);
                bs.Write(ch);
                bs.PadWithZeroToByteLength(padDist(rng));
                if (sender->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, ch,
                                 UNASSIGNED_SYSTEM_ADDRESS, true)) {
                    sendSeq[ch]++;
                    totalSent++;
                }
            }
            nextSend = Clock::now() + std::chrono::milliseconds(30);
        }
        // Drain sender while waiting
        for (Packet *p = sender->Receive(); p;
             sender->DeallocatePacket(p), p = sender->Receive()) {
        }
        RakSleep(1);
    }

    ASSERT_EQ(totalSent, totalExpected) << "Failed to send all packets";

    // Receive and validate ordering per channel
    unsigned int expectedSeq[NUM_CHANNELS] = {};
    unsigned int totalReceived = 0;

    auto recvDeadline = Clock::now() + std::chrono::seconds(10);
    while (Clock::now() < recvDeadline) {
        for (Packet *p = receiver->Receive(); p;
             receiver->DeallocatePacket(p), p = receiver->Receive()) {
            if (p->data[0] != ID_USER_PACKET_ENUM + 1)
                continue;

            unsigned int seq;
            unsigned char ch;
            BitStream bs(p->data, p->length, false);
            bs.IgnoreBytes(1);
            bs.Read(seq);
            bs.Read(ch);

            ASSERT_LT(ch, NUM_CHANNELS) << "Invalid channel number";
            EXPECT_EQ(seq, expectedSeq[ch])
                << "Out-of-order on channel " << static_cast<int>(ch)
                << ": expected " << expectedSeq[ch] << " got " << seq;

            if (seq >= expectedSeq[ch])
                expectedSeq[ch] = seq + 1;
            totalReceived++;
        }
        if (totalReceived >= totalExpected)
            break;
        RakSleep(30);
    }

    EXPECT_EQ(totalReceived, totalExpected)
        << "Did not receive all packets";

    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        EXPECT_EQ(expectedSeq[ch], static_cast<unsigned int>(PACKETS_PER_CHANNEL))
            << "Channel " << ch << " did not receive all packets";
    }
}
