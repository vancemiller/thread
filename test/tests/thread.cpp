#include "thread.hpp"
#include "gtest/gtest.h"

namespace wrapper {

int hello_world_main(int argc, char** argv) {
  EXPECT_EQ(argc, 1);
  EXPECT_STREQ("hello_world", argv[0]);
  return 1234;
}

TEST(Thread, ConstructDestruct) {
  Thread(hello_world_main, {"hello_world"});
}

TEST(Thread, ReturnValue) {
  Thread hello(hello_world_main, {"hello_world"});
  EXPECT_EQ(1234, hello.wait());
}

TEST(Thread, WaitTwice) {
  Thread hello(hello_world_main, {"hello_world"});
  hello.wait();
  EXPECT_THROW(hello.wait(), std::system_error);
}

TEST(Thread, Kill) {
  Thread hello(hello_world_main, {"hello_world"});
  hello.kill();
  hello.wait();
}

TEST(Thread, KillAfterWait) {
  Thread hello(hello_world_main, {"hello_world"});
  hello.wait();
  EXPECT_THROW(hello.kill(), std::system_error);
}

int infinite_loop_main(int argc, char** argv) {
  while (true)
    ;
  return 0;
}

TEST(Thread, InfiniteLoopKill) {
  Thread inf(infinite_loop_main, {"infinite_loop"});
  inf.kill();
  EXPECT_EQ(-1, inf.wait());
}

TEST(Thread, KillTwice) {
  Thread inf(infinite_loop_main, {"infinite_loop"});
  inf.kill();
  EXPECT_NO_THROW(inf.kill()); // can call kill as much as we want
  EXPECT_EQ(-1, inf.wait());
}

TEST(Thread, Move) {
  Thread inf(infinite_loop_main, {"infinite_loop"});
  Thread moved(std::move(inf));
  EXPECT_THROW(inf.kill(), std::system_error);
  EXPECT_THROW(inf.wait(), std::system_error);
  moved.kill();
  moved.wait();
}
} // namespace wrapper
