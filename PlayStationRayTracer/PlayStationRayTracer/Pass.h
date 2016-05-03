#pragma once
#include "Move.h"
#include <thread>
#include <sce_fiber.h>
#include <libsysmodule.h>

using namespace sce;

struct Fiber {
  Fiber() : nextFiber(nullptr), directory(nullptr) {}

  SceFiber fiber;
  SceFiber* nextFiber;

  std::vector<Sphere> spheres;
  int iteration;
  std::string* directory;
  bool deferSaving;
};

const int CONTEXT_SIZE = 10000;

class Pass {
public:
	Pass(xml_node<>* passNode, std::string& directory, int passIndex, std::string& threadMethod, int numThreads, std::string& ioMethod);
  ~Pass() {}

  void render();
private:
  int numFrames, threadCount, passIndex;
  std::string directory, threadMethod, ioMethod;
  std::vector<Move> moves;
  void threadedDoPass(int threadIndex, int passIndex, int startIndex, int endIndex);
  void fiberDoPass(int passOffset, int startIndex, int endIndex);
  void doPass(int passOffset, int startIndex, int endIndex);

  __attribute__((noreturn))
    static void fiberEntryLeader(uint64_t argOnInitialize, uint64_t argOnRun);
  __attribute__((noreturn))
    static void fiberEntryFollower(uint64_t argOnInitialize, uint64_t argOnRun);
  __attribute__((noreturn))
    static void fiberEntryTail(uint64_t argOnInitialize, uint64_t argOnRun);
};