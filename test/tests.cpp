// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include "TimedDoor.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Throw;

class MockDoor : public Door {
 public:
  MOCK_METHOD(void, lock, (), (override));
  MOCK_METHOD(void, unlock, (), (override));
  MOCK_METHOD(bool, isDoorOpened, (), (override));
};

class MockTimerClient : public TimerClient {
 public:
  MOCK_METHOD(void, Timeout, (), (override));
};

class MockTimer : public Timer {
 public:
  MOCK_METHOD(void, tregister, (int, TimerClient *), ());
  MOCK_METHOD(void, sleep, (int), ());
};

class TimedDoorTest : public ::testing::Test {
 protected:
  void SetUp() override { door = new TimedDoor(1000); }

  void TearDown() override { delete door; }

  TimedDoor *door;
};

TEST_F(TimedDoorTest, InitialStateIsLocked) {
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, UnlockChangesState) {
  door->unlock();
  EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, LockChangesState) {
  door->unlock();
  door->lock();
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, GetTimeoutReturnsCorrectValue) {
  EXPECT_EQ(door->getTimeOut(), 1000);
}

TEST_F(TimedDoorTest, AdapterTimeoutThrowsWhenDoorOpen) {
  door->unlock();
  EXPECT_THROW(door->throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, AdapterTimeoutDoesNotThrowWhenDoorClosed) {
  door->lock();
  EXPECT_NO_THROW(door->throwState());
}

TEST(TimerTest, RegisterCallsTimeout) {
  MockTimerClient client;
  Timer timer;

  EXPECT_CALL(client, Timeout()).Times(1);
  timer.tregister(100, &client);
}

TEST(IntegrationTest, DoorOpensAndCloses) {
  TimedDoor door(100);
  EXPECT_FALSE(door.isDoorOpened());

  door.unlock();
  EXPECT_TRUE(door.isDoorOpened());

  door.lock();
  EXPECT_FALSE(door.isDoorOpened());
}

TEST(IntegrationTest, DoorTimeoutThrowsWhenOpen) {
  TimedDoor door(100);
  door.unlock();

  Timer timer;
  DoorTimerAdapter adapter(door);

  EXPECT_THROW(timer.tregister(100, &adapter), std::runtime_error);
}

TEST(IntegrationTest, DoorTimeoutDoesNotThrowWhenClosed) {
  TimedDoor door(100);
  door.lock();

  Timer timer;
  DoorTimerAdapter adapter(door);

  EXPECT_NO_THROW(timer.tregister(100, &adapter));
}

