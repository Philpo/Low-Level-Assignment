#include "Pass.h"

Pass::Pass(xml_node<>* passNode, std::string& directory, int passIndex, std::string& threadMethod, int numThreads) : 
	directory(directory), passIndex(passIndex), threadMethod(threadMethod) {
  numFrames = convertStringToNumber<int>(passNode->first_attribute("frames")->value());
  if (threadMethod == "tp" || threadMethod == "p") {
    threadCount = numThreads;
  }
  else {
    threadCount = convertStringToNumber<int>(passNode->first_attribute("threads")->value());
  }

  for (xml_node<>* moveNode = passNode->first_node(); moveNode; moveNode = moveNode->next_sibling()) {
    moves.push_back(Move(moveNode, spheres));
  }
}

void Pass::render() {
  if (threadMethod == "tf") {
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    std::vector<std::thread> threads;
    images.clear();
    images.resize(numFrames);

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
    endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_time = endTime - start;
    total_elapsed_time += elapsed_time;
  }
  else {
    images.clear();
    images.resize(numFrames);

    doPass(passIndex * numFrames, 0, numFrames);

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
    endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_time = endTime - start;
    total_elapsed_time += elapsed_time;
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
      for (auto move : moves) {
        move.doMove(copy[move.getTargetSphere()]);
      }
      renderFrame(copy, i, directory);
      i++;
    }

    for (int i = 0; i < copy.size(); i++) {
      spheres[i] = copy[i];
    }
  }
  else {
    int i = startIndex;
    while (i < endIndex) {
      for (auto move : moves) {
        move.doMove(spheres[move.getTargetSphere()]);
      }
      if (threadMethod == "p") {
        partitionAndRender(spheres, i, directory, threadCount);
      }
      else {
        threadPartitionRender(spheres, i, directory, threadCount);
      }
      i++;
    }
  }
}