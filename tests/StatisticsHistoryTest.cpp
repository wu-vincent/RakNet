/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <cmath>

#include <gtest/gtest.h>

#include "GetTime.h"
#include "StatisticsHistory.h"
#include "RakSleep.h"

using namespace RakNet;

enum HistoryObject {
    HO_SIN_WAVE,
    HO_COS_WAVE,
};

class StatisticsHistoryTest : public ::testing::Test {
protected:
    StatisticsHistory history;

    void SetUp() override {
        history.SetDefaultTimeToTrack(10000);
        ASSERT_TRUE(history.AddObject(
            StatisticsHistory::TrackedObjectData(HO_SIN_WAVE, 0, nullptr)));
        ASSERT_TRUE(history.AddObject(
            StatisticsHistory::TrackedObjectData(HO_COS_WAVE, 0, nullptr)));
    }
};

TEST_F(StatisticsHistoryTest, AddAndRetrieveValues) {
    Time now = GetTime();

    history.AddValueByObjectID(HO_SIN_WAVE, "Waveform", 0.5, now, false);
    history.AddValueByObjectID(HO_SIN_WAVE, "Waveform", 1.0, now + 100, false);
    history.AddValueByObjectID(HO_SIN_WAVE, "Waveform", -0.5, now + 200, false);

    StatisticsHistory::TimeAndValueQueue *tav = nullptr;
    auto result = history.GetHistoryForKey(HO_SIN_WAVE, "Waveform", &tav, now + 300);
    ASSERT_EQ(result, StatisticsHistory::SH_OK);
    ASSERT_NE(tav, nullptr);
    EXPECT_EQ(tav->values.Size(), 3u);
}

TEST_F(StatisticsHistoryTest, LongTermTracking) {
    Time now = GetTime();

    for (int i = 0; i < 50; i++) {
        double val = sin(static_cast<double>(i) / 10.0);
        history.AddValueByObjectID(HO_SIN_WAVE, "Waveform", val, now + i * 50, false);
    }

    StatisticsHistory::TimeAndValueQueue *tav = nullptr;
    auto result = history.GetHistoryForKey(HO_SIN_WAVE, "Waveform", &tav, now + 2500);
    ASSERT_EQ(result, StatisticsHistory::SH_OK);

    EXPECT_LE(tav->GetLongTermHighest(), 1.0);
    EXPECT_GE(tav->GetLongTermLowest(), -1.0);
    EXPECT_EQ(tav->GetLongTermSum(), tav->GetRecentSum())
        << "All values are within tracking window, sums should match";
}

TEST_F(StatisticsHistoryTest, MergeObjectsOnKey) {
    Time now = GetTime();

    for (int i = 0; i < 20; i++) {
        double t = static_cast<double>(i) / 10.0;
        history.AddValueByObjectID(HO_SIN_WAVE, "Waveform", sin(t), now + i * 50, false);
        history.AddValueByObjectID(HO_COS_WAVE, "Waveform", cos(t), now + i * 50, false);
    }

    StatisticsHistory::TimeAndValueQueue merged;
    history.MergeAllObjectsOnKey("Waveform", &merged, StatisticsHistory::DC_CONTINUOUS);

    EXPECT_GT(merged.values.Size(), 0u) << "Merged result should contain data points";
}

TEST_F(StatisticsHistoryTest, ResizeSampleSet) {
    Time now = GetTime();

    for (int i = 0; i < 100; i++) {
        double val = sin(static_cast<double>(i) / 10.0);
        history.AddValueByObjectID(HO_SIN_WAVE, "Waveform", val, now + i * 20, false);
    }

    StatisticsHistory::TimeAndValueQueue *tav = nullptr;
    auto result = history.GetHistoryForKey(HO_SIN_WAVE, "Waveform", &tav, now + 2000);
    ASSERT_EQ(result, StatisticsHistory::SH_OK);

    DataStructures::Queue<StatisticsHistory::TimeAndValue> resampled;
    tav->ResizeSampleSet(10, resampled, StatisticsHistory::DC_CONTINUOUS);

    EXPECT_GT(resampled.Size(), 0u);
    EXPECT_LE(resampled.Size(), 15u)
        << "Resampled set should be approximately 10 samples";
}

TEST_F(StatisticsHistoryTest, UnknownKeyReturnsError) {
    StatisticsHistory::TimeAndValueQueue *tav = nullptr;
    auto result = history.GetHistoryForKey(HO_SIN_WAVE, "NonexistentKey", &tav, GetTime());
    EXPECT_EQ(result, StatisticsHistory::SH_UKNOWN_KEY);
}
