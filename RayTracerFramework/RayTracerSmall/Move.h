#pragma once
#include "RayTracer.h"
#include "RapidXML\rapidxml.hpp"
#include "StringUtils.h"
#include <vector>

using namespace rapidxml;

typedef void(*moveFunction)(Sphere&, float);

class Move {
public:
  Move(xml_node<>* moveNode);
  ~Move() {}

  int getTargetSphere() { return targetSphere; }

  inline void doMove(Sphere& target) { function(target, amount); }
  void doMove(int threadStart, Sphere& target);
private:
  int targetSphere;
  moveFunction function;
  Sphere target;
  float amount;
};