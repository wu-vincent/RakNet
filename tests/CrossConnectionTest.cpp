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
#include "GetTime.h"
#include "MessageIdentifiers.h"
#include "RakSleep.h"

using namespace RakNet;

class CrossConnectionTest : public ::testing::Test {
protected:
    RakPeerInterface *peer1 = nullptr;
    RakPeerInterface *peer2 = nullptr;

    void SetUp() override {
        peer1 = RakPeerInterface::GetInstance();
        ASSERT_NE(peer1, nullptr);

        peer2 = RakPeerInterface::GetInstance();
        ASSERT_NE(peer2, nullptr);

        peer1->SetMaximumIncomingConnections(8);
        peer2->SetMaximumIncomingConnections(8);
    }

    void TearDown() override {
        if (peer1) {
            peer1->Shutdown(0);
            RakPeerInterface::DestroyInstance(peer1);
        }
        if (peer2) {
            peer2->Shutdown(0);
            RakPeerInterface::DestroyInstance(peer2);
        }
    }
};

TEST_F(CrossConnectionTest, SimultaneousConnect) {
    SocketDescriptor sd1(0, nullptr), sd2(0, nullptr);
    ASSERT_EQ(peer1->Startup(1, &sd1, 1), RAKNET_STARTED);
    ASSERT_EQ(peer2->Startup(1, &sd2, 1), RAKNET_STARTED);

    unsigned short port1 = peer1->GetMyBoundAddress().GetPort();
    unsigned short port2 = peer2->GetMyBoundAddress().GetPort();

    RakSleep(100);
    peer1->Connect("127.0.0.1", port2, nullptr, 0);
    peer2->Connect("127.0.0.1", port1, nullptr, 0);

    // Wait for connection packets to arrive
    int gotConnectionRequestAccepted[2] = {0, 0};
    int gotNewIncomingConnection[2] = {0, 0};
    bool gotConnectionAttemptFailed = false;

    RakNet::Time deadline = RakNet::GetTime() + 5000;
    while (RakNet::GetTime() < deadline) {
        for (Packet *p = peer1->Receive(); p;
             peer1->DeallocatePacket(p), p = peer1->Receive()) {
            if (p->data[0] == ID_NEW_INCOMING_CONNECTION)
                gotNewIncomingConnection[0]++;
            else if (p->data[0] == ID_CONNECTION_REQUEST_ACCEPTED)
                gotConnectionRequestAccepted[0]++;
            else if (p->data[0] == ID_CONNECTION_ATTEMPT_FAILED)
                gotConnectionAttemptFailed = true;
        }
        for (Packet *p = peer2->Receive(); p;
             peer2->DeallocatePacket(p), p = peer2->Receive()) {
            if (p->data[0] == ID_NEW_INCOMING_CONNECTION)
                gotNewIncomingConnection[1]++;
            else if (p->data[0] == ID_CONNECTION_REQUEST_ACCEPTED)
                gotConnectionRequestAccepted[1]++;
            else if (p->data[0] == ID_CONNECTION_ATTEMPT_FAILED)
                gotConnectionAttemptFailed = true;
        }

        int totalEvents = gotConnectionRequestAccepted[0] + gotConnectionRequestAccepted[1]
                        + gotNewIncomingConnection[0] + gotNewIncomingConnection[1];
        if (totalEvents >= 2)
            break;
        RakSleep(30);
    }

    EXPECT_FALSE(gotConnectionAttemptFailed) << "Got ID_CONNECTION_ATTEMPT_FAILED";

    unsigned short numSystems[2] = {0, 0};
    peer1->GetConnectionList(nullptr, &numSystems[0]);
    peer2->GetConnectionList(nullptr, &numSystems[1]);

    EXPECT_EQ(numSystems[0], 1) << "Peer1 should have exactly 1 connection";
    EXPECT_EQ(numSystems[1], 1) << "Peer2 should have exactly 1 connection";

    int totalAccepted = gotConnectionRequestAccepted[0] + gotConnectionRequestAccepted[1];
    int totalIncoming = gotNewIncomingConnection[0] + gotNewIncomingConnection[1];

    // Cross-connection resolves as either:
    // - Both peers see CONNECTION_REQUEST_ACCEPTED (both outgoing connects succeed), or
    // - One sees CONNECTION_REQUEST_ACCEPTED and the other sees NEW_INCOMING_CONNECTION
    EXPECT_EQ(totalAccepted + totalIncoming, 2)
        << "Expected exactly 2 connection events total, got "
        << totalAccepted << " accepted + " << totalIncoming << " incoming";
    EXPECT_GE(totalAccepted, 1)
        << "At least one peer should get CONNECTION_REQUEST_ACCEPTED";
}
