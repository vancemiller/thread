#include "thread.hpp"

#include <cstring>
#include <exception>
#include <mutex>

namespace wrapper {

void* Thread::thread_function(void* arg) {
  int err;
  if ((err = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL)))
    throw std::system_error(err, std::generic_category(), "pthread_setcanceltype failed");
  const ThreadArgs* args = static_cast<const ThreadArgs*>(arg);
  {
    std::lock_guard<Mutex> lock(args->mutex);
    args->started = true;
    args->started_cond.broadcast();
  }
  int* ret = new int;
  *ret = args->main();
  return static_cast<void*>(ret);
}

pthread_t Thread::spawn_thread(ThreadArgs& args) {
  int err;
  pthread_t thread;
  if ((err = pthread_create(&thread, NULL, thread_function, &args)))
    throw std::system_error(err, std::generic_category(), "pthread_create failed");
  std::lock_guard<Mutex> lock(args.mutex);
  while (!args.started) args.started_cond.wait(args.mutex);
  return thread;
}

Thread::Thread(const std::function<int(void)> main) : args{main}, thread(spawn_thread(args)) {}

Thread::Thread(Thread&& o) : args(std::move(o.args)), thread(o.thread) { o.thread = 0; }

Thread::~Thread(void) {
  if (thread) {
    wait();
  }
}

int Thread::wait(void) const {
  int err;
  void* ret;
  if ((err = pthread_join(thread, &ret)))
    throw std::system_error(err, std::generic_category(), "pthread_join failed");
  int retval;
  if (ret == PTHREAD_CANCELED)
    retval = -1;
  else {
    retval = *reinterpret_cast<int*>(ret);
    free(ret);
  }
  thread = 0;
  return retval;
}

void Thread::kill(void) {
  int err;
  if ((err = pthread_cancel(thread)))
    throw std::system_error(err, std::generic_category(), "pthread_cancel failed");
}

} // namespace wrapper
