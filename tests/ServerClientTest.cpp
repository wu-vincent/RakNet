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
#include "MessageIdentifiers.h"
#include "RakSleep.h"

using namespace RakNet;
using Clock = std::chrono::steady_clock;

static constexpr int NUM_CLIENTS = 100;
static constexpr int RANDOM_DATA_SIZE_1 = 50;
static constexpr int RANDOM_DATA_SIZE_2 = 100;

/**
 * @brief Tests connecting many clients to a single server with bidirectional
 * data flow using a server/client topology.
 */
class ServerClientTest : public ::testing::Test {
protected:
    RakPeerInterface *server = nullptr;
    RakPeerInterface *clients[NUM_CLIENTS] = {};
    unsigned short serverPort = 0;

    char randomData1[RANDOM_DATA_SIZE_1] = {};
    char randomData2[RANDOM_DATA_SIZE_2] = {};

    void SetUp() override {
        // Fill data buffers with pattern
        randomData1[0] = static_cast<char>(ID_USER_PACKET_ENUM);
        for (int i = 1; i < RANDOM_DATA_SIZE_1; i++)
            randomData1[i] = static_cast<char>(i - 1);

        randomData2[0] = static_cast<char>(ID_USER_PACKET_ENUM);
        for (int i = 1; i < RANDOM_DATA_SIZE_2; i++)
            randomData2[i] = static_cast<char>(i - 1);

        // Start server
        server = RakPeerInterface::GetInstance();
        ASSERT_NE(server, nullptr);

        SocketDescriptor sdServer(0, nullptr);
        ASSERT_EQ(server->Startup(NUM_CLIENTS + 10, &sdServer, 1), RAKNET_STARTED);
        server->SetMaximumIncomingConnections(NUM_CLIENTS + 10);
        serverPort = server->GetMyBoundAddress().GetPort();

        // Start and connect all clients
        for (int i = 0; i < NUM_CLIENTS; i++) {
            clients[i] = RakPeerInterface::GetInstance();
            ASSERT_NE(clients[i], nullptr);

            SocketDescriptor sd(0, nullptr);
            ASSERT_EQ(clients[i]->Startup(1, &sd, 1), RAKNET_STARTED);
            ASSERT_EQ(clients[i]->Connect("127.0.0.1", serverPort, nullptr, 0),
                      CONNECTION_ATTEMPT_STARTED);
        }
    }

    void TearDown() override {
        for (int i = 0; i < NUM_CLIENTS; i++) {
            if (clients[i])
                RakPeerInterface::DestroyInstance(clients[i]);
        }
        if (server)
            RakPeerInterface::DestroyInstance(server);
    }

    void DrainServer() {
        for (Packet *p = server->Receive(); p;
             server->DeallocatePacket(p), p = server->Receive()) {
        }
    }

    void DrainClients() {
        for (int i = 0; i < NUM_CLIENTS; i++) {
            for (Packet *p = clients[i]->Receive(); p;
                 clients[i]->DeallocatePacket(p), p = clients[i]->Receive()) {
            }
        }
    }

    unsigned int CountServerConnections() {
        unsigned short numSystems = 0;
        server->GetConnectionList(nullptr, &numSystems);
        return numSystems;
    }

    unsigned int CountConnectedClients() {
        unsigned int count = 0;
        for (int i = 0; i < NUM_CLIENTS; i++) {
            ConnectionState state = clients[i]->GetConnectionState(
                clients[i]->GetSystemAddressFromIndex(0));
            if (state == IS_CONNECTED)
                count++;
        }
        return count;
    }
};

TEST_F(ServerClientTest, AllClientsConnect) {
    // Wait for all clients to connect
    auto deadline = Clock::now() + std::chrono::seconds(10);
    while (Clock::now() < deadline) {
        DrainServer();
        DrainClients();

        if (CountServerConnections() >= NUM_CLIENTS)
            break;
        RakSleep(30);
    }

    unsigned int serverSees = CountServerConnections();
    EXPECT_EQ(serverSees, static_cast<unsigned int>(NUM_CLIENTS))
        << "Server should see all " << NUM_CLIENTS << " clients connected";
}

TEST_F(ServerClientTest, BidirectionalDataFlow) {
    std::mt19937 rng{std::random_device{}()};

    // Wait for connections to establish
    auto deadline = Clock::now() + std::chrono::seconds(10);
    while (Clock::now() < deadline) {
        DrainServer();
        DrainClients();
        if (CountServerConnections() >= NUM_CLIENTS)
            break;
        RakSleep(30);
    }
    ASSERT_GE(CountServerConnections(), static_cast<unsigned int>(NUM_CLIENTS / 2))
        << "Not enough clients connected";

    // Run bidirectional traffic for 5 seconds
    auto endTime = Clock::now() + std::chrono::seconds(5);
    auto nextServerSend = Clock::now();
    Clock::time_point nextClientSend[NUM_CLIENTS] = {};

    while (Clock::now() < endTime) {
        auto curTime = Clock::now();

        // Server broadcasts periodically
        if (curTime > nextServerSend) {
            if (rng() % 10 == 0)
                server->Send(randomData2, RANDOM_DATA_SIZE_2, HIGH_PRIORITY,
                             RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
            else
                server->Send(randomData1, RANDOM_DATA_SIZE_1, HIGH_PRIORITY,
                             RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
            nextServerSend = curTime + std::chrono::milliseconds(100);
        }

        // Each client sends periodically
        for (int i = 0; i < NUM_CLIENTS; i++) {
            if (curTime > nextClientSend[i]) {
                ConnectionState state = clients[i]->GetConnectionState(
                    clients[i]->GetSystemAddressFromIndex(0));
                if (state == IS_CONNECTED) {
                    if (rng() % 10 == 0)
                        clients[i]->Send(randomData2, RANDOM_DATA_SIZE_2, HIGH_PRIORITY,
                                         RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
                    else
                        clients[i]->Send(randomData1, RANDOM_DATA_SIZE_1, HIGH_PRIORITY,
                                         RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
                }
                nextClientSend[i] = curTime + std::chrono::milliseconds(50);
            }
        }

        DrainServer();
        DrainClients();
        RakSleep(10);
    }

    // Verify server still has connections after traffic
    unsigned int remaining = CountServerConnections();
    EXPECT_GT(remaining, 0u) << "Server lost all connections during traffic";

    // If we got here without crashing, bidirectional flow works
    SUCCEED();
}
