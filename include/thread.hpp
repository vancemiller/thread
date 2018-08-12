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
      const std::function<int(int, char**)> main;
      const int argc;
      std::unique_ptr<char*[]> argv;
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
    Thread(const std::function<int(int, char**)> main, const std::list<std::string>& args);
    Thread(const std::function<int(int, char**)> main, int argc, std::unique_ptr<char*[]> argv);
    Thread(void) = delete;
    Thread(const Thread&) = delete;
    Thread(Thread&) = delete;
    Thread(Thread&& o);
    ~Thread(void);
  public:
    pthread_t gettid(void) const;
    int wait(void) const;
    void kill(void);
};
} // namespace wrapper
#endif
