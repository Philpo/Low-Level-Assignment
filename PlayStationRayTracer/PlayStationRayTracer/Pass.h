#pragma once
#include "Move.h"
#include <thread>
#include <sce_fiber.h>
//#include <kernel.h>
#include <libsysmodule.h>

using namespace sce;

struct Fiber {
  SceFiber fiber;
  SceFiber* nextFiber;

  std::vector<Sphere> spheres;
  int iteration;
  std::string directory;
  bool deferSaving;
};

#ifdef _DEBUG
const int CONTEXT_SIZE = 10000;
#else
const int CONTEXT_SIZE = 3686400;
#endif

class Pass {
public:
	Pass(xml_node<>* passNode, std::string& directory, int passIndex, std::string& threadMethod, int numThreads, std::string& ioMethod);
  ~Pass() {}

  void render();
private:
  int numFrames, threadCount, passIndex;
  std::string directory, threadMethod, ioMethod;
  std::vector<Move> moves;
  void doPass(int passIndex, int startIndex, int endIndex);

  __attribute__((noreturn))
    static void fiberEntryLeader(uint64_t argOnInitialize, uint64_t argOnRun);
  __attribute__((noreturn))
    static void fiberEntryFollower(uint64_t argOnInitialize, uint64_t argOnRun);
  __attribute__((noreturn))
    static void fiberEntryTail(uint64_t argOnInitialize, uint64_t argOnRun);

  Fiber leader __attribute__((aligned(SCE_FIBER_ALIGNMENT)));
  Fiber followers[6] __attribute__((aligned(SCE_FIBER_ALIGNMENT)));
  SceFiber tailFiber __attribute__((aligned(SCE_FIBER_ALIGNMENT)));
  char	  m_contextBuffer[8][CONTEXT_SIZE] __attribute__((aligned(SCE_FIBER_CONTEXT_ALIGNMENT)));
};