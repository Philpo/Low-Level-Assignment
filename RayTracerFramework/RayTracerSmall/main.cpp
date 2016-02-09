// [header]
// A very basic raytracer example.
// [/header]
// [compile]
// c++ -o raytracer -O3 -Wall raytracer.cpp
// [/compile]
// [ignore]
// Copyright (C) 2012  www.scratchapixel.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// [/ignore]
#include <stdlib.h>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <vector>
#include <iostream>
#include <cassert>
// Windows only
#include <algorithm>
#include <sstream>
#include <string.h>
#include <cmath>
#include <thread>
#include "RapidXML\rapidxml.hpp"
#include "RapidXML\rapidxml_utils.hpp"
#include <iomanip>
#include <ctime>
#include <chrono>
#include "StringUtils.h"

using namespace rapidxml;

#if defined __linux__ || defined __APPLE__
// "Compiled for Linux
#else
// Windows doesn't define these values by default, Linux does
  #define M_PI 3.141592653589793
  #define INFINITY 1e8
#endif

std::chrono::time_point<std::chrono::system_clock> start;
std::chrono::time_point<std::chrono::system_clock> endTime;
std::chrono::duration<double> total_elapsed_time;
ofstream speedResults("O1_results.txt");

template<typename T>
class Vec3 {
public:
  T x, y, z;
  Vec3() : x(T(0)), y(T(0)), z(T(0)) {}
  Vec3(T xx) : x(xx), y(xx), z(xx) {}
  Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}
  Vec3& normalize() {
    T nor2 = length2();
    if (nor2 > 0) {
      T invNor = 1 / sqrt(nor2);
      x *= invNor, y *= invNor, z *= invNor;
    }
    return *this;
  }
  Vec3<T> operator * (const T &f) const { return Vec3<T>(x * f, y * f, z * f); }
  Vec3<T> operator * (const Vec3<T> &v) const { return Vec3<T>(x * v.x, y * v.y, z * v.z); }
  T dot(const Vec3<T> &v) const { return x * v.x + y * v.y + z * v.z; }
  Vec3<T> operator - (const Vec3<T> &v) const { return Vec3<T>(x - v.x, y - v.y, z - v.z); }
  Vec3<T> operator + (const Vec3<T> &v) const { return Vec3<T>(x + v.x, y + v.y, z + v.z); }
  Vec3<T>& operator += (const Vec3<T> &v) { x += v.x, y += v.y, z += v.z; return *this; }
  Vec3<T>& operator *= (const Vec3<T> &v) { x *= v.x, y *= v.y, z *= v.z; return *this; }
  Vec3<T> operator - () const { return Vec3<T>(-x, -y, -z); }
  T length2() const { return x * x + y * y + z * z; }
  T length() const { return sqrt(length2()); }
  friend std::ostream & operator << (std::ostream &os, const Vec3<T> &v) {
    os << "[" << v.x << " " << v.y << " " << v.z << "]";
    return os;
  }
};

typedef Vec3<float> Vec3f;

class Sphere {
public:
  Vec3f center;                           /// position of the sphere
  float radius, radius2;                  /// sphere radius and radius^2
  Vec3f surfaceColor, emissionColor;      /// surface color and emission (light)
  float transparency, reflection;         /// surface transparency and reflectivity
  Sphere(
    const Vec3f &c,
    const float &r,
    const Vec3f &sc,
    const float &refl = 0,
    const float &transp = 0,
    const Vec3f &ec = 0) :
    center(c), radius(r), radius2(r * r), surfaceColor(sc), emissionColor(ec),
    transparency(transp), reflection(refl) { /* empty */
  }
  //[comment]
  // Compute a ray-sphere intersection using the geometric solution
  //[/comment]
  bool intersect(const Vec3f &rayorig, const Vec3f &raydir, float &t0, float &t1) const {
    Vec3f l = center - rayorig;
    float tca = l.dot(raydir);
    if (tca < 0) return false;
    float d2 = l.dot(l) - tca * tca;
    if (d2 > radius2) return false;
    float thc = sqrt(radius2 - d2);
    t0 = tca - thc;
    t1 = tca + thc;

    return true;
  }
};

//[comment]
// This variable controls the maximum recursion depth
//[/comment]
#define MAX_RAY_DEPTH 5

float mix(const float &a, const float &b, const float &mix) {
  return b * mix + a * (1 - mix);
}

//[comment]
// This is the main trace function. It takes a ray as argument (defined by its origin
// and direction). We test if this ray intersects any of the geometry in the scene.
// If the ray intersects an object, we compute the intersection point, the normal
// at the intersection point, and shade this point using this information.
// Shading depends on the surface property (is it transparent, reflective, diffuse).
// The function returns a color for the ray. If the ray intersects an object that
// is the color of the object at the intersection point, otherwise it returns
// the background color.
//[/comment]
Vec3f trace(
  const Vec3f &rayorig,
  const Vec3f &raydir,
  const std::vector<Sphere> &spheres,
  const int &depth) {
  //if (raydir.length() != 1) std::cerr << "Error " << raydir << std::endl;
  float tnear = INFINITY;
  const Sphere* sphere = NULL;
  // find intersection of this ray with the sphere in the scene
  for (unsigned i = 0; i < spheres.size(); ++i) {
    float t0 = INFINITY, t1 = INFINITY;
    if (spheres[i].intersect(rayorig, raydir, t0, t1)) {
      if (t0 < 0) t0 = t1;
      if (t0 < tnear) {
        tnear = t0;
        sphere = &spheres[i];
      }
    }
  }
  // if there's no intersection return black or background color
  if (!sphere) return Vec3f(2);
  Vec3f surfaceColor = 0; // color of the ray/surfaceof the object intersected by the ray
  Vec3f phit = rayorig + raydir * tnear; // point of intersection
  Vec3f nhit = phit - sphere->center; // normal at the intersection point
  nhit.normalize(); // normalize normal direction
  // If the normal and the view direction are not opposite to each other
  // reverse the normal direction. That also means we are inside the sphere so set
  // the inside bool to true. Finally reverse the sign of IdotN which we want
  // positive.
  float bias = 1e-4; // add some bias to the point from which we will be tracing
  bool inside = false;
  if (raydir.dot(nhit) > 0) nhit = -nhit, inside = true;
  if ((sphere->transparency > 0 || sphere->reflection > 0) && depth < MAX_RAY_DEPTH) {
    float facingratio = -raydir.dot(nhit);
    // change the mix value to tweak the effect
    float fresneleffect = mix(pow(1 - facingratio, 3), 1, 0.1);
    // compute reflection direction (not need to normalize because all vectors
    // are already normalized)
    Vec3f refldir = raydir - nhit * 2 * raydir.dot(nhit);
    refldir.normalize();
    Vec3f reflection = trace(phit + nhit * bias, refldir, spheres, depth + 1);
    Vec3f refraction = 0;
    // if the sphere is also transparent compute refraction ray (transmission)
    if (sphere->transparency) {
      float ior = 1.1, eta = (inside) ? ior : 1 / ior; // are we inside or outside the surface?
      float cosi = -nhit.dot(raydir);
      float k = 1 - eta * eta * (1 - cosi * cosi);
      Vec3f refrdir = raydir * eta + nhit * (eta *  cosi - sqrt(k));
      refrdir.normalize();
      refraction = trace(phit - nhit * bias, refrdir, spheres, depth + 1);
    }
    // the result is a mix of reflection and refraction (if the sphere is transparent)
    surfaceColor = (
      reflection * fresneleffect +
      refraction * (1 - fresneleffect) * sphere->transparency) * sphere->surfaceColor;
  }
  else {
    // it's a diffuse object, no need to raytrace any further
    for (unsigned i = 0; i < spheres.size(); ++i) {
      if (spheres[i].emissionColor.x > 0) {
        // this is a light
        Vec3f transmission = 1;
        Vec3f lightDirection = spheres[i].center - phit;
        lightDirection.normalize();
        for (unsigned j = 0; j < spheres.size(); ++j) {
          if (i != j) {
            float t0, t1;
            if (spheres[j].intersect(phit + nhit * bias, lightDirection, t0, t1)) {
              transmission = 0;
              break;
            }
          }
        }
        surfaceColor += sphere->surfaceColor * transmission *
          max(float(0), nhit.dot(lightDirection)) * spheres[i].emissionColor;
      }
    }
  }

  return surfaceColor + sphere->emissionColor;
}

//[comment]
// Main rendering function. We compute a camera ray for each pixel of the image
// trace it and return a color. If the ray hits a sphere, we return the color of the
// sphere at the intersection point, else we return the background color.
//[/comment]
void render(const std::vector<Sphere> &spheres, int iteration, std::string& directory) {
  start = std::chrono::system_clock::now();

  // quick and dirty
#ifdef _DEBUG
  unsigned width = 640, height = 480;
#else
  unsigned width = 1920, height = 1080;
#endif
  // Recommended Testing Resolution
  //unsigned width = 640, height = 480;

  // Recommended Production Resolution
  //unsigned width = 1920, height = 1080;
  Vec3f *image = new Vec3f[width * height], *pixel = image;
  float invWidth = 1 / float(width), invHeight = 1 / float(height);
  float fov = 30, aspectratio = width / float(height);
  float angle = tan(M_PI * 0.5 * fov / 180.);
  // Trace rays
  for (unsigned y = 0; y < height; ++y) {
    for (unsigned x = 0; x < width; ++x, ++pixel) {
      float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
      float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
      Vec3f raydir(xx, yy, -1);
      raydir.normalize();
      *pixel = trace(Vec3f(0), raydir, spheres, 0);
    }
  }
  // Save result to a PPM image (keep these flags if you compile under Windows)
  std::stringstream ss;
  if (iteration < 10) {
    ss << ".\\" + directory + "\\spheres00" << iteration << ".ppm";
  }
  else if (iteration < 100) {
    ss << ".\\" + directory + "\\spheres0" << iteration << ".ppm";
  }
  else {
    ss << ".\\" + directory + "\\spheres" << iteration << ".ppm";
  }
  std::string tempString = ss.str();
  char* filename = (char*) tempString.c_str();

  std::ofstream ofs(filename, std::ios::out | std::ios::binary);
  ofs << "P6\n" << width << " " << height << "\n255\n";
  for (unsigned i = 0; i < width * height; ++i) {
    ofs << (unsigned char) (min(float(1), image[i].x) * 255) <<
      (unsigned char) (min(float(1), image[i].y) * 255) <<
      (unsigned char) (min(float(1), image[i].z) * 255);
  }
  ofs.close();
  delete[] image;

  endTime = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_time = endTime - start;
  total_elapsed_time += elapsed_time;
  std::cout << "Finished image render in " << elapsed_time.count() << std::endl;
  speedResults << "Finished image render in " << elapsed_time.count() << std::endl;
}

void BasicRender(std::string& directory) {
  std::vector<Sphere> spheres;
  // Vector structure for Sphere (position, radius, surface color, reflectivity, transparency, emission color)

  spheres.push_back(Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
  spheres.push_back(Sphere(Vec3f(0.0, 0, -20), 4, Vec3f(1.00, 0.32, 0.36), 1, 0.5)); // The radius paramter is the value we will change
  spheres.push_back(Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
  spheres.push_back(Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));

  // This creates a file, titled 1.ppm in the current working directory
  render(spheres, 1, directory);

}

void SimpleShrinking(std::string& directory) {
  std::vector<Sphere> spheres;
  // Vector structure for Sphere (position, radius, surface color, reflectivity, transparency, emission color)

  for (int i = 0; i < 4; i++) {
    if (i == 0) {
      spheres.push_back(Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
      spheres.push_back(Sphere(Vec3f(0.0, 0, -20), 4, Vec3f(1.00, 0.32, 0.36), 1, 0.5)); // The radius paramter is the value we will change
      spheres.push_back(Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
      spheres.push_back(Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));

    }
    else if (i == 1) {
      spheres.push_back(Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
      spheres.push_back(Sphere(Vec3f(0.0, 0, -20), 3, Vec3f(1.00, 0.32, 0.36), 1, 0.5)); // Radius--
      spheres.push_back(Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
      spheres.push_back(Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));
    }
    else if (i == 2) {
      spheres.push_back(Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
      spheres.push_back(Sphere(Vec3f(0.0, 0, -20), 2, Vec3f(1.00, 0.32, 0.36), 1, 0.5)); // Radius--
      spheres.push_back(Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
      spheres.push_back(Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));
    }
    else if (i == 3) {
      spheres.push_back(Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
      spheres.push_back(Sphere(Vec3f(0.0, 0, -20), 1, Vec3f(1.00, 0.32, 0.36), 1, 0.5)); // Radius--
      spheres.push_back(Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
      spheres.push_back(Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));
    }

    render(spheres, i, directory);
    // Dont forget to clear the Vector holding the spheres.
    spheres.clear();
  }
}

void SmoothScaling(std::string& directory) {
  std::vector<Sphere> spheres;
  // Vector structure for Sphere (position, radius, surface color, reflectivity, transparency, emission color)

  for (float r = 0; r <= 100; r++) {
    spheres.push_back(Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
    spheres.push_back(Sphere(Vec3f(0.0, 0, -20), 1, Vec3f(1.00, 0.32, 0.36), 1, 0.5)); // Radius++ change here
    spheres.push_back(Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
    spheres.push_back(Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));
    render(spheres, r, directory);
    std::cout << "Rendered and saved spheres" << r << ".ppm" << std::endl;
    // Dont forget to clear the Vector holding the spheres.
    spheres.clear();
  }
}

void moveX(Sphere& toMove, float amount) {
  toMove.center.x += amount;
}

void moveY(Sphere& toMove, float amount) {
  toMove.center.y += amount;
}

void moveZ(Sphere& toMove, float amount) {
  toMove.center.z += amount;
}

void rotateX(Sphere& toRotate, float angle) {
  Vec3f topRow(1.0f, 0.0f, 0.0f);
  Vec3f middleRow(0.0f, cos(angle), sin(angle));
  Vec3f bottomRow(0.0f, -sin(angle), cos(angle));

  float newX = (topRow.x * toRotate.center.x) + (topRow.y * toRotate.center.y) + (topRow.z * toRotate.center.z);
  float newY = (middleRow.x * toRotate.center.x) + (middleRow.y * toRotate.center.y) + (middleRow.z * toRotate.center.z);
  float newZ = (bottomRow.x * toRotate.center.x) + (bottomRow.y * toRotate.center.y) + (bottomRow.z * toRotate.center.z);

  toRotate.center.x = newX;
  toRotate.center.y = newY;
  toRotate.center.z = newZ;
}

void rotateY(Sphere& toRotate, float angle) {
  Vec3f topRow(cos(angle), 0.0f, -sin(angle));
  Vec3f middleRow(0.0f, 1.0f, 0.0f);
  Vec3f bottomRow(sin(angle), 0.0f, cos(angle));

  float newX = (topRow.x * toRotate.center.x) + (topRow.y * toRotate.center.y) + (topRow.z * toRotate.center.z);
  float newY = (middleRow.x * toRotate.center.x) + (middleRow.y * toRotate.center.y) + (middleRow.z * toRotate.center.z);
  float newZ = (bottomRow.x * toRotate.center.x) + (bottomRow.y * toRotate.center.y) + (bottomRow.z * toRotate.center.z);

  toRotate.center.x = newX;
  toRotate.center.y = newY;
  toRotate.center.z = newZ;
}

void rotateZ(Sphere& toRotate, float angle) {
  Vec3f topRow(cos(angle), sin(angle), 0.0f);
  Vec3f middleRow(-sin(angle), cos(angle), 0.0f);
  Vec3f bottomRow(0.0f, 0.0f, 1.0f);

  float newX = (topRow.x * toRotate.center.x) + (topRow.y * toRotate.center.y) + (topRow.z * toRotate.center.z);
  float newY = (middleRow.x * toRotate.center.x) + (middleRow.y * toRotate.center.y) + (middleRow.z * toRotate.center.z);
  float newZ = (bottomRow.x * toRotate.center.x) + (bottomRow.y * toRotate.center.y) + (bottomRow.z * toRotate.center.z);

  toRotate.center.x = newX;
  toRotate.center.y = newY;
  toRotate.center.z = newZ;
}

//void moveSpheres() {
//  std::vector<Sphere> spheres;
//  // Vector structure for Sphere (position, radius, surface color, reflectivity, transparency, emission color)
//  spheres.push_back(Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
//  spheres.push_back(Sphere(Vec3f(0.0, 0, -20), 1, Vec3f(1.00, 0.32, 0.36), 1, 0.5)); // Radius++ change here
//  spheres.push_back(Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
//  spheres.push_back(Sphere(Vec3f(5.0, 0, -25), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));
//
//  for (float r = 0; r <= 360; r++) {
//    moveX(spheres[1], r / 100.0f);
//    spheres[2].center.y = sin(r * M_PI / 180.0f);
//    //moveY(spheres[2], r / 100.0f);
//    moveZ(spheres[3], r / 100.0f);
//    render(spheres, r);
//    std::cout << "Rendered and saved spheres" << r << ".ppm" << std::endl;
//  }
//  // Dont forget to clear the Vector holding the spheres.
//  spheres.clear();
//}

int calcOffset(int startIndex) {
  int returnVal = 0;

  for (int i = 0; i < startIndex; i++) {
    returnVal += i;
  }
  
  return returnVal;
}

void moveSpheres(int startIndex, int endIndex, std::string& directory) {
  std::vector<Sphere> spheres;
  // Vector structure for Sphere (position, radius, surface color, reflectivity, transparency, emission color)
  spheres.push_back(Sphere(Vec3f(0.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
  spheres.push_back(Sphere(Vec3f(calcOffset(startIndex) / 100.0f, 0, -20), 1, Vec3f(1.00, 0.32, 0.36), 1, 0.5)); // Radius++ change here
  spheres.push_back(Sphere(Vec3f(5.0, -1, -15), 2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
  spheres.push_back(Sphere(Vec3f(5.0, 0, -25 + (calcOffset(startIndex) / 100.0f)), 3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));

  for (float r = startIndex; r <= endIndex; r++) {
    moveX(spheres[1], r / 100.0f);
    spheres[2].center.y = sin(r * M_PI / 180.0f);
    moveZ(spheres[3], r / 100.0f);
    render(spheres, r, directory);
    std::cout << "Rendered and saved spheres" << r << ".ppm" << std::endl;
  }
  // Dont forget to clear the Vector holding the spheres.
  spheres.clear();
}

void rotateSpheres(std::vector<Sphere>& spheres, int startIndex, int endIndex, std::string& directory) {
  rotateX(spheres[1], startIndex * M_PI / 180.0f);
  rotateY(spheres[2], startIndex * M_PI / 180.0f);
  rotateZ(spheres[3], startIndex * M_PI / 180.0f);

  for (float r = startIndex; r <= endIndex; r++) {
    rotateX(spheres[1], 1.0f * M_PI / 180.0f);
    rotateY(spheres[2], 1.0f * M_PI / 180.0f);
    rotateZ(spheres[3], 1.0f * M_PI / 180.0f);
    render(spheres, r, directory);
    std::cout << "Rendered and saved spheres" << r << ".ppm" << std::endl;
  }
  // Dont forget to clear the Vector holding the spheres.
  spheres.clear();
}

//[comment]
// In the main function, we will create the scene which is composed of 5 spheres
// and 1 light (which is also a sphere). Then, once the scene description is complete
// we render that scene, by calling the render() function.
//[/comment]
int main(int argc, char **argv) {
  // This sample only allows one choice per program execution. Feel free to improve upon this
  srand(13);


  std::string directory;
  auto now = std::time(nullptr);
  std::ostringstream os;
  os << std::put_time(std::gmtime(&now), "%Y-%m-%d_%H%M%S");
  directory = "spheres" + os.str();

  std::string mkdirCommand = "mkdir .\\" + directory;
  system(mkdirCommand.c_str());

  //BasicRender();
  //SimpleShrinking();
  //SmoothScaling();

  std::vector<Sphere> spheres;
  int totalFrames, numThreads;

  try {
    file<> sceneFile("scene.xml");
    xml_document<> doc;
    doc.parse<0>(sceneFile.data());
    xml_node<>* rootNode = doc.first_node();

    float x, y, z, radius, r, g, b, reflection, transparency, emissionR, emissionG, emissionB;
    reflection = transparency = emissionR = emissionG = emissionB = 0.0f;

    for (xml_node<>* sphereNode = rootNode->first_node(); sphereNode; sphereNode = sphereNode->next_sibling()) {
      x = convertStringToNumber<float>(sphereNode->first_attribute("x")->value());
      y = convertStringToNumber<float>(sphereNode->first_attribute("y")->value());
      z = convertStringToNumber<float>(sphereNode->first_attribute("z")->value());
      radius = convertStringToNumber<float>(sphereNode->first_attribute("radius")->value());
      r = convertStringToNumber<float>(sphereNode->first_attribute("r")->value());
      g = convertStringToNumber<float>(sphereNode->first_attribute("g")->value());
      b = convertStringToNumber<float>(sphereNode->first_attribute("b")->value());

      if (sphereNode->first_attribute("reflection")) {
        reflection = convertStringToNumber<float>(sphereNode->first_attribute("reflection")->value());
      }
      if (sphereNode->first_attribute("transparency")) {
        transparency = convertStringToNumber<float>(sphereNode->first_attribute("transparency")->value());
      }
      if (sphereNode->first_attribute("emissionR")) {
        emissionR = convertStringToNumber<float>(sphereNode->first_attribute("emissionR")->value());
      }
      if (sphereNode->first_attribute("emissionG")) {
        emissionG = convertStringToNumber<float>(sphereNode->first_attribute("emissionG")->value());
      }
      if (sphereNode->first_attribute("emissionB")) {
        emissionB = convertStringToNumber<float>(sphereNode->first_attribute("emissionB")->value());
      }

      spheres.push_back(Sphere(Vec3f(x, y, z), radius, Vec3f(r, g, b), reflection, transparency, Vec3f(emissionR, emissionG, emissionB)));
    }
  }
  catch (parse_error& e) {
    ofstream errorFile;
    errorFile.open("error_file.txt");
    errorFile << "Error reading scene file " << ": " << e.what() << endl;
    errorFile.close();
    return 0;
  }

  try {
    file<> passFile("test.xml");
    xml_document<> doc;
    doc.parse<0>(passFile.data());
    xml_node<>* rootNode = doc.first_node();

    totalFrames = convertStringToNumber<int>(rootNode->first_attribute("total_frames")->value());
    numThreads = convertStringToNumber<int>(rootNode->first_attribute("threads")->value());

    for (xml_node<>* moveNode = rootNode->first_node(); moveNode; moveNode = moveNode->next_sibling()) {
      
    }
  }
  catch (parse_error& e) {
    ofstream errorFile;
    errorFile.open("error_file.txt");
    errorFile << "Error reading pass file " << ": " << e.what() << endl;
    errorFile.close();
    return 0;
  }

  std::vector<std::thread> threads;

  int threadWorkload = 0;
  int remainder = 0;

  threadWorkload = totalFrames / numThreads;

  if (totalFrames % numThreads != 0) {
    remainder = totalFrames - (threadWorkload * (numThreads - 1));
  }

  for (int i = 0; i < numThreads; i++) {
    if (i == numThreads - 1 && remainder > 0) {
      threads.push_back(std::thread(rotateSpheres, spheres, i * threadWorkload, (i * threadWorkload) + remainder, directory));
    }
    else {
      threads.push_back(std::thread(rotateSpheres, spheres, i * threadWorkload, (i * threadWorkload) + threadWorkload, directory));
    }
  }

  for (int i = 0; i < numThreads; i++) {
    threads[i].join();
  }

  start = std::chrono::system_clock::now();
  std::string ffmpegCommand = "ffmpeg -i .\\" + directory + "\\spheres%03d.ppm -y .\\" + directory + "\\out.mp4";
  system(ffmpegCommand.c_str());
  endTime = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_time = endTime - start;
  total_elapsed_time += elapsed_time;

  std::cout << "**********************" << std::endl;
  std::cout << "Finished video render in " << elapsed_time.count() << std::endl;
  std::cout << "**********************" << std::endl;
  std::cout << "**********************" << std::endl;
  std::cout << "Total Render Time: " << total_elapsed_time.count() << std::endl;
  std::cout << "**********************" << std::endl;

  speedResults << "**********************" << std::endl;
  speedResults << "Finished video render in " << elapsed_time.count() << std::endl;
  speedResults << "**********************" << std::endl;
  speedResults << "**********************" << std::endl;
  speedResults << "Total Render Time: " << total_elapsed_time.count() << std::endl;
  speedResults << "**********************" << std::endl;

  speedResults.close();

  return 0;
}