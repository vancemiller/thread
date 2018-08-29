#ifndef THREAD_HPP
#define THREAD_HPP

#include "lock.hpp"

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <pthread.h>

namespace wrapper {
class Thread {
  private:
    struct ThreadArgs {
      const std::function<int(void)> main;
      mutable bool started = false;
      mutable Mutex mutex;
      mutable Condition started_cond;
    };
    ThreadArgs args;
    mutable pthread_t thread;
  private:
    static void* thread_function(void* arg);
    static pthread_t spawn_thread(ThreadArgs& args);
    void run_action(void);
  public:
    Thread(const std::function<int(void)> main);
    Thread(void) = delete;
    Thread(const Thread&) = delete;
    Thread(Thread&) = delete;
    Thread(Thread&& o);
    ~Thread(void);
  public:
    int wait(void) const;
    void kill(void);
    pthread_t gettid(void) const noexcept { return thread; }
    bool operator==(const Thread& o) const noexcept { return thread == o.thread; }
    bool operator!=(const Thread& o) const noexcept { return thread != o.thread; }
};
} // namespace wrapper
#endif
