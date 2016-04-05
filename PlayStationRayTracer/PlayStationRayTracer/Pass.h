#pragma once
#include "Move.h"
#include <thread>

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
};