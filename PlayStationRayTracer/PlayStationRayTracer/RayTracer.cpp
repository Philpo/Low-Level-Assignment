#include "RayTracer.h"
#include "Move.h"

std::chrono::time_point<std::chrono::system_clock> start;
std::chrono::time_point<std::chrono::system_clock> endTime;
std::chrono::duration<double> total_elapsed_time;
std::ofstream speedResults;
std::vector<Sphere> spheres;
std::vector<Vec3f*> images;
LinearAllocator onionAllocator;

void* operator new[](size_t size, off_t& physicalAddress) {
  int32_t ret = sceKernelAllocateDirectMemory(
    0,
    sceKernelGetDirectMemorySize() - 1,
    size,
    0,
    SCE_KERNEL_WB_ONION,
    &physicalAddress);

  assert(ret == SCE_OK);

  void* baseAddress = NULL;
  ret = sceKernelMapDirectMemory(
    &baseAddress,
    size,
    SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_ALL,
    0,
    physicalAddress,
    0);
  assert(ret == SCE_OK);

  return baseAddress;
}

void operator delete[](void* data, size_t size, off_t& physicalAddress) {
  int32_t ret = sceKernelMunmap(data, size);
  assert(ret == SCE_OK);
  ret = sceKernelReleaseDirectMemory(physicalAddress, size);
  assert(ret == SCE_OK);
}

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
Vec3f trace(const Vec3f &rayorig, const Vec3f &raydir, const std::vector<Sphere> &spheres, const int &depth) {
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
        surfaceColor += sphere->surfaceColor * transmission * std::max(float(0), nhit.dot(lightDirection)) * spheres[i].emissionColor;
      }
    }
  }

  return surfaceColor + sphere->emissionColor;
}

void threadedRender(int startIndex, int width, int startHeight, int endHeight, float invWidth, float invHeight, float angle, float aspectratio, Vec3f* image) {
  Vec3f* pixel = image + startIndex;

  for (unsigned y = startHeight; y < endHeight; ++y) {
    for (unsigned x = 0; x < width; ++x, ++pixel) {
      float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
      float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
      Vec3f raydir(xx, yy, -1);
      raydir.normalize();
      Vec3f result = trace(Vec3f(0), raydir, spheres, 0);
      *pixel = result;
    }
  }
}

void partionedRender(int numThreads, int top, int width, int height, float invWidth, float invHeight, float angle, float aspectratio, Vec3f* image) {
  int threadWorkload = 0;
  int heightRemainder = 0;
  int dividedHeight = 0;
  std::vector<std::thread> threads;

  dividedHeight = height / numThreads;

  if (height % numThreads != 0) {
    heightRemainder = height - (dividedHeight * (numThreads - 1));
  }

  threadWorkload = dividedHeight * width;

  for (int i = 0; i < numThreads; i++) {
    int heightEnd = (i * dividedHeight) + dividedHeight;

    if (i == (numThreads - 1) && heightRemainder > 0) {
      heightEnd = (i * dividedHeight) + heightRemainder;
    }


    threads.push_back(std::thread(threadedRender, (top * width) + (i * threadWorkload), width, top + (i * dividedHeight),  top + heightEnd, invWidth, invHeight, angle, aspectratio, image));
  }

  for (int i = 0; i < numThreads; i++) {
    threads[i].join();
  }
}

//[comment]
// Main rendering function. We compute a camera ray for each pixel of the image
// trace it and return a color. If the ray hits a sphere, we return the color of the
// sphere at the intersection point, else we return the background color.
//[/comment]
void renderFrame(const std::vector<Sphere> &spheres, int iteration, std::string& directory, bool deferSaving) {
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

  // quick and dirty
#ifdef _DEBUG
  unsigned width = 640, height = 480;
#else
  unsigned width = 1920, height = 1080;
#endif

  size_t totalSize = sizeof(Vec3f)* width * height;

  void* buffer = onionAllocator.allocate(totalSize, sce::Gnm::kAlignmentOfBufferInBytes);

  Vec3f* image = reinterpret_cast<Vec3f *>(buffer);
  Vec3f* pixel = image;
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
  endTime = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_time = endTime - start;
  std::cout << "Finished trace in " << elapsed_time.count() << std::endl;
  speedResults << "Finished trace in " << elapsed_time.count() << std::endl;

  fileSave(iteration, directory, true);
}

void partitionAndRender(int iteration, std::string& directory, int numThreads, bool deferSaving) {
	start = std::chrono::system_clock::now();

	// quick and dirty
#ifdef _DEBUG
	unsigned width = 640, height = 480;
#else
	unsigned width = 1920, height = 1080;
#endif

  size_t totalSize = sizeof(Vec3f)* width * height;

  void* buffer = onionAllocator.allocate(totalSize, sce::Gnm::kAlignmentOfBufferInBytes);

  Vec3f* image = reinterpret_cast<Vec3f *>(buffer);
	Vec3f* pixel = image;
	float invWidth = 1 / float(width), invHeight = 1 / float(height);
	float fov = 30, aspectratio = width / float(height);
	float angle = tan(M_PI * 0.5 * fov / 180.);

	int totalPixels = width * height;

	std::vector<std::thread> threads;

	int threadWorkload = 0;
	int remainder = 0;
	int heightRemainder = 0;
	int widthRemainder = 0;
	int dividedHeight = 0;
	int dividedWidth = 0;

	dividedHeight = height / 4;

	partionedRender(numThreads, 0, width, dividedHeight, invWidth, invHeight, angle, aspectratio, image);
  partionedRender(numThreads, dividedHeight, width, dividedHeight, invWidth, invHeight, angle, aspectratio, image);
  partionedRender(numThreads, dividedHeight * 2, width, dividedHeight, invWidth, invHeight, angle, aspectratio, image);
  partionedRender(numThreads, dividedHeight * 3, width, dividedHeight, invWidth, invHeight, angle, aspectratio, image);

  endTime = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_time = endTime - start;
  total_elapsed_time += elapsed_time;
  std::cout << "Finished trace in " << elapsed_time.count() << std::endl;
  speedResults << "Finished trace in " << elapsed_time.count() << std::endl;

  fileSave(iteration, directory, true);
}

void threadPartitionRender(int iteration, std::string& directory, int numThreads, bool deferSaving) {
	start = std::chrono::system_clock::now();

	// quick and dirty
#ifdef _DEBUG
	unsigned width = 640, height = 480;
#else
	unsigned width = 1920, height = 1080;
#endif

  size_t totalSize = sizeof(Vec3f)* width * height;

  void* buffer = onionAllocator.allocate(totalSize, sce::Gnm::kAlignmentOfBufferInBytes);

  Vec3f* image = reinterpret_cast<Vec3f *>(buffer);
	Vec3f* pixel = image;
	float invWidth = 1 / float(width), invHeight = 1 / float(height);
	float fov = 30, aspectratio = width / float(height);
	float angle = tan(M_PI * 0.5 * fov / 180.);

	int totalPixels = width * height;

	std::vector<std::thread> threads;

	int threadWorkload = 0;
	int remainder = 0;
	int heightRemainder = 0;
	int widthRemainder = 0;
	int dividedHeight = 0;
	int dividedWidth = 0;

	threadWorkload = totalPixels / numThreads;
	dividedHeight = height / numThreads;
	dividedWidth = width / numThreads;

	if (height % numThreads != 0) {
	  heightRemainder = height - (dividedHeight * (numThreads - 1));
	}

	for (int i = 0; i < numThreads; i++) {
	  int heightEnd = (i * dividedHeight) + dividedHeight;
	  if (i == numThreads - 1 && heightRemainder > 0) {
	    heightEnd = (i * dividedHeight) + heightRemainder;
	  }

    threads.push_back(std::thread(threadedRender, i * threadWorkload, width, i * dividedHeight, heightEnd, invWidth, invHeight, angle, aspectratio, image));
	}

	for (int i = 0; i < numThreads; i++) {
	  threads[i].join();
  }

  endTime = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_time = endTime - start;
  total_elapsed_time += elapsed_time;
  std::cout << "Finished trace in " << elapsed_time.count() << std::endl;
  speedResults << "Finished trace in " << elapsed_time.count() << std::endl;

  fileSave(iteration, directory, true);
}

void fileSave(int iteration, std::string& directory, bool updateTime, Vec3f*& image) {
#ifdef _DEBUG
  unsigned width = 640, height = 480;
#else
  unsigned width = 1920, height = 1080;
#endif

  std::chrono::time_point<std::chrono::system_clock> savingStart = std::chrono::system_clock::now();

  // Save result to a PPM image (keep these flags if you compile under Windows)
  std::stringstream ss;
  if (iteration < 10) {
    ss << "/" + directory + "/spheres00" << iteration << ".ppm";
  }
  else if (iteration < 100) {
    ss << "/" + directory + "/spheres0" << iteration << ".ppm";
  }
  else {
    ss << "/" + directory + "/spheres" << iteration << ".ppm";
  }
  std::string tempString = ss.str();
  char* filename = (char*) tempString.c_str();

  std::ofstream ofs(filename, std::ios::out | std::ios::binary);
  ofs << "P6\n" << width << " " << height << "\n255\n";
  for (unsigned i = 0; i < width * height; ++i) {
    ofs << (unsigned char) (std::min(float(1), image[i].x) * 255) <<
      (unsigned char) (std::min(float(1), image[i].y) * 255) <<
      (unsigned char) (std::min(float(1), image[i].z) * 255);
  }
  ofs.close();
  //delete[] image;

  endTime = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_time = endTime - savingStart;
  if (updateTime) {
    total_elapsed_time += elapsed_time;
  }
  std::cout << "Finished saving in " << elapsed_time.count() << std::endl;
  speedResults << "Finished saving in " << elapsed_time.count() << std::endl;
}

void threadedFileSave(int iteration, std::string& directory, int startIndex, int endIndex) {
#ifdef _DEBUG
  unsigned width = 640, height = 480;
#else
  unsigned width = 1920, height = 1080;
#endif

  for (int i = startIndex; i < endIndex; i++) {
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
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
    for (unsigned j = 0; j < width * height; ++j) {
      ofs << (unsigned char) (std::min(float(1), images[i][j].x) * 255) <<
        (unsigned char) (std::min(float(1), images[i][j].y) * 255) <<
        (unsigned char) (std::min(float(1), images[i][j].z) * 255);
    }
    ofs.close();
    delete[] images[i];

    std::chrono::time_point<std::chrono::system_clock> endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_time = endTime - start;
    std::cout << "Finished saving in " << elapsed_time.count() << std::endl;
    speedResults << "Finished saving in " << elapsed_time.count() << std::endl;
    iteration++;
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
  angle = angle * M_PI / 180.0f;
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
  angle = angle * M_PI / 180.0f;
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
  angle = angle * M_PI / 180.0f;
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

void scale(Sphere& toScale, float amount) {
  toScale.radius += amount;
  toScale.radius2 = toScale.radius * toScale.radius;
}