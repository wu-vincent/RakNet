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

#include <gtest/gtest.h>

#include "RakPeerInterface.h"
#include "BitStream.h"
#include "MessageIdentifiers.h"
#include "RakSleep.h"

using namespace RakNet;
using Clock = std::chrono::steady_clock;

/**
 * @brief Tests sending messages to systems you are not connected to, including
 * AdvertiseSystem and offline pings.
 */
class OfflineMessagesTest : public ::testing::Test {
protected:
    RakPeerInterface *peer1 = nullptr;
    RakPeerInterface *peer2 = nullptr;

    void SetUp() override {
        peer1 = RakPeerInterface::GetInstance();
        ASSERT_NE(peer1, nullptr);

        peer2 = RakPeerInterface::GetInstance();
        ASSERT_NE(peer2, nullptr);

        peer1->SetMaximumIncomingConnections(1);

        SocketDescriptor sd1(0, nullptr);
        ASSERT_EQ(peer1->Startup(1, &sd1, 1), RAKNET_STARTED);

        SocketDescriptor sd2(0, nullptr);
        ASSERT_EQ(peer2->Startup(1, &sd2, 1), RAKNET_STARTED);
    }

    void TearDown() override {
        if (peer1) { RakPeerInterface::DestroyInstance(peer1); }
        if (peer2) { RakPeerInterface::DestroyInstance(peer2); }
    }
};

TEST_F(OfflineMessagesTest, AdvertiseAndPing) {
    const char *pingData = "Offline Ping Data";
    const char *advertiseData = "hello world";

    peer1->SetOfflinePingResponse(pingData, static_cast<int>(strlen(pingData)) + 1);

    // Verify GetOfflinePingResponse returns what we just set
    char *responseData = nullptr;
    unsigned int responseLen = 0;
    peer1->GetOfflinePingResponse(&responseData, &responseLen);
    ASSERT_NE(responseData, nullptr);
    EXPECT_STREQ(responseData, pingData);
    EXPECT_EQ(responseLen, static_cast<unsigned int>(strlen(pingData)) + 1);

    // Verify GUIDs are retrievable
    RakNetGUID guid1 = peer1->GetGuidFromSystemAddress(UNASSIGNED_SYSTEM_ADDRESS);
    RakNetGUID guid2 = peer2->GetGuidFromSystemAddress(UNASSIGNED_SYSTEM_ADDRESS);
    EXPECT_NE(guid1, UNASSIGNED_RAKNET_GUID);
    EXPECT_NE(guid2, UNASSIGNED_RAKNET_GUID);
    EXPECT_NE(guid1, guid2);

    unsigned short peer2Port = peer2->GetMyBoundAddress().GetPort();
    unsigned short peer1Port = peer1->GetMyBoundAddress().GetPort();

    RakSleep(100);
    peer1->AdvertiseSystem("127.0.0.1", peer2Port, advertiseData,
                           static_cast<int>(strlen(advertiseData)) + 1);

    // State machine: wait for advertise, then ping, then wait for pong
    bool gotAdvertise = false;
    bool gotPong = false;

    auto deadline = Clock::now() + std::chrono::seconds(5);
    while (Clock::now() < deadline) {
        peer1->DeallocatePacket(peer1->Receive());

        for (Packet *p = peer2->Receive(); p;
             peer2->DeallocatePacket(p), p = peer2->Receive()) {
            if (p->data[0] == ID_ADVERTISE_SYSTEM) {
                gotAdvertise = true;
                EXPECT_GT(p->length, 1u) << "Advertise packet should contain data";
                if (p->length > 1) {
                    EXPECT_STREQ(reinterpret_cast<const char *>(p->data + 1), advertiseData);
                }
                peer2->Ping("127.0.0.1", peer1Port, false);
            } else if (p->data[0] == ID_UNCONNECTED_PONG) {
                gotPong = true;
                unsigned int expectedDataLen = static_cast<unsigned int>(strlen(pingData)) + 1;
                unsigned int headerSize = sizeof(unsigned char) + sizeof(RakNet::TimeMS);
                EXPECT_EQ(p->length - headerSize, expectedDataLen)
                    << "Pong data length should match offline ping response";
                if (p->length > headerSize) {
                    EXPECT_STREQ(reinterpret_cast<const char *>(p->data + headerSize), pingData);
                }
            }
        }

        if (gotAdvertise && gotPong)
            break;
        RakSleep(30);
    }

    EXPECT_TRUE(gotAdvertise) << "Did not receive ID_ADVERTISE_SYSTEM";
    EXPECT_TRUE(gotPong) << "Did not receive ID_UNCONNECTED_PONG";
}
