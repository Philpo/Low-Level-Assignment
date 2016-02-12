#pragma once
#include "Move.h"
#include <thread>

class Pass {
public:
  Pass(xml_node<>* passNode, std::vector<Sphere>& spheres, std::string& directory);
  ~Pass() {}

  void render(std::chrono::time_point<std::chrono::system_clock>& start, std::chrono::time_point<std::chrono::system_clock>& endTime, std::chrono::duration<double>& total_elapsed_time, std::ofstream& speedResults);
private:
  int numFrames, threadCount;
  std::string directory;
  std::vector<Move> moves;
  std::vector<Sphere> spheres;
};