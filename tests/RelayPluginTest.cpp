/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <gtest/gtest.h>

#include "RakPeerInterface.h"
#include "BitStream.h"
#include "GetTime.h"
#include "MessageIdentifiers.h"
#include "RakSleep.h"
#include "RelayPlugin.h"

using namespace RakNet;

class RelayPluginTest : public ::testing::Test {
protected:
    RakPeerInterface *server = nullptr;
    RakPeerInterface *clientA = nullptr;
    RakPeerInterface *clientB = nullptr;
    RelayPlugin *serverRelay = nullptr;
    RelayPlugin *clientARelay = nullptr;
    RelayPlugin *clientBRelay = nullptr;
    unsigned short serverPort = 0;
    RakNetGUID serverGuid;

    void SetUp() override {
        server = RakPeerInterface::GetInstance();
        clientA = RakPeerInterface::GetInstance();
        clientB = RakPeerInterface::GetInstance();
        ASSERT_NE(server, nullptr);
        ASSERT_NE(clientA, nullptr);
        ASSERT_NE(clientB, nullptr);

        serverRelay = RelayPlugin::GetInstance();
        clientARelay = RelayPlugin::GetInstance();
        clientBRelay = RelayPlugin::GetInstance();
        ASSERT_NE(serverRelay, nullptr);
        ASSERT_NE(clientARelay, nullptr);
        ASSERT_NE(clientBRelay, nullptr);

        server->AttachPlugin(serverRelay);
        clientA->AttachPlugin(clientARelay);
        clientB->AttachPlugin(clientBRelay);

        serverRelay->SetAcceptAddParticipantRequests(true);

        server->SetMaximumIncomingConnections(8);
        SocketDescriptor sdServer(0, nullptr);
        ASSERT_EQ(server->Startup(8, &sdServer, 1), RAKNET_STARTED);
        serverPort = server->GetMyBoundAddress().GetPort();

        SocketDescriptor sdA(0, nullptr), sdB(0, nullptr);
        ASSERT_EQ(clientA->Startup(1, &sdA, 1), RAKNET_STARTED);
        ASSERT_EQ(clientB->Startup(1, &sdB, 1), RAKNET_STARTED);

        ASSERT_EQ(clientA->Connect("127.0.0.1", serverPort, nullptr, 0),
                  CONNECTION_ATTEMPT_STARTED);
        ASSERT_EQ(clientB->Connect("127.0.0.1", serverPort, nullptr, 0),
                  CONNECTION_ATTEMPT_STARTED);

        // Wait for both clients to connect
        bool aConnected = false, bConnected = false;
        Time deadline = GetTime() + 5000;
        while (GetTime() < deadline && !(aConnected && bConnected)) {
            DrainServer();
            for (Packet *p = clientA->Receive(); p;
                 clientA->DeallocatePacket(p), p = clientA->Receive()) {
                if (p->data[0] == ID_CONNECTION_REQUEST_ACCEPTED) {
                    serverGuid = p->guid;
                    aConnected = true;
                }
            }
            for (Packet *p = clientB->Receive(); p;
                 clientB->DeallocatePacket(p), p = clientB->Receive()) {
                if (p->data[0] == ID_CONNECTION_REQUEST_ACCEPTED)
                    bConnected = true;
            }
            RakSleep(10);
        }
        ASSERT_TRUE(aConnected) << "Client A did not connect";
        ASSERT_TRUE(bConnected) << "Client B did not connect";
    }

    void TearDown() override {
        if (clientA) {
            clientA->DetachPlugin(clientARelay);
            RakPeerInterface::DestroyInstance(clientA);
        }
        if (clientB) {
            clientB->DetachPlugin(clientBRelay);
            RakPeerInterface::DestroyInstance(clientB);
        }
        if (server) {
            server->DetachPlugin(serverRelay);
            RakPeerInterface::DestroyInstance(server);
        }
        if (clientARelay) RelayPlugin::DestroyInstance(clientARelay);
        if (clientBRelay) RelayPlugin::DestroyInstance(clientBRelay);
        if (serverRelay) RelayPlugin::DestroyInstance(serverRelay);
    }

    void DrainServer() {
        for (Packet *p = server->Receive(); p;
             server->DeallocatePacket(p), p = server->Receive()) {
        }
    }

    RelayPluginEnums WaitForRelayResponse(RakPeerInterface *peer, int timeoutMs = 3000) {
        Time deadline = GetTime() + timeoutMs;
        while (GetTime() < deadline) {
            DrainServer();
            for (Packet *p = peer->Receive(); p;
                 peer->DeallocatePacket(p), p = peer->Receive()) {
                if (p->data[0] == ID_RELAY_PLUGIN) {
                    BitStream bs(p->data, p->length, false);
                    bs.IgnoreBytes(sizeof(MessageID));
                    RelayPluginEnums rpe;
                    bs.ReadCasted<MessageID>(rpe);
                    return rpe;
                }
            }
            RakSleep(10);
        }
        return static_cast<RelayPluginEnums>(-1);
    }
};

TEST_F(RelayPluginTest, ParticipantRegistration) {
    // Register both clients as participants
    clientARelay->AddParticipantRequestFromClient("Alice", serverGuid);
    RelayPluginEnums resultA = WaitForRelayResponse(clientA);
    EXPECT_EQ(resultA, RPE_ADD_CLIENT_SUCCESS) << "Client A registration failed";

    clientBRelay->AddParticipantRequestFromClient("Bob", serverGuid);
    RelayPluginEnums resultB = WaitForRelayResponse(clientB);
    EXPECT_EQ(resultB, RPE_ADD_CLIENT_SUCCESS) << "Client B registration failed";

    // Duplicate name should fail
    clientBRelay->AddParticipantRequestFromClient("Alice", serverGuid);
    RelayPluginEnums resultDup = WaitForRelayResponse(clientB);
    EXPECT_EQ(resultDup, RPE_ADD_CLIENT_NAME_ALREADY_IN_USE);
}

TEST_F(RelayPluginTest, PointToPointMessage) {
    // Register both participants
    clientARelay->AddParticipantRequestFromClient("Alice", serverGuid);
    ASSERT_EQ(WaitForRelayResponse(clientA), RPE_ADD_CLIENT_SUCCESS);

    clientBRelay->AddParticipantRequestFromClient("Bob", serverGuid);
    ASSERT_EQ(WaitForRelayResponse(clientB), RPE_ADD_CLIENT_SUCCESS);

    // Alice sends a message to Bob via the relay
    RakString msg = "hello from Alice";
    BitStream msgBs;
    msgBs.WriteCompressed(msg);
    clientARelay->SendToParticipant(serverGuid, "Bob", &msgBs,
                                    HIGH_PRIORITY, RELIABLE_ORDERED, 0);

    // Bob should receive RPE_MESSAGE_TO_CLIENT_FROM_SERVER
    bool gotMessage = false;
    Time deadline = GetTime() + 3000;
    while (GetTime() < deadline && !gotMessage) {
        DrainServer();
        for (Packet *p = clientB->Receive(); p;
             clientB->DeallocatePacket(p), p = clientB->Receive()) {
            if (p->data[0] == ID_RELAY_PLUGIN) {
                BitStream bs(p->data, p->length, false);
                bs.IgnoreBytes(sizeof(MessageID));
                RelayPluginEnums rpe;
                bs.ReadCasted<MessageID>(rpe);
                if (rpe == RPE_MESSAGE_TO_CLIENT_FROM_SERVER) {
                    RakString senderName;
                    bs.ReadCompressed(senderName);
                    bs.AlignReadToByteBoundary();
                    RakString data;
                    bs.ReadCompressed(data);
                    EXPECT_STREQ(senderName.C_String(), "Alice");
                    EXPECT_STREQ(data.C_String(), "hello from Alice");
                    gotMessage = true;
                }
            }
        }
        // Drain client A too
        for (Packet *p = clientA->Receive(); p;
             clientA->DeallocatePacket(p), p = clientA->Receive()) {
        }
        RakSleep(10);
    }
    EXPECT_TRUE(gotMessage) << "Bob did not receive relayed message from Alice";
}

TEST_F(RelayPluginTest, GroupMessaging) {
    // Register both participants
    clientARelay->AddParticipantRequestFromClient("Alice", serverGuid);
    ASSERT_EQ(WaitForRelayResponse(clientA), RPE_ADD_CLIENT_SUCCESS);

    clientBRelay->AddParticipantRequestFromClient("Bob", serverGuid);
    ASSERT_EQ(WaitForRelayResponse(clientB), RPE_ADD_CLIENT_SUCCESS);

    // Both join a group
    clientARelay->JoinGroupRequest(serverGuid, "TestRoom");
    clientBRelay->JoinGroupRequest(serverGuid, "TestRoom");

    // Wait for join notifications to settle
    RakSleep(500);
    DrainServer();
    // Drain join notifications
    for (Packet *p = clientA->Receive(); p;
         clientA->DeallocatePacket(p), p = clientA->Receive()) {
    }
    for (Packet *p = clientB->Receive(); p;
         clientB->DeallocatePacket(p), p = clientB->Receive()) {
    }

    // Alice sends a group message
    RakString msg = "group hello";
    BitStream msgBs;
    msgBs.Write(msg);
    clientARelay->SendGroupMessage(serverGuid, &msgBs,
                                   HIGH_PRIORITY, RELIABLE_ORDERED, 0);

    // Bob should receive RPE_GROUP_MSG_FROM_SERVER
    bool bobGotGroupMsg = false;
    Time deadline = GetTime() + 3000;
    while (GetTime() < deadline && !bobGotGroupMsg) {
        DrainServer();
        for (Packet *p = clientB->Receive(); p;
             clientB->DeallocatePacket(p), p = clientB->Receive()) {
            if (p->data[0] == ID_RELAY_PLUGIN) {
                BitStream bs(p->data, p->length, false);
                bs.IgnoreBytes(sizeof(MessageID));
                RelayPluginEnums rpe;
                bs.ReadCasted<MessageID>(rpe);
                if (rpe == RPE_GROUP_MSG_FROM_SERVER) {
                    RakString senderName;
                    bs.ReadCompressed(senderName);
                    bs.AlignReadToByteBoundary();
                    RakString data;
                    bs.Read(data);
                    EXPECT_STREQ(senderName.C_String(), "Alice");
                    EXPECT_STREQ(data.C_String(), "group hello");
                    bobGotGroupMsg = true;
                }
            }
        }
        for (Packet *p = clientA->Receive(); p;
             clientA->DeallocatePacket(p), p = clientA->Receive()) {
        }
        RakSleep(10);
    }
    EXPECT_TRUE(bobGotGroupMsg) << "Bob did not receive group message";
}
