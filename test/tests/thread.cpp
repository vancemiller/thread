#include "thread.hpp"
#include "gtest/gtest.h"

namespace wrapper {

int hello_world(int data) {
  EXPECT_EQ(1, data);
  return 1234;
}

TEST(Thread, ConstructDestruct) {
  Thread([] (void) -> int { return hello_world(1); });
}

TEST(Thread, ReturnValue) {
  Thread hello([] (void) -> int { return hello_world(1); });
  EXPECT_EQ(1234, hello.wait());
}

TEST(Thread, WaitTwice) {
  Thread hello([] (void) -> int { return hello_world(1); });
  hello.wait();
  EXPECT_THROW(hello.wait(), std::system_error);
}

TEST(Thread, Kill) {
  Thread hello([] (void) -> int { return hello_world(1); });
  hello.kill();
  hello.wait();
}

TEST(Thread, KillAfterWait) {
  Thread hello([] (void) -> int { return hello_world(1); });
  hello.wait();
  EXPECT_THROW(hello.kill(), std::system_error);
}

int infinite_loop(void) {
  while (true)
    ;
  return 0;
}

TEST(Thread, InfiniteLoopKill) {
  Thread inf([] (void) -> int { return infinite_loop(); });
  inf.kill();
  EXPECT_EQ(-1, inf.wait());
}

TEST(Thread, KillTwice) {
  Thread inf([] (void) -> int { return infinite_loop(); });
  inf.kill();
  EXPECT_NO_THROW(inf.kill()); // can call kill as much as we want
  EXPECT_EQ(-1, inf.wait());
}

TEST(Thread, Move) {
  Thread inf([] (void) -> int { return infinite_loop(); });
  Thread moved(std::move(inf));
  EXPECT_THROW(inf.kill(), std::system_error);
  EXPECT_THROW(inf.wait(), std::system_error);
  moved.kill();
  moved.wait();
}
} // namespace wrapper
