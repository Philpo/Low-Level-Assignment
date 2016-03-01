#pragma once
#include "Move.h"
#include <thread>

class Pass {
public:
	Pass(xml_node<>* passNode, std::string& directory, int passIndex, std::string& threadMethod, int numThreads);
  ~Pass() {}

  void render();
private:
  int numFrames, threadCount, passIndex;
  std::string directory, threadMethod;
  std::vector<Move> moves;
  void doPass(int passIndex, int startIndex, int endIndex);
};