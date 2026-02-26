/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <cstring>
#include <vector>

#include <gtest/gtest.h>

#include "RakPeerInterface.h"
#include "BitStream.h"
#include "GetTime.h"
#include "MessageIdentifiers.h"
#include "RakSleep.h"

using namespace RakNet;

class BigPacketTest : public ::testing::Test {
protected:
    RakPeerInterface *server = nullptr;
    RakPeerInterface *client = nullptr;
    unsigned short serverPort = 0;

    void SetUp() override {
        server = RakPeerInterface::GetInstance();
        ASSERT_NE(server, nullptr);

        client = RakPeerInterface::GetInstance();
        ASSERT_NE(client, nullptr);

        server->SetMaximumIncomingConnections(4);
        server->SetTimeoutTime(5000, UNASSIGNED_SYSTEM_ADDRESS);
        client->SetTimeoutTime(5000, UNASSIGNED_SYSTEM_ADDRESS);

        SocketDescriptor sdServer(0, nullptr);
        ASSERT_EQ(server->Startup(4, &sdServer, 1), RAKNET_STARTED);

        SocketDescriptor sdClient(0, nullptr);
        ASSERT_EQ(client->Startup(4, &sdClient, 1), RAKNET_STARTED);

        serverPort = server->GetMyBoundAddress().GetPort();

        client->SetSplitMessageProgressInterval(10000);
        ASSERT_EQ(client->Connect("127.0.0.1", serverPort, nullptr, 0),
                  CONNECTION_ATTEMPT_STARTED);
    }

    void TearDown() override {
        if (client) RakPeerInterface::DestroyInstance(client);
        if (server) RakPeerInterface::DestroyInstance(server);
    }

    void SendAndVerifyBigPacket(int packetSize, bool verifyData) {
        std::vector<char> sendBuf(packetSize);

        // Fill with pattern: 255-(i&255), first byte acts as message ID
        for (int i = 0; i < packetSize; i++)
            sendBuf[i] = static_cast<char>(255 - (i & 255));

        // Wait for client to connect, then server sends
        bool clientConnected = false;
        bool serverSawClient = false;
        SystemAddress clientAddr;

        Time deadline = GetTime() + 5000;
        while (GetTime() < deadline && !(clientConnected && serverSawClient)) {
            for (Packet *p = server->Receive(); p;
                 server->DeallocatePacket(p), p = server->Receive()) {
                if (p->data[0] == ID_NEW_INCOMING_CONNECTION) {
                    clientAddr = p->systemAddress;
                    serverSawClient = true;
                }
            }
            for (Packet *p = client->Receive(); p;
                 client->DeallocatePacket(p), p = client->Receive()) {
                if (p->data[0] == ID_CONNECTION_REQUEST_ACCEPTED)
                    clientConnected = true;
            }
            RakSleep(10);
        }
        ASSERT_TRUE(clientConnected) << "Client did not connect";
        ASSERT_TRUE(serverSawClient) << "Server did not see client";

        // Server sends the big packet
        unsigned int bytesSent = server->Send(
            sendBuf.data(), packetSize, LOW_PRIORITY,
            RELIABLE_ORDERED_WITH_ACK_RECEIPT, 0, clientAddr, false);
        ASSERT_GT(bytesSent, 0u) << "Send returned 0";

        // Client receives and verifies
        bool received = false;
        int progressCount = 0;

        // Allow generous time for large packets
        int timeoutMs = 5000 + packetSize / 10000;
        Time recvDeadline = GetTime() + timeoutMs;
        while (GetTime() < recvDeadline && !received) {
            for (Packet *p = client->Receive(); p;
                 client->DeallocatePacket(p), p = client->Receive()) {
                if (p->data[0] == ID_DOWNLOAD_PROGRESS) {
                    progressCount++;
                } else if (p->data[0] == static_cast<unsigned char>(255)) {
                    EXPECT_EQ(static_cast<int>(p->length), packetSize)
                        << "Received wrong number of bytes";

                    if (verifyData && p->length == static_cast<unsigned int>(packetSize)) {
                        for (int i = 0; i < packetSize; i++) {
                            if (p->data[i] != static_cast<unsigned char>(255 - (i & 255))) {
                                FAIL() << "Data mismatch at byte " << i
                                       << ": expected " << (255 - (i & 255))
                                       << " got " << static_cast<int>(p->data[i]);
                            }
                        }
                    }
                    received = true;
                } else if (p->data[0] == ID_CONNECTION_LOST ||
                           p->data[0] == ID_DISCONNECTION_NOTIFICATION) {
                    FAIL() << "Lost connection during transfer";
                }
            }
            // Drain server packets too
            for (Packet *p = server->Receive(); p;
                 server->DeallocatePacket(p), p = server->Receive()) {
            }
            RakSleep(10);
        }

        EXPECT_TRUE(received) << "Did not receive big packet within timeout";
    }
};

TEST_F(BigPacketTest, SmallBigPacket) {
    // 50KB with full data verification
    SendAndVerifyBigPacket(50000, true);
}

TEST_F(BigPacketTest, MediumBigPacket) {
    // 500KB with full data verification
    SendAndVerifyBigPacket(500000, true);
}

TEST_F(BigPacketTest, LargeBigPacket) {
    // 5MB with length-only check (mirrors original behavior for large sizes)
    SendAndVerifyBigPacket(5000000, false);
}
