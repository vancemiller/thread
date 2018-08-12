#include "thread.hpp"
#include "lock.hpp"

#include <cstring>
#include <csignal>
#include <exception>
#include <fstream>
#include <iostream>
#include <mutex>
#include <unistd.h>


static struct ThreadArgs {
  const std::function<int(int, char**)>& main;
  const int argc;
  const std::unique_ptr<char*[]>& argv;
  mutable bool started = false;
  mutable Mutex mutex;
  mutable Condition started_cond;
}

static int thread_function(const void* arg) {
  const ThreadArgs* args = static_cast<const ThreadArgs*>(arg);
  {
    std::lock_guard<Mutex> lock(args->mutex);
    args->started = true;
    args->started_cond.broadcast();
  }
  return args->main(args->argc, args->argv);
}

static std::unique_ptr<char*[]> build_args(const std::list<std::string>& args) {
  std::unique_ptr<char*[]> argv = std::make_unique<char*[]>(args.size() + 1);
  size_t i = 0;
  for (auto it : args) {
    argv[i] = new char[it.length() + 1];
    std::strncpy(argv[i], it.c_str(), it.length() + 1);
    i++;
  }
  argv[args.size()] = NULL;
  return argv;
}

static pthread_t spawn_thread(const std::function<int(int, char**)>& main, int argc,
    std::unique_ptr<char*[]>& argv) {
  ThreadArgs args{main, argc, argv};
  int err;
  if ((err = pthread_create(&action_thread, NULL, thread_function, &args)))
    throw std::system_error(err, std::generic_category(), "pthread_create failed");
  std::lock_guard<Mutex> lock(args.mutex);
  while (!args->started) args.started_cond.wait(args.mutex);
}

Thread::Thread(const std::function<int(int, char**)> main, const std::list<std::string>& args) :
    Thread(main, args.size(), build_args(args)) {}

Thread::Thread(const std::function<int(int, char**)> main, int argc,
    std::unique_ptr<char*[]> argv) : main(main), argc(argc), argv(argv),
    thread(spawn_thread(main, argc, argv)) {}

Thread::Thread(Thread&& o) {
  // TODO
}

Thread::~Thread(void) {

}

Thread::Thread(std::function<int(void)> action, std::function<void(void)> cancel) :
      action(action), cancel(cancel), joined(false), thread_started(false) {
}

Thread::Thread(Action& a) : Thread([&a] (void) -> int {return a.main(); },
   [&a] (void) -> void { a.cancel(); }) {}

Thread::~Thread(void) {
  if (!joined) {
    cancel();
    join();
  }
}

void Thread::run_action(void) {
  {
    std::lock_guard<Mutex> lock(thread_started_mutex);
    thread_started = true;
    thread_started_cond.broadcast();
  }
  if (std::signal(SIGPIPE, SIG_IGN) == SIG_ERR) throw std::runtime_error("Ignore SIGPIPE failed.");
  int ret = action();
}

void Thread::join(void) {
  int err;
  if (!joined && (err = pthread_join(action_thread, NULL))) {
    throw std::runtime_error(std::string("Thread join failed: ") + strerror(err) + " " +
        std::to_string(action_thread) + ".");
  }
  joined = true;
}

const pthread_t Thread::id(void) const {
  return action_thread;
}
