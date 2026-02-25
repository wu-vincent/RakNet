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

using namespace RakNet;

class BurstTest : public ::testing::Test {
protected:
    RakPeerInterface *sender = nullptr;
    RakPeerInterface *receiver = nullptr;

    void SetUp() override {
        sender = RakPeerInterface::GetInstance();
        ASSERT_NE(sender, nullptr);

        receiver = RakPeerInterface::GetInstance();
        ASSERT_NE(receiver, nullptr);

        receiver->SetMaximumIncomingConnections(32);

        SocketDescriptor sd1(0, nullptr), sd2(0, nullptr);
        ASSERT_EQ(receiver->Startup(32, &sd1, 1), RAKNET_STARTED);
        ASSERT_EQ(sender->Startup(1, &sd2, 1), RAKNET_STARTED);

        unsigned short receiverPort = receiver->GetMyBoundAddress().GetPort();
        ASSERT_EQ(sender->Connect("127.0.0.1", receiverPort, nullptr, 0),
                  CONNECTION_ATTEMPT_STARTED);
        RakSleep(100);
    }

    void TearDown() override {
        if (sender) { RakPeerInterface::DestroyInstance(sender); }
        if (receiver) { RakPeerInterface::DestroyInstance(receiver); }
    }

    void SendAndVerifyBurst(uint32_t msgSize, uint32_t msgCount) {
        // Send burst
        for (uint32_t index = 0; index < msgCount; index++) {
            BitStream bs;
            bs.Write(static_cast<MessageID>(ID_USER_PACKET_ENUM));
            bs.Write(msgSize);
            bs.Write(index);
            bs.Write(msgCount);
            bs.PadWithZeroToByteLength(msgSize);
            sender->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0,
                         UNASSIGNED_SYSTEM_ADDRESS, true);
        }

        // Drain sender queue
        for (Packet *p = sender->Receive(); p;
             sender->DeallocatePacket(p), p = sender->Receive()) {
        }

        // Receive and validate
        uint32_t received = 0;
        Time deadline = GetTime() + 5000;
        while (GetTime() < deadline) {
            for (Packet *p = receiver->Receive(); p;
                 receiver->DeallocatePacket(p), p = receiver->Receive()) {
                if (p->data[0] == ID_USER_PACKET_ENUM) {
                    uint32_t rxSize, rxIndex, rxCount;
                    BitStream bs(p->data, p->length, false);
                    bs.IgnoreBytes(sizeof(MessageID));
                    bs.Read(rxSize);
                    bs.Read(rxIndex);
                    bs.Read(rxCount);

                    EXPECT_EQ(rxSize, msgSize);
                    EXPECT_EQ(rxIndex, received)
                        << "Out-of-order packet: expected index " << received;
                    EXPECT_EQ(rxCount, msgCount);
                    EXPECT_GE(p->length, msgSize)
                        << "Packet " << rxIndex << " is underlength: "
                        << p->length << " < " << msgSize;
                    received++;
                }
            }
            if (received == msgCount)
                break;
            RakSleep(30);
        }

        EXPECT_EQ(received, msgCount)
            << "msgSize=" << msgSize << " msgCount=" << msgCount
            << ": received " << received << "/" << msgCount;
    }
};

TEST_F(BurstTest, SmallMessages) {
    SendAndVerifyBurst(64, 128);
}

TEST_F(BurstTest, MediumMessages) {
    SendAndVerifyBurst(512, 64);
}

TEST_F(BurstTest, LargeMessages) {
    SendAndVerifyBurst(4096, 16);
}
