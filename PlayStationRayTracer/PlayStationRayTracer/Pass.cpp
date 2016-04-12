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
}

void Pass::render() {
  //doPass(0, 0, numFrames);
  if (threadMethod == "tf") {
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
        threads.push_back(std::thread(&Pass::doPass, this, (passIndex * numFrames), i * threadWorkload, (i * threadWorkload) + remainder));
      }
      else {
        threads.push_back(std::thread(&Pass::doPass, this, (passIndex * numFrames), i * threadWorkload, (i * threadWorkload) + threadWorkload));
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

void Pass::doPass(int passOffset, int startIndex, int endIndex) {
  if (threadMethod == "tf") {
    std::vector<Sphere> copy;

    cout << CONTEXT_SIZE << endl;

    for (Sphere sphere : spheres) {
      copy.push_back(sphere);
    }

    for (auto move : moves) {
      move.doMove(startIndex, copy[move.getTargetSphere()]);
    }

    int ret = SCE_OK;

    ret = sceSysmoduleLoadModule(SCE_SYSMODULE_FIBER);
    assert(ret == SCE_OK);

    ret = sceFiberStartContextSizeCheck(0);
    assert(ret == SCE_OK);

    ret = sceFiberInitialize(&leader.fiber, "leaderFiber", fiberEntryLeader, (uint64_t) &leader, (void*) m_contextBuffer[0], CONTEXT_SIZE, NULL);
    assert(ret == SCE_OK);

    leader.nextFiber = &followers[0].fiber;
    leader.directory = directory;
    leader.spheres = copy;
    leader.deferSaving = ioMethod == "tio";
    leader.iteration = ioMethod == "tio" ? startIndex : passOffset + startIndex;

    ret = sceFiberInitialize(&tailFiber, "tailFiber", fiberEntryTail, (uint64_t) &tailFiber, (void*) m_contextBuffer[7], CONTEXT_SIZE, NULL);
    assert(ret == SCE_OK);

    int i = startIndex;
    int fiberIndex = 0;
    while (i < endIndex) {
      int iteration = ioMethod == "tio" ? i : passOffset + i;
      for (auto move : moves) {
        move.doMove(copy[move.getTargetSphere()]);
      }

      ret = sceFiberInitialize(&followers[fiberIndex].fiber, "followerFiber", fiberEntryFollower, (uint64_t) &followers[fiberIndex], (void*) m_contextBuffer[fiberIndex + 1], CONTEXT_SIZE, NULL);
      assert(ret == SCE_OK);

      if (fiberIndex != 5){
        followers[fiberIndex].nextFiber = &followers[fiberIndex + 1].fiber;
      }
      else{
        followers[fiberIndex].nextFiber = &tailFiber;
      }

      followers[fiberIndex].directory = directory;
      followers[fiberIndex].spheres = copy;
      followers[fiberIndex].deferSaving = ioMethod == "tio";
      followers[fiberIndex].iteration = iteration;

      fiberIndex++;

//      renderFrame(copy, iteration, directory, ioMethod == "tio");
      i++;
    }

    uint64_t argOnReturn = 0;

    ret = sceFiberRun(&leader.fiber, 0, &argOnReturn);
    assert(ret == SCE_OK);
    assert(argOnReturn == 0);

    for (int i = 0; i < copy.size(); i++) {
      spheres[i] = copy[i];
    }
  }
  else {
    int i = startIndex;
    while (i < endIndex) {
      int iteration = ioMethod == "tio" ? i : passOffset + i;
      for (auto move : moves) {
        move.doMove(spheres[move.getTargetSphere()]);
      }
      if (threadMethod == "p") {
        partitionAndRender(iteration, directory, threadCount, ioMethod == "tio");
      }
      else {
        threadPartitionRender(iteration, directory, threadCount, ioMethod == "tio");
      }
      i++;
    }
  }
}

void Pass::fiberEntryLeader(uint64_t argOnInitialize, uint64_t argOnRun) {
  cout << "leader" << endl;

  Fiber* me = (Fiber*) argOnInitialize;

  //renderFrame(me->spheres, me->iteration, me->directory, me->deferSaving);
  int32_t ret = sceFiberSwitch(me->nextFiber, (uint64_t) me, &argOnRun);
  assert(ret == SCE_OK);
}

void Pass::fiberEntryFollower(uint64_t argOnInitialize, uint64_t argOnRun) {
  cout << "follower" << endl;

  Fiber* me = (Fiber*) argOnInitialize;

  renderFrame(me->spheres, me->iteration, me->directory, me->deferSaving);
  int32_t ret = sceFiberSwitch(me->nextFiber, (uint64_t) me, &argOnRun);
  assert(ret == SCE_OK);
}

void Pass::fiberEntryTail(uint64_t argOnInitialize, uint64_t argOnRun) {
  cout << "finished" << endl;
  int ret = sceFiberReturnToThread(0, NULL);
  assert(ret == SCE_OK);
}