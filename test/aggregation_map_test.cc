/**
 * This file is part of the "FnordMetric" project
 *   Copyright (c) 2017 Paul Asmuth <paul@asmuth.com>
 *
 * FnordTable is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "unittest.h"
#include "fnordmetric/aggregation_service.h"
#include "fnordmetric/util/time.h"

using namespace fnordmetric;

UNIT_TEST(AggregationMapTest);

TEST_CASE(AggregationMapTest, TestSlotLookup, [] () {
  AggregationMap map;

  auto ts = WallClock::unixMicros();

  auto slot_a = map.getSlot(
      ts,
      10 * kMicrosPerSecond,
      ts + 10 * kMicrosPerSecond,
      "test_tbl",
      { { "test", "blah" } });

  EXPECT(slot_a != nullptr);

  auto slot_b = map.getSlot(
      ts + 10 * kMicrosPerSecond,
      10 * kMicrosPerSecond,
      ts + 20 * kMicrosPerSecond,
      "test_tbl",
      { { "test", "blah" } });

  EXPECT(slot_b != nullptr);
  EXPECT(slot_b != slot_a);

  auto slot_c = map.getSlot(
      ts,
      10 * kMicrosPerSecond,
      ts + 10 * kMicrosPerSecond,
      "test_tbl",
      { { "test", "blah" } });

  EXPECT(slot_c != nullptr);
  EXPECT(slot_c != slot_b);
  EXPECT(slot_c == slot_a);

  auto slot_d = map.getSlot(
      ts,
      10 * kMicrosPerSecond,
      ts + 10 * kMicrosPerSecond,
      "test_tbl",
      { { "test", "blahx" } });

  EXPECT(slot_d != nullptr);
  EXPECT(slot_d != slot_a);
  EXPECT(slot_d != slot_b);

  auto slot_e = map.getSlot(
      ts,
      10 * kMicrosPerSecond,
      ts + 10 * kMicrosPerSecond,
      "test_tbl",
      {});

  EXPECT(slot_e != nullptr);
  EXPECT(slot_e != slot_a);
  EXPECT(slot_e != slot_b);
  EXPECT(slot_e != slot_d);

  auto slot_f = map.getSlot(
      ts + 10 * kMicrosPerSecond,
      10 * kMicrosPerSecond,
      ts + 20 * kMicrosPerSecond,
      "test_tbl",
      {});

  EXPECT(slot_f != nullptr);
  EXPECT(slot_f != slot_a);
  EXPECT(slot_f != slot_b);
  EXPECT(slot_f != slot_d);
  EXPECT(slot_f != slot_e);

  auto slot_g = map.getSlot(
      ts,
      10 * kMicrosPerSecond,
      ts + 10 * kMicrosPerSecond,
      "test_tbl",
      {});

  EXPECT(slot_g != nullptr);
  EXPECT(slot_g != slot_a);
  EXPECT(slot_g != slot_b);
  EXPECT(slot_g != slot_d);
  EXPECT(slot_g == slot_e);
  EXPECT(slot_g != slot_f);
});

TEST_CASE(AggregationMapTest, TestExpiration, [] () {
  AggregationMap map;

  auto ts = WallClock::unixMicros();

  map.getSlot(
      ts + 30 * kMicrosPerSecond,
      10 * kMicrosPerSecond,
      ts + 40 * kMicrosPerSecond,
      "test_tbl",
      { { "slot", "4" } });

  map.getSlot(
      ts + 40 * kMicrosPerSecond,
      10 * kMicrosPerSecond,
      ts + 50 * kMicrosPerSecond,
      "test_tbl",
      { { "slot", "5" } });

  map.getSlot(
      ts + 50 * kMicrosPerSecond,
      10 * kMicrosPerSecond,
      ts + 60 * kMicrosPerSecond,
      "test_tbl",
      { { "slot", "6" } });

  map.getSlot(
      ts + 20 * kMicrosPerSecond,
      10 * kMicrosPerSecond,
      ts + 30 * kMicrosPerSecond,
      "test_tbl",
      { { "slot", "3" } });

  map.getSlot(
      ts,
      10 * kMicrosPerSecond,
      ts + 10 * kMicrosPerSecond,
      "test_tbl",
      { { "slot", "1" } });

  map.getSlot(
      ts + 10 * kMicrosPerSecond,
      10 * kMicrosPerSecond,
      ts + 20 * kMicrosPerSecond,
      "test_tbl",
      { { "slot", "2" } });

  std::vector<std::unique_ptr<AggregationMap::Slot>> expired;
  map.getExpiredSlots(ts, &expired);
  EXPECT(expired.size() == 0);

  map.getExpiredSlots(ts + 10 * kMicrosPerSecond, &expired);
  EXPECT(expired.size() == 1);
  EXPECT(expired[0]->labels.size() == 1);
  EXPECT(expired[0]->labels[0].first == "slot");
  EXPECT(expired[0]->labels[0].second == "1");
  expired.clear();

  map.getExpiredSlots(ts + 11 * kMicrosPerSecond, &expired);
  EXPECT(expired.size() == 0);

  map.getExpiredSlots(ts + 35 * kMicrosPerSecond, &expired);
  EXPECT(expired[0]->labels.size() == 1);
  EXPECT(expired[0]->labels[0].first == "slot");
  EXPECT(expired[0]->labels[0].second == "2");
  EXPECT(expired[1]->labels.size() == 1);
  EXPECT(expired[1]->labels[0].first == "slot");
  EXPECT(expired[1]->labels[0].second == "3");
  EXPECT(expired.size() == 2);
  expired.clear();

  map.getExpiredSlots(ts + 60 * kMicrosPerSecond, &expired);
  EXPECT(expired[0]->labels.size() == 1);
  EXPECT(expired[0]->labels[0].first == "slot");
  EXPECT(expired[0]->labels[0].second == "4");
  EXPECT(expired[1]->labels.size() == 1);
  EXPECT(expired[1]->labels[0].first == "slot");
  EXPECT(expired[1]->labels[0].second == "5");
  EXPECT(expired[2]->labels.size() == 1);
  EXPECT(expired[2]->labels[0].first == "slot");
  EXPECT(expired[2]->labels[0].second == "6");
  EXPECT(expired.size() == 3);
  expired.clear();

  map.getExpiredSlots(ts + 60 * kMicrosPerSecond, &expired);
  EXPECT(expired.size() == 0);

  map.getSlot(
      ts + 30 * kMicrosPerSecond,
      10 * kMicrosPerSecond,
      ts + 40 * kMicrosPerSecond,
      "test_tbl",
      { { "slot", "4" } });

  map.getSlot(
      ts + 40 * kMicrosPerSecond,
      10 * kMicrosPerSecond,
      ts + 50 * kMicrosPerSecond,
      "test_tbl",
      { { "slot", "5" } });

  map.getSlot(
      ts + 50 * kMicrosPerSecond,
      10 * kMicrosPerSecond,
      ts + 60 * kMicrosPerSecond,
      "test_tbl",
      { { "slot", "6" } });

  map.getSlot(
      ts + 20 * kMicrosPerSecond,
      10 * kMicrosPerSecond,
      ts + 30 * kMicrosPerSecond,
      "test_tbl",
      { { "slot", "3" } });

  map.getSlot(
      ts,
      10 * kMicrosPerSecond,
      ts + 10 * kMicrosPerSecond,
      "test_tbl",
      { { "slot", "1" } });

  map.getSlot(
      ts + 10 * kMicrosPerSecond,
      10 * kMicrosPerSecond,
      ts + 20 * kMicrosPerSecond,
      "test_tbl",
      { { "slot", "2" } });

  map.getExpiredSlots(ts, &expired);
  EXPECT(expired.size() == 0);

  map.getExpiredSlots(ts + 10 * kMicrosPerSecond, &expired);
  EXPECT(expired.size() == 1);
  EXPECT(expired[0]->labels.size() == 1);
  EXPECT(expired[0]->labels[0].first == "slot");
  EXPECT(expired[0]->labels[0].second == "1");
  expired.clear();

  map.getExpiredSlots(ts + 11 * kMicrosPerSecond, &expired);
  EXPECT(expired.size() == 0);

  map.getExpiredSlots(ts + 35 * kMicrosPerSecond, &expired);
  EXPECT(expired[0]->labels.size() == 1);
  EXPECT(expired[0]->labels[0].first == "slot");
  EXPECT(expired[0]->labels[0].second == "2");
  EXPECT(expired[1]->labels.size() == 1);
  EXPECT(expired[1]->labels[0].first == "slot");
  EXPECT(expired[1]->labels[0].second == "3");
  EXPECT(expired.size() == 2);
  expired.clear();

  map.getExpiredSlots(ts + 60 * kMicrosPerSecond, &expired);
  EXPECT(expired[0]->labels.size() == 1);
  EXPECT(expired[0]->labels[0].first == "slot");
  EXPECT(expired[0]->labels[0].second == "4");
  EXPECT(expired[1]->labels.size() == 1);
  EXPECT(expired[1]->labels[0].first == "slot");
  EXPECT(expired[1]->labels[0].second == "5");
  EXPECT(expired[2]->labels.size() == 1);
  EXPECT(expired[2]->labels[0].first == "slot");
  EXPECT(expired[2]->labels[0].second == "6");
  EXPECT(expired.size() == 3);
});
