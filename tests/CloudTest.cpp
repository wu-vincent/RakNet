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
#include "CloudServer.h"
#include "CloudClient.h"

using namespace RakNet;
using Clock = std::chrono::steady_clock;

/**
 * @brief Tests the CloudServer and CloudClient plugins for cloud data
 * operations including Post, Get, Release, and subscription notifications.
 */
class CloudTest : public ::testing::Test {
protected:
    static constexpr int CLIENT_1 = 0;
    static constexpr int CLIENT_2 = 1;
    static constexpr int SERVER_1 = 2;
    static constexpr int SERVER_2 = 3;
    static constexpr int PEER_COUNT = 4;

    RakPeerInterface *peers[PEER_COUNT] = {};
    CloudServer cloudServers[2];
    CloudClient cloudClients[2];
    unsigned short ports[PEER_COUNT] = {};

    void SetUp() override {
        // Create and start all peers
        for (int i = 0; i < PEER_COUNT; i++) {
            peers[i] = RakPeerInterface::GetInstance();
            ASSERT_NE(peers[i], nullptr);
            SocketDescriptor sd(0, nullptr);
            ASSERT_EQ(peers[i]->Startup(PEER_COUNT, &sd, 1), RAKNET_STARTED);
            ports[i] = peers[i]->GetMyBoundAddress().GetPort();
        }

        // Servers accept incoming connections
        peers[SERVER_1]->SetMaximumIncomingConnections(PEER_COUNT);
        peers[SERVER_2]->SetMaximumIncomingConnections(PEER_COUNT);

        // Attach plugins
        peers[CLIENT_1]->AttachPlugin(&cloudClients[0]);
        peers[CLIENT_2]->AttachPlugin(&cloudClients[1]);
        peers[SERVER_1]->AttachPlugin(&cloudServers[0]);
        peers[SERVER_2]->AttachPlugin(&cloudServers[1]);

        // Connect servers to each other
        ASSERT_EQ(peers[SERVER_2]->Connect("127.0.0.1", ports[SERVER_1], nullptr, 0),
                  CONNECTION_ATTEMPT_STARTED);
        RakSleep(200);
        DrainAll();

        // Tell servers about each other
        unsigned short numSys = 0;
        peers[SERVER_1]->GetConnectionList(nullptr, &numSys);
        for (unsigned short j = 0; j < numSys; j++)
            cloudServers[0].AddServer(peers[SERVER_1]->GetGUIDFromIndex(j));

        numSys = 0;
        peers[SERVER_2]->GetConnectionList(nullptr, &numSys);
        for (unsigned short j = 0; j < numSys; j++)
            cloudServers[1].AddServer(peers[SERVER_2]->GetGUIDFromIndex(j));

        // Connect client 1 to server 1, client 2 to server 2
        ASSERT_EQ(peers[CLIENT_1]->Connect("127.0.0.1", ports[SERVER_1], nullptr, 0),
                  CONNECTION_ATTEMPT_STARTED);
        ASSERT_EQ(peers[CLIENT_2]->Connect("127.0.0.1", ports[SERVER_2], nullptr, 0),
                  CONNECTION_ATTEMPT_STARTED);

        // Wait for client connections
        bool c1 = false, c2 = false;
        auto deadline = Clock::now() + std::chrono::seconds(5);
        while (Clock::now() < deadline && !(c1 && c2)) {
            for (Packet *p = peers[CLIENT_1]->Receive(); p;
                 peers[CLIENT_1]->DeallocatePacket(p), p = peers[CLIENT_1]->Receive()) {
                if (p->data[0] == ID_CONNECTION_REQUEST_ACCEPTED) c1 = true;
            }
            for (Packet *p = peers[CLIENT_2]->Receive(); p;
                 peers[CLIENT_2]->DeallocatePacket(p), p = peers[CLIENT_2]->Receive()) {
                if (p->data[0] == ID_CONNECTION_REQUEST_ACCEPTED) c2 = true;
            }
            DrainServers();
            RakSleep(10);
        }
        ASSERT_TRUE(c1) << "Client 1 did not connect to server 1";
        ASSERT_TRUE(c2) << "Client 2 did not connect to server 2";
    }

    void TearDown() override {
        for (int i = 0; i < PEER_COUNT; i++) {
            if (peers[i]) {
                peers[i]->Shutdown(0);
                RakPeerInterface::DestroyInstance(peers[i]);
            }
        }
    }

    void DrainServers() {
        for (Packet *p = peers[SERVER_1]->Receive(); p;
             peers[SERVER_1]->DeallocatePacket(p), p = peers[SERVER_1]->Receive()) {
        }
        for (Packet *p = peers[SERVER_2]->Receive(); p;
             peers[SERVER_2]->DeallocatePacket(p), p = peers[SERVER_2]->Receive()) {
        }
    }

    void DrainAll() {
        for (int i = 0; i < PEER_COUNT; i++) {
            for (Packet *p = peers[i]->Receive(); p;
                 peers[i]->DeallocatePacket(p), p = peers[i]->Receive()) {
            }
        }
    }

    RakNetGUID ServerGUID(int clientIdx) {
        return peers[clientIdx]->GetGUIDFromIndex(0);
    }
};

TEST_F(CloudTest, PostAndGet) {
    // Client 1 posts data to server 1
    CloudKey key;
    key.primaryKey = "AppName";
    key.secondaryKey = 1;
    cloudClients[0].Post(&key, reinterpret_cast<const unsigned char *>("hello"),
                         6, ServerGUID(CLIENT_1));

    RakSleep(200);
    DrainAll();

    // Client 1 gets data back from server 1
    CloudQuery query;
    query.keys.Push(CloudKey("AppName", 1), _FILE_AND_LINE_);
    query.maxRowsToReturn = 0;
    query.startingRowIndex = 0;
    query.subscribeToResults = false;
    cloudClients[0].Get(&query, ServerGUID(CLIENT_1));

    // Wait for ID_CLOUD_GET_RESPONSE
    bool gotResponse = false;
    auto deadline = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < deadline && !gotResponse) {
        DrainServers();
        for (Packet *p = peers[CLIENT_1]->Receive(); p;
             peers[CLIENT_1]->DeallocatePacket(p), p = peers[CLIENT_1]->Receive()) {
            if (p->data[0] == ID_CLOUD_GET_RESPONSE) {
                CloudQueryResult result;
                cloudClients[0].OnGetReponse(&result, p);
                EXPECT_GT(result.rowsReturned.Size(), 0u) << "Expected at least one row";
                if (result.rowsReturned.Size() > 0) {
                    EXPECT_EQ(result.rowsReturned[0]->length, 6u);
                    EXPECT_STREQ(reinterpret_cast<const char *>(result.rowsReturned[0]->data), "hello");
                }
                cloudClients[0].DeallocateWithDefaultAllocator(&result);
                gotResponse = true;
            }
        }
        for (Packet *p = peers[CLIENT_2]->Receive(); p;
             peers[CLIENT_2]->DeallocatePacket(p), p = peers[CLIENT_2]->Receive()) {
        }
        RakSleep(10);
    }
    EXPECT_TRUE(gotResponse) << "Did not receive ID_CLOUD_GET_RESPONSE";
}

TEST_F(CloudTest, CrossServerGet) {
    // Client 1 posts to server 1
    CloudKey key;
    key.primaryKey = "AppName";
    key.secondaryKey = 1;
    cloudClients[0].Post(&key, reinterpret_cast<const unsigned char *>("cross"),
                         6, ServerGUID(CLIENT_1));

    RakSleep(500);
    DrainAll();

    // Client 2 gets from server 2 — data should propagate across servers
    CloudQuery query;
    query.keys.Push(CloudKey("AppName", 1), _FILE_AND_LINE_);
    query.maxRowsToReturn = 0;
    query.startingRowIndex = 0;
    query.subscribeToResults = false;
    cloudClients[1].Get(&query, ServerGUID(CLIENT_2));

    bool gotResponse = false;
    auto deadline = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < deadline && !gotResponse) {
        DrainServers();
        for (Packet *p = peers[CLIENT_2]->Receive(); p;
             peers[CLIENT_2]->DeallocatePacket(p), p = peers[CLIENT_2]->Receive()) {
            if (p->data[0] == ID_CLOUD_GET_RESPONSE) {
                CloudQueryResult result;
                cloudClients[1].OnGetReponse(&result, p);
                EXPECT_GT(result.rowsReturned.Size(), 0u)
                    << "Cross-server get should return data posted on other server";
                if (result.rowsReturned.Size() > 0) {
                    EXPECT_STREQ(reinterpret_cast<const char *>(result.rowsReturned[0]->data), "cross");
                }
                cloudClients[1].DeallocateWithDefaultAllocator(&result);
                gotResponse = true;
            }
        }
        for (Packet *p = peers[CLIENT_1]->Receive(); p;
             peers[CLIENT_1]->DeallocatePacket(p), p = peers[CLIENT_1]->Receive()) {
        }
        RakSleep(10);
    }
    EXPECT_TRUE(gotResponse) << "Did not receive cross-server get response";
}

TEST_F(CloudTest, SubscriptionNotification) {
    // Client 2 subscribes to key on server 2
    CloudQuery query;
    query.keys.Push(CloudKey("AppName", 1), _FILE_AND_LINE_);
    query.maxRowsToReturn = 0;
    query.startingRowIndex = 0;
    query.subscribeToResults = true;
    cloudClients[1].Get(&query, ServerGUID(CLIENT_2));

    // Wait for the initial get response
    auto deadline = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < deadline) {
        DrainServers();
        for (Packet *p = peers[CLIENT_2]->Receive(); p;
             peers[CLIENT_2]->DeallocatePacket(p), p = peers[CLIENT_2]->Receive()) {
            if (p->data[0] == ID_CLOUD_GET_RESPONSE) {
                CloudQueryResult result;
                cloudClients[1].OnGetReponse(&result, p);
                cloudClients[1].DeallocateWithDefaultAllocator(&result);
                goto subscribed;
            }
        }
        for (Packet *p = peers[CLIENT_1]->Receive(); p;
             peers[CLIENT_1]->DeallocatePacket(p), p = peers[CLIENT_1]->Receive()) {
        }
        RakSleep(10);
    }
    FAIL() << "Did not receive initial subscription get response";
subscribed:

    // Client 1 posts data — should trigger subscription notification to client 2
    CloudKey key;
    key.primaryKey = "AppName";
    key.secondaryKey = 1;
    cloudClients[0].Post(&key, reinterpret_cast<const unsigned char *>("update"),
                         7, ServerGUID(CLIENT_1));

    bool gotNotification = false;
    deadline = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < deadline && !gotNotification) {
        DrainServers();
        for (Packet *p = peers[CLIENT_2]->Receive(); p;
             peers[CLIENT_2]->DeallocatePacket(p), p = peers[CLIENT_2]->Receive()) {
            if (p->data[0] == ID_CLOUD_SUBSCRIPTION_NOTIFICATION) {
                bool wasUpdated = false;
                CloudQueryRow row;
                cloudClients[1].OnSubscriptionNotification(&wasUpdated, &row, p);
                EXPECT_TRUE(wasUpdated) << "Expected update notification, not deletion";
                cloudClients[1].DeallocateWithDefaultAllocator(&row);
                gotNotification = true;
            }
        }
        for (Packet *p = peers[CLIENT_1]->Receive(); p;
             peers[CLIENT_1]->DeallocatePacket(p), p = peers[CLIENT_1]->Receive()) {
        }
        RakSleep(10);
    }
    EXPECT_TRUE(gotNotification) << "Did not receive subscription notification";
}
