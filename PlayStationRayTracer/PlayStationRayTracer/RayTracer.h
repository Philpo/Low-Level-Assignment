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
#include <thread>
#include "Vecf.h"
#include "Sphere.h"
#include "allocator.h"
#include <gnmx.h>
#include <scebase.h>
#include <kernel.h>
#include <video_out.h>

class Move;

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
extern Vec3f* image;
extern std::vector<Vec3f*> images;
extern LinearAllocator onionAllocator;
static const size_t ONION_MEMORY_SIZE = 64 * 1024 * 1024;

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
Vec3f trace(const Vec3f &rayorig, const Vec3f &raydir, const std::vector<Sphere> &spheres, const int &depth, bool deferSaving);

// renders an individual frame using one thread; the multi-threading aspect come from computing frames on multiple threads
void renderFrame(const std::vector<Sphere> &spheres, int iteration, std::string& directory, bool deferSaving);

// partition the screen into quarters and compute each partition using numThreads threads
void partitionAndRender(int iteration, std::string& directory, int numThreads, bool deferSaving);

// the screen is partitioned into numThreads partitions & each partition is computed on 1 thread
void threadPartitionRender(int iteration, std::string& directory, int numThreads, bool deferSaving);

void fileSave(int iteration, std::string& directory, bool upateTime);

void threadedFileSave(int iteration, std::string& directory, int startIndex, int endIndex);

void moveX(Sphere& toMove, float amount);

void moveY(Sphere& toMove, float amount);

void moveZ(Sphere& toMove, float amount);

void rotateX(Sphere& toRotate, float angle);

void rotateY(Sphere& toRotate, float angle);

void rotateZ(Sphere& toRotate, float angle);

void scale(Sphere& toScale, float amount);