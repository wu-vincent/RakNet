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
#include <cstring>
#include <random>

#include <gtest/gtest.h>

#include "RakPeerInterface.h"
#include "RakNetStatistics.h"
#include "MessageIdentifiers.h"
#include "RakSleep.h"

using namespace RakNet;
using Clock = std::chrono::steady_clock;

static constexpr int NUM_PEERS = 10;
static constexpr int CONNECTIONS_PER_SYSTEM = 4;

/**
 * @brief Stress test that exercises a bit of everything—connect, disconnect,
 * send, ping, statistics—to verify RakNet does not crash or leak under
 * sustained random operations.
 */
class ComprehensiveTest : public ::testing::Test {
protected:
    RakPeerInterface *peers[NUM_PEERS] = {};
    unsigned short ports[NUM_PEERS] = {};

    void SetUp() override {
        for (int i = 0; i < NUM_PEERS; i++) {
            peers[i] = RakPeerInterface::GetInstance();
            ASSERT_NE(peers[i], nullptr);

            peers[i]->SetMaximumIncomingConnections(CONNECTIONS_PER_SYSTEM);
            SocketDescriptor sd(0, nullptr);
            ASSERT_EQ(peers[i]->Startup(NUM_PEERS, &sd, 1), RAKNET_STARTED);
            peers[i]->SetOfflinePingResponse("Offline Ping Data",
                                             static_cast<int>(strlen("Offline Ping Data")) + 1);
            ports[i] = peers[i]->GetMyBoundAddress().GetPort();
        }
    }

    void TearDown() override {
        for (int i = 0; i < NUM_PEERS; i++) {
            if (peers[i])
                RakPeerInterface::DestroyInstance(peers[i]);
        }
    }

    bool IsBusyWith(int peerIdx, int targetIdx) {
        SystemAddress addr;
        addr.SetBinaryAddress("127.0.0.1");
        addr.SetPortHostOrder(ports[targetIdx]);
        ConnectionState state = peers[peerIdx]->GetConnectionState(addr);
        return state == IS_CONNECTED || state == IS_CONNECTING ||
               state == IS_PENDING || state == IS_DISCONNECTING;
    }
};

TEST_F(ComprehensiveTest, RandomOperationsNoCrash) {
    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> peerDist(0, NUM_PEERS - 1);
    std::uniform_real_distribution<float> actionDist(0.0f, 1.0f);

    // Initial connections: each peer connects to a random peer
    for (int i = 0; i < NUM_PEERS; i++) {
        int target = peerDist(rng);
        if (!IsBusyWith(i, target)) {
            auto result = peers[i]->Connect("127.0.0.1", ports[target], nullptr, 0);
            EXPECT_TRUE(result == CONNECTION_ATTEMPT_STARTED ||
                        result == ALREADY_CONNECTED_TO_ENDPOINT)
                << "Initial connect failed for peer " << i;
        }
    }

    char data[8096];
    auto endTime = Clock::now() + std::chrono::seconds(5);

    while (Clock::now() < endTime) {
        float action = actionDist(rng);
        int peerIdx = peerDist(rng);

        if (action < 0.04f) {
            // Re-startup and connect
            SocketDescriptor sd(ports[peerIdx], nullptr);
            peers[peerIdx]->Startup(NUM_PEERS, &sd, 1);
            int target = peerDist(rng);
            if (!IsBusyWith(peerIdx, target)) {
                auto result = peers[peerIdx]->Connect("127.0.0.1", ports[target], nullptr, 0);
                EXPECT_TRUE(result == CONNECTION_ATTEMPT_STARTED ||
                            result == ALREADY_CONNECTED_TO_ENDPOINT)
                    << "Re-startup connect failed for peer " << peerIdx;
            }
        } else if (action < 0.09f) {
            // Connect to random peer
            int target = peerDist(rng);
            if (!IsBusyWith(peerIdx, target)) {
                auto result = peers[peerIdx]->Connect("127.0.0.1", ports[target], nullptr, 0);
                EXPECT_TRUE(result == CONNECTION_ATTEMPT_STARTED ||
                            result == ALREADY_CONNECTED_TO_ENDPOINT)
                    << "Connect failed for peer " << peerIdx;
            }
        } else if (action < 0.12f) {
            // GetConnectionList
            SystemAddress remoteSystems[NUM_PEERS];
            unsigned short numSystems = NUM_PEERS;
            peers[peerIdx]->GetConnectionList(remoteSystems, &numSystems);
        } else if (action < 0.14f) {
            // Send random data
            data[0] = ID_USER_PACKET_ENUM;
            int dataLength = 3 + static_cast<int>(rng() % 8000);
            auto priority = static_cast<PacketPriority>(rng() % static_cast<int>(NUMBER_OF_PRIORITIES));
            auto reliability = static_cast<PacketReliability>(rng() % (static_cast<int>(RELIABLE_SEQUENCED) + 1));
            unsigned char orderingChannel = rng() % 32;
            SystemAddress target;
            if ((rng() % NUM_PEERS) == 0)
                target = UNASSIGNED_SYSTEM_ADDRESS;
            else
                target = peers[peerIdx]->GetSystemAddressFromIndex(peerDist(rng));
            bool broadcast = (rng() % 2) > 0;
            data[dataLength - 1] = 0;
            peers[peerIdx]->Send(data, dataLength, priority, reliability,
                                 orderingChannel, target, broadcast);
        } else if (action < 0.181f) {
            // CloseConnection
            SystemAddress target = peers[peerIdx]->GetSystemAddressFromIndex(peerDist(rng));
            peers[peerIdx]->CloseConnection(target, (rng() % 2) > 0, 0);
        } else if (action < 0.20f) {
            // Offline Ping
            int target = peerDist(rng);
            peers[peerIdx]->Ping("127.0.0.1", ports[target], (rng() % 2) > 0);
        } else if (action < 0.21f) {
            // Online Ping
            SystemAddress target = peers[peerIdx]->GetSystemAddressFromIndex(peerDist(rng));
            peers[peerIdx]->Ping(target);
        } else if (action < 0.25f) {
            // GetStatistics
            SystemAddress myAddr = peers[peerIdx]->GetInternalID();
            peers[peerIdx]->GetStatistics(myAddr);
            SystemAddress target = peers[peerIdx]->GetSystemAddressFromIndex(peerDist(rng));
            peers[peerIdx]->GetStatistics(target);
        }

        // Drain all packets
        for (int i = 0; i < NUM_PEERS; i++)
            peers[i]->DeallocatePacket(peers[i]->Receive());

        RakSleep(0);
    }

    // If we got here without crashing, the test passed
    SUCCEED();
}
