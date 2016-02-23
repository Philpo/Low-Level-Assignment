#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <functional>
#include <deque>

class ThreadPool;
typedef std::function<void (int, int, int, float, float, float, float)> rayTraceFunc;

struct RayData {
  int x, y, width;
  float invHeight, invWidth, angle, aspectratio;
};

// our worker thread objects
class Worker {
public:
  Worker(ThreadPool &s) : pool(s) {}
  void operator()();
private:
  ThreadPool &pool;
};

// the actual thread pool
class ThreadPool {
public:
  ThreadPool(size_t);
  void enqueue(rayTraceFunc f, RayData data);
  ~ThreadPool();
private:
  friend class Worker;

  // need to keep track of threads so we can join them
  std::vector<std::thread> workers;

  // the task queue
  std::deque<rayTraceFunc> tasks;
  std::deque<RayData> taskData;

  // synchronization
  std::mutex queue_mutex;
  std::condition_variable condition;
  bool stop;
};