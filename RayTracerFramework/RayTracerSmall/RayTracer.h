#pragma once
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
#include <iomanip>
#include <ctime>
#include <chrono>

class Move;
class Sphere;

#if defined __linux__ || defined __APPLE__
// "Compiled for Linux
#else
// Windows doesn't define these values by default, Linux does
#define M_PI 3.141592653589793
#define INFINITY 1e8
#endif

extern std::chrono::time_point<std::chrono::system_clock> start;
extern std::chrono::time_point<std::chrono::system_clock> endTime;
extern std::chrono::duration<double> total_elapsed_time;
extern std::ofstream speedResults;
extern std::vector<Sphere> spheres;

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
extern Vec3f* image;

class Sphere {
public:
  Vec3f center;                           /// position of the sphere
  float radius, radius2;                  /// sphere radius and radius^2
  Vec3f surfaceColor, emissionColor;      /// surface color and emission (light)
  float transparency, reflection;         /// surface transparency and reflectivity
  Sphere() {}
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

float mix(const float &a, const float &b, const float &mix);

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
Vec3f trace(const Vec3f &rayorig, const Vec3f &raydir, const std::vector<Sphere> &spheres, const int &depth);

//[comment]
// Main rendering function. We compute a camera ray for each pixel of the image
// trace it and return a color. If the ray hits a sphere, we return the color of the
// sphere at the intersection point, else we return the background color.
//[/comment]
void render(const std::vector<Sphere> &spheres, int iteration, std::string& directory);

void moveX(Sphere& toMove, float amount);

void moveY(Sphere& toMove, float amount);

void moveZ(Sphere& toMove, float amount);

void rotateX(Sphere& toRotate, float angle);

void rotateY(Sphere& toRotate, float angle);

void rotateZ(Sphere& toRotate, float angle);

void scale(Sphere& toScale, float amount);

void doPass(int passIndex, int startIndex, int endIndex, std::vector<Move>& moves, std::string& directory);