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

static constexpr int NUM_CLIENTS = 9;

/**
 * @brief Tests silently dropping multiple instances of RakNet. Verifies that
 * lost connections are detected properly.
 */
class DroppedConnectionTest : public ::testing::Test {
protected:
    RakPeerInterface *server = nullptr;
    RakPeerInterface *clients[NUM_CLIENTS] = {};
    unsigned short serverPort = 0;
    SystemAddress serverAddr;

    void SetUp() override {
        server = RakPeerInterface::GetInstance();
        ASSERT_NE(server, nullptr);

        SocketDescriptor sdServer(0, nullptr);
        ASSERT_EQ(server->Startup(NUM_CLIENTS, &sdServer, 1), RAKNET_STARTED);
        server->SetMaximumIncomingConnections(NUM_CLIENTS);
        server->SetTimeoutTime(2000, UNASSIGNED_SYSTEM_ADDRESS);

        serverPort = server->GetMyBoundAddress().GetPort();
        serverAddr.SetBinaryAddress("127.0.0.1");
        serverAddr.SetPortHostOrder(serverPort);

        for (int i = 0; i < NUM_CLIENTS; i++) {
            clients[i] = RakPeerInterface::GetInstance();
            ASSERT_NE(clients[i], nullptr);

            SocketDescriptor sd(0, nullptr);
            ASSERT_EQ(clients[i]->Startup(1, &sd, 1), RAKNET_STARTED);
            ASSERT_EQ(clients[i]->Connect("127.0.0.1", serverPort, nullptr, 0),
                      CONNECTION_ATTEMPT_STARTED);
            clients[i]->SetTimeoutTime(5000, UNASSIGNED_SYSTEM_ADDRESS);
            RakSleep(100);
        }

        // Wait for all clients to connect
        RakSleep(1000);
        DrainAllPackets();
    }

    void TearDown() override {
        for (int i = 0; i < NUM_CLIENTS; i++) {
            if (clients[i])
                RakPeerInterface::DestroyInstance(clients[i]);
        }
        if (server)
            RakPeerInterface::DestroyInstance(server);
    }

    void DrainAllPackets() {
        for (Packet *p = server->Receive(); p;
             server->DeallocatePacket(p), p = server->Receive()) {
        }
        for (int i = 0; i < NUM_CLIENTS; i++) {
            for (Packet *p = clients[i]->Receive(); p;
                 clients[i]->DeallocatePacket(p), p = clients[i]->Receive()) {
            }
        }
    }

    bool IsClientBusyWithServer(int index) {
        ConnectionState state = clients[index]->GetConnectionState(serverAddr);
        return state == IS_CONNECTED || state == IS_CONNECTING ||
               state == IS_PENDING || state == IS_DISCONNECTING;
    }

    void VerifyNoClientHasMultipleConnections() {
        for (int i = 0; i < NUM_CLIENTS; i++) {
            unsigned short numSystems = 0;
            clients[i]->GetConnectionList(nullptr, &numSystems);
            EXPECT_LE(numSystems, 1u)
                << "Client " << i << " has " << numSystems << " connections";
        }
    }

    unsigned int CountConnectedClients() {
        unsigned int count = 0;
        for (int i = 0; i < NUM_CLIENTS; i++) {
            unsigned short numSystems = 0;
            clients[i]->GetConnectionList(nullptr, &numSystems);
            if (numSystems == 1)
                count++;
        }
        return count;
    }
};

TEST_F(DroppedConnectionTest, RandomDisconnectReconnectCycle) {
    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> actionDist(0, 3);
    std::uniform_int_distribution<int> clientDist(0, NUM_CLIENTS - 1);

    auto entryTime = Clock::now();

    // Run for 15 seconds (ConvertTest ran for 30s)
    while (Clock::now() - entryTime < std::chrono::seconds(15)) {
        int action = actionDist(rng);

        switch (action) {
        case 0: {
            // Silent disconnect a random client
            int idx = clientDist(rng);
            clients[idx]->CloseConnection(serverAddr, false, 0);
            break;
        }
        case 1: {
            // Reconnect a random client if not busy
            int idx = clientDist(rng);
            if (!IsClientBusyWithServer(idx)) {
                EXPECT_EQ(clients[idx]->Connect("127.0.0.1", serverPort, nullptr, 0),
                          CONNECTION_ATTEMPT_STARTED)
                    << "Connect failed for client " << idx;
            }
            break;
        }
        case 2: {
            // Random connect/disconnect for all clients
            for (int i = 0; i < NUM_CLIENTS; i++) {
                if (NUM_CLIENTS == 1 || (rng() % 2) == 0) {
                    if (clients[i]->IsActive()) {
                        bool notify = (rng() % 2) != 0;
                        clients[i]->CloseConnection(serverAddr, notify, 0);
                    }
                } else {
                    if (!IsClientBusyWithServer(i)) {
                        EXPECT_EQ(clients[i]->Connect("127.0.0.1", serverPort, nullptr, 0),
                                  CONNECTION_ATTEMPT_STARTED)
                            << "Connect failed for client " << i;
                    }
                }
            }
            break;
        }
        case 3: {
            // Wait for timeout, then verify dropped connections detected
            RakSleep(1000);
            DrainAllPackets();
            RakSleep(1000);

            unsigned short serverSees = 0;
            server->GetConnectionList(nullptr, &serverSees);
            unsigned int actualConnected = CountConnectedClients();

            EXPECT_EQ(static_cast<unsigned int>(serverSees), actualConnected)
                << "Server thinks " << serverSees << " clients connected, "
                << "but actually " << actualConnected << " are connected";
            break;
        }
        }

        DrainAllPackets();
        VerifyNoClientHasMultipleConnections();
        RakSleep(10);
    }
}
