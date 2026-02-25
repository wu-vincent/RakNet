/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <array>

#include <gtest/gtest.h>

#include "RakPeerInterface.h"
#include "GetTime.h"
#include "MessageIdentifiers.h"
#include "RakSleep.h"

using namespace RakNet;

class MessageSizeTest : public ::testing::Test {
protected:
    RakPeerInterface *sender = nullptr;
    RakPeerInterface *receiver = nullptr;

    void SetUp() override {
        sender = RakPeerInterface::GetInstance();
        ASSERT_NE(sender, nullptr);

        receiver = RakPeerInterface::GetInstance();
        ASSERT_NE(receiver, nullptr);

        SocketDescriptor sd1(0, nullptr);
        ASSERT_EQ(receiver->Startup(32, &sd1, 1), RAKNET_STARTED);
        receiver->SetMaximumIncomingConnections(32);
        unsigned short receiverPort = receiver->GetMyBoundAddress().GetPort();

        SocketDescriptor sd2(0, nullptr);
        ASSERT_EQ(sender->Startup(1, &sd2, 1), RAKNET_STARTED);
        ASSERT_EQ(sender->Connect("127.0.0.1", receiverPort, nullptr, 0), CONNECTION_ATTEMPT_STARTED);

        // Wait for connection to be fully established
        RakSleep(100);
    }

    void TearDown() override {
        if (sender) { RakPeerInterface::DestroyInstance(sender); }
        if (receiver) {
            RakPeerInterface::DestroyInstance(receiver);
        }
    }

    void SendAndVerifyStride(int stride) {
        unsigned char data[4000];
        data[0] = ID_USER_PACKET_ENUM;
        for (unsigned int i = 1; i < 4000; i++) {
            data[i] = i % 256;
        }

        int sendCount = 0;
        for (int sum = 0; sum < 4000; sum += stride) {
            sender->Send((const char *) data, stride, HIGH_PRIORITY,
                         RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
            sendCount++;
        }


        for (Packet *p = sender->Receive(); p;
             sender->DeallocatePacket(p), p = sender->Receive()) {
            // Drain sender's receive queue
        }

        int receiveCount = 0;
        RakNet::Time timeout = RakNet::GetTime() + 1000;
        while (RakNet::GetTime() < timeout) {
            for (Packet *p = receiver->Receive(); p;
                 receiver->DeallocatePacket(p), p = receiver->Receive()) {
                if (p->data[0] == ID_USER_PACKET_ENUM) {
                    receiveCount++;
                    for (unsigned int i = 1; i < p->length; i++) {
                        EXPECT_EQ(p->data[i], i % 256)
                            << "Data mismatch at byte " << i
                            << " for stride " << stride;
                    }
                }
            }
            RakSleep(30);
            if (receiveCount == sendCount) {
                break;
            }
        }

        EXPECT_EQ(receiveCount, sendCount)
            << "Stride " << stride << ": sent " << sendCount
            << " but received " << receiveCount;
    }
};

TEST_F(MessageSizeTest, RepresentativeStrides) {
    for (int stride: {1, 10, 100, 500, 999, 1500, 1999}) {
        SCOPED_TRACE("stride=" + std::to_string(stride));
        SendAndVerifyStride(stride);
    }
}
