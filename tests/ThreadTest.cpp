/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <atomic>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "RakSleep.h"

using namespace RakNet;

/**
 * @brief Tests that RakNet will not crash if called from multiple threads
 * concurrently.
 */
class ThreadTest : public ::testing::Test {
protected:
    RakPeerInterface *peer1 = nullptr;
    RakPeerInterface *peer2 = nullptr;

    void SetUp() override {
        peer1 = RakPeerInterface::GetInstance();
        ASSERT_NE(peer1, nullptr);

        peer2 = RakPeerInterface::GetInstance();
        ASSERT_NE(peer2, nullptr);

        peer1->SetMaximumIncomingConnections(1);
        peer2->SetMaximumIncomingConnections(1);

        SocketDescriptor sd1(0, nullptr), sd2(0, nullptr);
        ASSERT_EQ(peer1->Startup(1, &sd1, 1), RAKNET_STARTED);
        ASSERT_EQ(peer2->Startup(1, &sd2, 1), RAKNET_STARTED);

        unsigned short port1 = peer1->GetMyBoundAddress().GetPort();
        unsigned short port2 = peer2->GetMyBoundAddress().GetPort();

        RakSleep(100);
        peer1->Connect("127.0.0.1", port2, nullptr, 0);
        peer2->Connect("127.0.0.1", port1, nullptr, 0);
        RakSleep(500);
    }

    void TearDown() override {
        if (peer1) { RakPeerInterface::DestroyInstance(peer1); }
        if (peer2) { RakPeerInterface::DestroyInstance(peer2); }
    }
};

TEST_F(ThreadTest, ConcurrentProducersAndConsumers) {
    std::atomic<bool> stop{false};
    std::atomic<int> receivedCount{0};

    auto producer = [&](int id) {
        char out[2];
        out[0] = static_cast<char>(ID_USER_PACKET_ENUM);
        out[1] = static_cast<char>(id);
        while (!stop) {
            RakPeerInterface *peer = (id & 1) ? peer1 : peer2;
            peer->Send(out, 2, HIGH_PRIORITY, RELIABLE_ORDERED, 0,
                       UNASSIGNED_SYSTEM_ADDRESS, true);
            RakSleep(30);
        }
    };

    auto consumer = [&](int id) {
        while (!stop) {
            RakPeerInterface *peer = (id & 1) ? peer1 : peer2;
            Packet *p = peer->Receive();
            if (p) {
                if (p->data[0] == ID_USER_PACKET_ENUM) {
                    receivedCount++;
                }
                peer->DeallocatePacket(p);
            }
            RakSleep(30);
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(30);
    for (int i = 0; i < 10; i++) {
        threads.emplace_back(producer, i);
    }
    for (int i = 10; i < 20; i++) {
        threads.emplace_back(consumer, i);
    }

    // Run for 3 seconds (original ran for 60s — too slow for CI)
    RakSleep(3000);
    stop = true;

    for (auto &t: threads) {
        t.join();
    }

    EXPECT_GT(receivedCount.load(), 0)
        << "Expected at least some packets to be received across threads";
}
