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

static constexpr unsigned char RELAY_MARKER = 255;

/**
 * @brief Measures effective throughput of RakNet via a three-system loopback
 * (source -> relay -> destination) at configurable packet rates and sizes.
 */
class LoopbackPerformanceTest : public ::testing::Test {
protected:
    RakPeerInterface *destination = nullptr;
    RakPeerInterface *relay = nullptr;
    RakPeerInterface *source = nullptr;
    unsigned short destPort = 0;
    unsigned short relayPort = 0;

    void SetUp() override {
        destination = RakPeerInterface::GetInstance();
        relay = RakPeerInterface::GetInstance();
        source = RakPeerInterface::GetInstance();
        ASSERT_NE(destination, nullptr);
        ASSERT_NE(relay, nullptr);
        ASSERT_NE(source, nullptr);

        // Start destination
        destination->SetMaximumIncomingConnections(1);
        SocketDescriptor sdDest(0, nullptr);
        ASSERT_EQ(destination->Startup(1, &sdDest, 1), RAKNET_STARTED);
        destPort = destination->GetMyBoundAddress().GetPort();

        // Start relay, connect to destination
        relay->SetMaximumIncomingConnections(1);
        SocketDescriptor sdRelay(0, nullptr);
        ASSERT_EQ(relay->Startup(2, &sdRelay, 1), RAKNET_STARTED);
        relayPort = relay->GetMyBoundAddress().GetPort();
        ASSERT_EQ(relay->Connect("127.0.0.1", destPort, nullptr, 0),
                  CONNECTION_ATTEMPT_STARTED);

        // Wait for relay->destination connection
        bool relayConnected = false;
        auto deadline = Clock::now() + std::chrono::seconds(5);
        while (Clock::now() < deadline && !relayConnected) {
            for (Packet *p = relay->Receive(); p;
                 relay->DeallocatePacket(p), p = relay->Receive()) {
                if (p->data[0] == ID_CONNECTION_REQUEST_ACCEPTED)
                    relayConnected = true;
            }
            for (Packet *p = destination->Receive(); p;
                 destination->DeallocatePacket(p), p = destination->Receive()) {
            }
            RakSleep(10);
        }
        ASSERT_TRUE(relayConnected) << "Relay did not connect to destination";

        // Start source, connect to relay
        SocketDescriptor sdSrc(0, nullptr);
        ASSERT_EQ(source->Startup(1, &sdSrc, 1), RAKNET_STARTED);
        ASSERT_EQ(source->Connect("127.0.0.1", relayPort, nullptr, 0),
                  CONNECTION_ATTEMPT_STARTED);

        bool sourceConnected = false;
        deadline = Clock::now() + std::chrono::seconds(5);
        while (Clock::now() < deadline && !sourceConnected) {
            for (Packet *p = source->Receive(); p;
                 source->DeallocatePacket(p), p = source->Receive()) {
                if (p->data[0] == ID_CONNECTION_REQUEST_ACCEPTED)
                    sourceConnected = true;
            }
            // Pump relay and destination
            for (Packet *p = relay->Receive(); p;
                 relay->DeallocatePacket(p), p = relay->Receive()) {
            }
            for (Packet *p = destination->Receive(); p;
                 destination->DeallocatePacket(p), p = destination->Receive()) {
            }
            RakSleep(10);
        }
        ASSERT_TRUE(sourceConnected) << "Source did not connect to relay";
    }

    void TearDown() override {
        if (source) RakPeerInterface::DestroyInstance(source);
        if (relay) RakPeerInterface::DestroyInstance(relay);
        if (destination) RakPeerInterface::DestroyInstance(destination);
    }

    // Relay forwards packets with RELAY_MARKER to the other connection
    void PumpRelay(PacketReliability reliability) {
        for (Packet *p = relay->Receive(); p;
             relay->DeallocatePacket(p), p = relay->Receive()) {
            if (p->data[0] == RELAY_MARKER) {
                relay->Send(reinterpret_cast<const char *>(p->data),
                            p->length, HIGH_PRIORITY, reliability,
                            0, p->systemAddress, true);
            }
        }
    }

    struct ThroughputResult {
        unsigned int sent;
        unsigned int received;
    };

    ThroughputResult RunThroughput(unsigned int packetsPerSecond,
                                   unsigned int bytesPerPacket,
                                   PacketReliability reliability,
                                   int durationMs) {
        unsigned char data[4096];
        memset(data, 0, sizeof(data));
        data[0] = RELAY_MARKER;

        unsigned int totalSent = 0;
        unsigned int totalReceived = 0;

        auto startTime = Clock::now();
        auto endTime = startTime + std::chrono::milliseconds(durationMs);
        auto lastSendTime = startTime;

        while (Clock::now() < endTime) {
            auto now = Clock::now();

            // Calculate how many packets to send this tick
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - lastSendTime).count();
            unsigned int num = static_cast<unsigned int>(
                packetsPerSecond * elapsed / 1000);

            if (num > 0) {
                for (unsigned int i = 0; i < num; i++) {
                    source->Send(reinterpret_cast<const char *>(data),
                                 bytesPerPacket, HIGH_PRIORITY, reliability,
                                 0, UNASSIGNED_SYSTEM_ADDRESS, true);
                    totalSent++;
                }
                lastSendTime = now;
            }

            PumpRelay(reliability);

            // Drain source
            for (Packet *p = source->Receive(); p;
                 source->DeallocatePacket(p), p = source->Receive()) {
            }

            // Count destination arrivals
            for (Packet *p = destination->Receive(); p;
                 destination->DeallocatePacket(p), p = destination->Receive()) {
                if (p->data[0] == RELAY_MARKER)
                    totalReceived++;
            }

            RakSleep(10);
        }

        // Drain remaining packets
        auto drainDeadline = Clock::now() + std::chrono::seconds(3);
        while (Clock::now() < drainDeadline && totalReceived < totalSent) {
            PumpRelay(reliability);
            for (Packet *p = destination->Receive(); p;
                 destination->DeallocatePacket(p), p = destination->Receive()) {
                if (p->data[0] == RELAY_MARKER)
                    totalReceived++;
            }
            for (Packet *p = source->Receive(); p;
                 source->DeallocatePacket(p), p = source->Receive()) {
            }
            RakSleep(10);
        }

        return {totalSent, totalReceived};
    }
};

TEST_F(LoopbackPerformanceTest, ReliableOrdered) {
    // 500 packets/sec, 400 bytes each, RELIABLE_ORDERED, 3 seconds
    auto result = RunThroughput(500, 400, RELIABLE_ORDERED, 3000);

    EXPECT_GT(result.sent, 0u) << "No packets were sent";
    EXPECT_EQ(result.received, result.sent)
        << "RELIABLE_ORDERED should deliver all packets; got "
        << result.received << "/" << result.sent;
}

TEST_F(LoopbackPerformanceTest, Reliable) {
    // 500 packets/sec, 400 bytes each, RELIABLE, 3 seconds
    auto result = RunThroughput(500, 400, RELIABLE, 3000);

    EXPECT_GT(result.sent, 0u);
    EXPECT_EQ(result.received, result.sent)
        << "RELIABLE should deliver all packets; got "
        << result.received << "/" << result.sent;
}

TEST_F(LoopbackPerformanceTest, Unreliable) {
    // 500 packets/sec, 400 bytes each, UNRELIABLE, 3 seconds
    // Unreliable may lose packets on loopback but should deliver most
    auto result = RunThroughput(500, 400, UNRELIABLE, 3000);

    EXPECT_GT(result.sent, 0u);
    EXPECT_GT(result.received, result.sent / 2)
        << "Even unreliable should deliver most packets on loopback; got "
        << result.received << "/" << result.sent;
}
