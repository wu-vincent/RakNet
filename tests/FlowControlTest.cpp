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
#include "MessageIdentifiers.h"
#include "RakSleep.h"

using namespace RakNet;
using Clock = std::chrono::steady_clock;

/**
 * @brief Tests network flow control and reliability by sending data through a
 * relay at variable rates and packet sizes, verifying that packets arrive
 * without prolonged gaps.
 */
class FlowControlTest : public ::testing::Test {
protected:
    RakPeerInterface *relay = nullptr;
    RakPeerInterface *sender = nullptr;
    RakPeerInterface *receiver = nullptr;
    unsigned short relayPort = 0;

    void SetUp() override {
        relay = RakPeerInterface::GetInstance();
        sender = RakPeerInterface::GetInstance();
        receiver = RakPeerInterface::GetInstance();
        ASSERT_NE(relay, nullptr);
        ASSERT_NE(sender, nullptr);
        ASSERT_NE(receiver, nullptr);

        relay->SetMaximumIncomingConnections(8);

        SocketDescriptor sdRelay(0, nullptr), sdSender(0, nullptr), sdRecv(0, nullptr);
        ASSERT_EQ(relay->Startup(8, &sdRelay, 1), RAKNET_STARTED);
        ASSERT_EQ(sender->Startup(1, &sdSender, 1), RAKNET_STARTED);
        ASSERT_EQ(receiver->Startup(1, &sdRecv, 1), RAKNET_STARTED);

        relayPort = relay->GetMyBoundAddress().GetPort();

        ASSERT_EQ(sender->Connect("127.0.0.1", relayPort, nullptr, 0),
                  CONNECTION_ATTEMPT_STARTED);
        ASSERT_EQ(receiver->Connect("127.0.0.1", relayPort, nullptr, 0),
                  CONNECTION_ATTEMPT_STARTED);

        // Wait for both to connect
        bool sConn = false, rConn = false;
        auto deadline = Clock::now() + std::chrono::seconds(5);
        while (Clock::now() < deadline && !(sConn && rConn)) {
            DrainRelay();
            for (Packet *p = sender->Receive(); p;
                 sender->DeallocatePacket(p), p = sender->Receive()) {
                if (p->data[0] == ID_CONNECTION_REQUEST_ACCEPTED) sConn = true;
            }
            for (Packet *p = receiver->Receive(); p;
                 receiver->DeallocatePacket(p), p = receiver->Receive()) {
                if (p->data[0] == ID_CONNECTION_REQUEST_ACCEPTED) rConn = true;
            }
            RakSleep(10);
        }
        ASSERT_TRUE(sConn) << "Sender did not connect to relay";
        ASSERT_TRUE(rConn) << "Receiver did not connect to relay";
    }

    void TearDown() override {
        if (sender) RakPeerInterface::DestroyInstance(sender);
        if (receiver) RakPeerInterface::DestroyInstance(receiver);
        if (relay) RakPeerInterface::DestroyInstance(relay);
    }

    // Relay forwards user packets to all other connections (like the original)
    void PumpRelay() {
        for (Packet *p = relay->Receive(); p;
             relay->DeallocatePacket(p), p = relay->Receive()) {
            if (p->data[0] >= ID_USER_PACKET_ENUM) {
                relay->Send(reinterpret_cast<const char *>(p->data),
                            p->length, HIGH_PRIORITY, RELIABLE_ORDERED,
                            0, p->systemAddress, true);
            }
        }
    }

    void DrainRelay() {
        for (Packet *p = relay->Receive(); p;
             relay->DeallocatePacket(p), p = relay->Receive()) {
        }
    }
};

TEST_F(FlowControlTest, RelayedDataArrives) {
    char data[64];
    memset(data, 255, sizeof(data));
    data[0] = static_cast<char>(ID_USER_PACKET_ENUM);

    int sendCount = 0;
    int recvCount = 0;
    auto endTime = Clock::now() + std::chrono::seconds(3);
    auto nextSend = Clock::now();

    while (Clock::now() < endTime) {
        PumpRelay();

        // Sender sends at ~128ms intervals
        if (Clock::now() >= nextSend) {
            sender->Send(data, sizeof(data), HIGH_PRIORITY, RELIABLE_ORDERED,
                         0, UNASSIGNED_SYSTEM_ADDRESS, true);
            sendCount++;
            nextSend = Clock::now() + std::chrono::milliseconds(128);
        }

        // Drain sender
        for (Packet *p = sender->Receive(); p;
             sender->DeallocatePacket(p), p = sender->Receive()) {
        }

        // Receiver counts arrivals
        for (Packet *p = receiver->Receive(); p;
             receiver->DeallocatePacket(p), p = receiver->Receive()) {
            if (p->data[0] == ID_USER_PACKET_ENUM)
                recvCount++;
        }

        RakSleep(10);
    }

    // Allow remaining packets to drain
    auto drainDeadline = Clock::now() + std::chrono::seconds(2);
    while (Clock::now() < drainDeadline) {
        PumpRelay();
        for (Packet *p = receiver->Receive(); p;
             receiver->DeallocatePacket(p), p = receiver->Receive()) {
            if (p->data[0] == ID_USER_PACKET_ENUM)
                recvCount++;
        }
        for (Packet *p = sender->Receive(); p;
             sender->DeallocatePacket(p), p = sender->Receive()) {
        }
        if (recvCount >= sendCount) break;
        RakSleep(10);
    }

    EXPECT_GT(sendCount, 0) << "Nothing was sent";
    EXPECT_EQ(recvCount, sendCount)
        << "Receiver got " << recvCount << "/" << sendCount << " packets through relay";
}

TEST_F(FlowControlTest, VariablePacketSizes) {
    // Test multiple packet sizes like the original's interactive +/- controls
    int sizes[] = {64, 256, 1024, 4096};

    for (int packetSize : sizes) {
        SCOPED_TRACE("packetSize=" + std::to_string(packetSize));

        char data[4096];
        memset(data, 255, sizeof(data));
        data[0] = static_cast<char>(ID_USER_PACKET_ENUM);

        int sendCount = 0;
        int recvCount = 0;
        auto endTime = Clock::now() + std::chrono::milliseconds(500);
        auto nextSend = Clock::now();

        while (Clock::now() < endTime) {
            PumpRelay();

            if (Clock::now() >= nextSend) {
                sender->Send(data, packetSize, HIGH_PRIORITY, RELIABLE_ORDERED,
                             0, UNASSIGNED_SYSTEM_ADDRESS, true);
                sendCount++;
                nextSend = Clock::now() + std::chrono::milliseconds(64);
            }

            for (Packet *p = sender->Receive(); p;
                 sender->DeallocatePacket(p), p = sender->Receive()) {
            }
            for (Packet *p = receiver->Receive(); p;
                 receiver->DeallocatePacket(p), p = receiver->Receive()) {
                if (p->data[0] == ID_USER_PACKET_ENUM) {
                    EXPECT_EQ(p->length, static_cast<unsigned int>(packetSize));
                    recvCount++;
                }
            }
            RakSleep(5);
        }

        // Drain remaining
        auto drain = Clock::now() + std::chrono::seconds(2);
        while (Clock::now() < drain && recvCount < sendCount) {
            PumpRelay();
            for (Packet *p = receiver->Receive(); p;
                 receiver->DeallocatePacket(p), p = receiver->Receive()) {
                if (p->data[0] == ID_USER_PACKET_ENUM)
                    recvCount++;
            }
            for (Packet *p = sender->Receive(); p;
                 sender->DeallocatePacket(p), p = sender->Receive()) {
            }
            RakSleep(10);
        }

        EXPECT_GT(sendCount, 0);
        EXPECT_EQ(recvCount, sendCount)
            << "Lost packets at size " << packetSize;
    }
}
