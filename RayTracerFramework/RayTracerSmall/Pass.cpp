#include "Pass.h"

Pass::Pass(xml_node<>* passNode, std::vector<Sphere>& spheres, std::string& directory) : spheres(spheres), directory(directory) {
  numFrames = convertStringToNumber<int>(passNode->first_attribute("frames")->value());
  threadCount = convertStringToNumber<int>(passNode->first_attribute("threads")->value());

  for (xml_node<>* moveNode = passNode->first_node(); moveNode; moveNode = moveNode->next_sibling()) {
    moves.push_back(Move(moveNode, spheres));
  }
}

void Pass::render() {
  std::vector<std::thread> threads;

  int threadWorkload = 0;
  int remainder = 0;

  threadWorkload = numFrames / threadCount;

  if (numFrames % threadCount != 0) {
    remainder = numFrames - (threadWorkload * (threadCount - 1));
  }

  for (int i = 0; i < threadCount; i++) {
    if (i == threadCount - 1 && remainder > 0) {
      threads.push_back(std::thread(doPass, spheres, i * threadWorkload, (i * threadWorkload) + remainder, moves, directory));
    }
    else {
      threads.push_back(std::thread(doPass, spheres, i * threadWorkload, (i * threadWorkload) + threadWorkload, moves, directory));
    }
  }

  for (int i = 0; i < threadCount; i++) {
    threads[i].join();
  }
}