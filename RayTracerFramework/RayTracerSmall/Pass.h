#pragma once
#include "Move.h"
#include <thread>

class Pass {
public:
  Pass(xml_node<>* passNode, std::vector<Sphere>& spheres, std::string& directory, int passIndex);
  ~Pass() {}

  void render();
private:
  int numFrames, threadCount, passIndex;
  std::string directory;
  std::vector<Move> moves;
  std::vector<Sphere> spheres;
};