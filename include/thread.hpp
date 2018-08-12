#ifndef THREAD_HPP
#define THREAD_HPP

#include <functional>
#include <string>
#include <pthread.h>

class Thread {
  private:
    const std::function<int(int, char**)> main;
    const int argc;
    const std::unique_ptr<char*[]> argv;
    pthread_t thread;
    bool joined;
    bool thread_started;
  private:
    static void* thread_function(void* arg);
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
    const pthread_t gettid(void) const;
    void join(void) const;
    void kill(void);
};

#endif
