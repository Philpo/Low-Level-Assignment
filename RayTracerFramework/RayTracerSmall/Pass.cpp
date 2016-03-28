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
  if (threadMethod == "tf") {
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    std::vector<std::thread> threads;

    if (ioMethod == "tio") {
      images.clear();
      images.resize(numFrames);
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
      images.resize(numFrames);
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
      renderFrame(copy, iteration, directory, ioMethod == "tio");
      i++;
    }

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