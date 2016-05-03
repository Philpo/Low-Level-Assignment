#include "Pass.h"

Pass::Pass(xml_node<>* passNode, std::string& directory, int passIndex, std::string& threadMethod, int numThreads, std::string& ioMethod) : 
	directory(directory), passIndex(passIndex), threadMethod(threadMethod), ioMethod(ioMethod) {
  numFrames = convertStringToNumber<int>(passNode->first_attribute("frames")->value());
  if (threadMethod == "tp" || threadMethod == "p") {
    threadCount = numThreads;
  }
  else {
    threadCount = convertStringToNumber<int>(passNode->first_attribute("threads")->value());
  }

  for (xml_node<>* moveNode = passNode->first_node(); moveNode; moveNode = moveNode->next_sibling()) {
    moves.push_back(Move(moveNode));
  }

  int ret = sceSysmoduleLoadModule(SCE_SYSMODULE_FIBER);
  assert(ret == SCE_OK);
}

void Pass::render() {
  //doPass(0, 0, numFrames);
  if (threadMethod == "tm1") {
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    std::vector<std::thread> threads;

    if (ioMethod == "tio") {
      images.clear();
      physicalAddresses.clear();
      images.resize(numFrames);
      physicalAddresses.resize(numFrames);
    }

    int threadWorkload = 0;
    int remainder = 0;

    threadWorkload = numFrames / threadCount;

    if (numFrames % threadCount != 0) {
      remainder = numFrames - (threadWorkload * (threadCount - 1));
    }

    for (int i = 0; i < threadCount; i++) {
      if (i == threadCount - 1 && remainder > 0) {
        threads.push_back(std::thread(&Pass::threadedDoPass, this, i, (passIndex * numFrames), i * threadWorkload, (i * threadWorkload) + remainder));
      }
      else {
        threads.push_back(std::thread(&Pass::threadedDoPass, this, i, (passIndex * numFrames), i * threadWorkload, (i * threadWorkload) + threadWorkload));
      }
    }

    for (int i = 0; i < threadCount; i++) {
      threads[i].join();
    }

    if (ioMethod == "tio") {
      threads.clear();

      for (int i = 0; i < threadCount; i++) {
        if (i == threadCount - 1 && remainder > 0) {
          threads.push_back(std::thread(&threadedFileSave, (passIndex * numFrames) + (i * threadWorkload), std::ref(directory), i * threadWorkload, (i * threadWorkload) + remainder));
        }
        else {
          threads.push_back(std::thread(&threadedFileSave, (passIndex * numFrames) + (i * threadWorkload), std::ref(directory), i * threadWorkload, (i * threadWorkload) + threadWorkload));
        }
      }

      for (int i = 0; i < threadCount; i++) {
        threads[i].join();
      }
    }

    std::chrono::time_point<std::chrono::system_clock> endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_time = endTime - start;
    total_elapsed_time += elapsed_time;
  }
  else if (threadMethod == "fi") {
    if (ioMethod == "tio") {
      images.clear();
      physicalAddresses.clear();
      images.resize(numFrames);
      physicalAddresses.resize(numFrames);
    }

    fiberDoPass(passIndex * numFrames, 0, numFrames);

    if (ioMethod == "tio") {
      std::chrono::time_point<std::chrono::system_clock> savingStart = std::chrono::system_clock::now();
      std::vector<std::thread> threads;

      int threadWorkload = 0;
      int remainder = 0;

      threadWorkload = numFrames / threadCount;

      if (numFrames % threadCount != 0) {
        remainder = numFrames - (threadWorkload * (threadCount - 1));
      }

      for (int i = 0; i < threadCount; i++) {
        if (i == threadCount - 1 && remainder > 0) {
          threads.push_back(std::thread(&threadedFileSave, (passIndex * numFrames) + (i * threadWorkload), std::ref(directory), i * threadWorkload, (i * threadWorkload) + remainder));
        }
        else {
          threads.push_back(std::thread(&threadedFileSave, (passIndex * numFrames) + (i * threadWorkload), std::ref(directory), i * threadWorkload, (i * threadWorkload) + threadWorkload));
        }
      }

      for (int i = 0; i < threadCount; i++) {
        threads[i].join();
      }
      std::chrono::time_point<std::chrono::system_clock> endTime = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed_time = endTime - savingStart;
      total_elapsed_time += elapsed_time;
    }
  }
  else {
    if (ioMethod == "tio") {
      images.clear();
      physicalAddresses.clear();
      images.resize(numFrames);
      physicalAddresses.resize(numFrames);
    }

    doPass(passIndex * numFrames, 0, numFrames);

    if (ioMethod == "tio") {
      std::chrono::time_point<std::chrono::system_clock> savingStart = std::chrono::system_clock::now();
      std::vector<std::thread> threads;

      int threadWorkload = 0;
      int remainder = 0;

      threadWorkload = numFrames / threadCount;

      if (numFrames % threadCount != 0) {
        remainder = numFrames - (threadWorkload * (threadCount - 1));
      }

      for (int i = 0; i < threadCount; i++) {
        if (i == threadCount - 1 && remainder > 0) {
          threads.push_back(std::thread(&threadedFileSave, (passIndex * numFrames) + (i * threadWorkload), std::ref(directory), i * threadWorkload, (i * threadWorkload) + remainder));
        }
        else {
          threads.push_back(std::thread(&threadedFileSave, (passIndex * numFrames) + (i * threadWorkload), std::ref(directory), i * threadWorkload, (i * threadWorkload) + threadWorkload));
        }
      }

      for (int i = 0; i < threadCount; i++) {
        threads[i].join();
      }
      std::chrono::time_point<std::chrono::system_clock> endTime = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed_time = endTime - savingStart;
      total_elapsed_time += elapsed_time;
    }
  }
}

void Pass::threadedDoPass(int threadIndex, int passOffset, int startIndex, int endIndex) {
  std::vector<Sphere> copy;

  for (Sphere sphere : spheres) {
    copy.push_back(sphere);
  }

  for (auto move : moves) {
    move.doMove(startIndex, copy[move.getTargetSphere()]);
  }

  int i = startIndex;
  while (i < endIndex) {
    int iteration = ioMethod == "tio" ? i : passOffset + i;
    for (auto move : moves) {
      move.doMove(copy[move.getTargetSphere()]);
    }

    threadMethod1(copy, iteration, directory, ioMethod == "tio");
    i++;
  }

  if (threadIndex == threadCount - 1) {
    for (int i = 0; i < copy.size(); i++) {
      spheres[i] = copy[i];
    }
  }
}

void Pass::fiberDoPass(int passOffset, int startIndex, int endIndex) {
  Fiber leader __attribute__((aligned(SCE_FIBER_ALIGNMENT)));
  Fiber* followers __attribute__((aligned(SCE_FIBER_ALIGNMENT)));
  SceFiber tailFiber __attribute__((aligned(SCE_FIBER_ALIGNMENT)));
  char m_contextBuffer[CONTEXT_SIZE] __attribute__((aligned(SCE_FIBER_CONTEXT_ALIGNMENT)));
  off_t physicalAddress;

  int ret = SCE_OK;

  ret = sceFiberInitialize(&leader.fiber, "leaderFiber", fiberEntryLeader, (uint64_t) &leader, (void*) m_contextBuffer, CONTEXT_SIZE, NULL);
  assert(ret == SCE_OK);

  leader.nextFiber = &followers[0].fiber;
  leader.directory = &directory;
  leader.spheres = spheres;
  leader.deferSaving = ioMethod == "tio";
  leader.iteration = ioMethod == "tio" ? startIndex : passOffset + startIndex;

  ret = sceFiberInitialize(&tailFiber, "tailFiber", fiberEntryTail, (uint64_t) &tailFiber, (void*) m_contextBuffer, CONTEXT_SIZE, NULL);
  assert(ret == SCE_OK);

  int framesPerThread = endIndex - startIndex;
  size_t totalSize = sizeof(Fiber) * framesPerThread;
  followers = (Fiber*) operator new[](totalSize, physicalAddress);

  int i = startIndex;
  int fiberIndex = 0;
  while (i < endIndex) {
    int iteration = ioMethod == "tio" ? i : passOffset + i;
    for (auto move : moves) {
      move.doMove(spheres[move.getTargetSphere()]);
    }

    ret = sceFiberInitialize(&followers[fiberIndex].fiber, "followerFiber", fiberEntryFollower, (uint64_t) &followers[fiberIndex], (void*) m_contextBuffer, CONTEXT_SIZE, NULL);
    assert(ret == SCE_OK);

    if (fiberIndex != framesPerThread - 1) {
      followers[fiberIndex].nextFiber = &followers[fiberIndex + 1].fiber;
    }
    else {
      followers[fiberIndex].nextFiber = &tailFiber;
    }

    followers[fiberIndex].directory = &directory;
    followers[fiberIndex].spheres = spheres;
    followers[fiberIndex].deferSaving = ioMethod == "tio";
    followers[fiberIndex].iteration = iteration;

    fiberIndex++;
    i++;
  }

  uint64_t argOnReturn = 0;

  leader.nextFiber = &followers[0].fiber;
  ret = sceFiberRun(&leader.fiber, 0, &argOnReturn);
  assert(ret == SCE_OK);
  assert(argOnReturn == 0);

  ret = sceFiberFinalize(&tailFiber);
  assert(ret == SCE_OK);

  for (int i = 0; i < framesPerThread; i++) {
    ret = sceFiberFinalize(&followers[i].fiber);
    assert(ret == SCE_OK);
  }

  ret = sceFiberFinalize(&leader.fiber);
  assert(ret == SCE_OK);

  operator delete[](followers, totalSize, physicalAddress);
}

void Pass::doPass(int passOffset, int startIndex, int endIndex) {
  int i = startIndex;
  while (i < endIndex) {
    int iteration = ioMethod == "tio" ? i : passOffset + i;
    for (auto move : moves) {
      move.doMove(spheres[move.getTargetSphere()]);
    }
    if (threadMethod == "tm3") {
      threadMethod3(iteration, directory, threadCount, ioMethod == "tio");
    }
    else {
      threadMethod2(iteration, directory, threadCount, ioMethod == "tio");
    }
    i++;
  }
}

void Pass::fiberEntryLeader(uint64_t argOnInitialize, uint64_t argOnRun) {
  cout << "leader" << endl;

  Fiber* me = (Fiber*) argOnInitialize;

  int32_t ret = sceFiberSwitch(me->nextFiber, (uint64_t) me, &argOnRun);
  assert(ret == SCE_OK);
}

void Pass::fiberEntryFollower(uint64_t argOnInitialize, uint64_t argOnRun) {
  cout << "follower" << endl;

  Fiber* me = (Fiber*) argOnInitialize;

  threadMethod1(me->spheres, me->iteration, *me->directory, me->deferSaving);
  int32_t ret = sceFiberSwitch(me->nextFiber, (uint64_t) me, &argOnRun);
  assert(ret == SCE_OK);
}

void Pass::fiberEntryTail(uint64_t argOnInitialize, uint64_t argOnRun) {
  cout << "finished" << endl;
  int ret = sceFiberReturnToThread(0, NULL);
  assert(ret == SCE_OK);
}