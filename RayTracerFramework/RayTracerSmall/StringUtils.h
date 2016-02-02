#pragma once
#include <string>
#include <sstream>

using namespace std;

inline bool convertStringToBool(const string& toConvert) {
  if (toConvert == "true") {
    return true;
  }
  else {
    return false;
  }
}

template <class T>
inline T convertStringToNumber(const string& toConvert) {
  T r;
  stringstream(toConvert) >> r;
  return r;
}