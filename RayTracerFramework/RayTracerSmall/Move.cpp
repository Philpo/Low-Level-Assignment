#include "Move.h"

Move::Move(xml_node<>* moveNode) {
  amount = convertStringToNumber<float>(moveNode->first_attribute("amount")->value());
  targetSphere = convertStringToNumber<int>(moveNode->first_attribute("target")->value());
  target = spheres[convertStringToNumber<int>(moveNode->first_attribute("target")->value())];
  std::string functionName = moveNode->first_attribute("function")->value();

  if (functionName == "moveX") {
    function = &moveX;
  }
  else if (functionName == "moveY") {
    function = &moveY;
  }
  else if (functionName == "moveZ") {
    function = &moveZ;
  }
  else if (functionName == "rotateX") {
    function = &rotateX;
  }
  else if (functionName == "rotateY") {
    function = &rotateY;
  }
  else if (functionName == "rotateZ") {
    function = &rotateZ;
  }
  else if (functionName == "scale") {
    function = &scale;
  }
}

void Move::doMove(int threadStart, Sphere& target) {
  int i = 0;
  while (i < threadStart) {
    doMove(target);
    i++;
  }
}