#include "RayTracer.h"
#include "Move.h"

std::chrono::time_point<std::chrono::system_clock> start;
std::chrono::time_point<std::chrono::system_clock> endTime;
std::chrono::duration<double> total_elapsed_time;
std::ofstream speedResults("O1_results.txt");
std::vector<Sphere> spheres;
Vec3f* image;

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

void threadedRender(int startIndex, int endIndex, int width, int startHeight, int endHeight, float invWidth, float invHeight, float angle, float aspectratio) {
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

void threadedRay(int x, int y, int width, float invHeight, float invWidth, float angle, float aspectratio) {
  Vec3f* pixel = image + ((y * width) + x);

  float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
  float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
  Vec3f raydir(xx, yy, -1);
  raydir.normalize();
  Vec3f result = trace(Vec3f(0), raydir, spheres, 0);
  *pixel = result;
}

void threadPoolRender(int numThreads, int iteration, std::string& directory) {
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
  image = new Vec3f[width * height];//, *pixel = image;
  Vec3f* pixel = image;
  float invWidth = 1 / float(width), invHeight = 1 / float(height);
  float fov = 30, aspectratio = width / float(height);
  float angle = tan(M_PI * 0.5 * fov / 180.);

  ThreadPool* pool = new ThreadPool(numThreads);

  int totalPixels = width * height;
  RayData data;
  data.width = width;
  data.invHeight = invHeight;
  data.invWidth = invWidth;
  data.angle = angle;
  data.aspectratio = aspectratio;

  for (unsigned y = 0; y < height; ++y) {
    for (unsigned x = 0; x < width; ++x, ++pixel) {
      std::cout << "(" << x << "," << y << ")" << std::endl;
      data.x = x;
      data.y = y;
      pool->enqueue(threadedRay, data);
    }
  }

  delete pool;

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
    ofs << (unsigned char) (std::min(float(1), image[i].x) * 255) <<
      (unsigned char) (std::min(float(1), image[i].y) * 255) <<
      (unsigned char) (std::min(float(1), image[i].z) * 255);
  }
  ofs.close();
  delete[] image;

  endTime = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_time = endTime - start;
  total_elapsed_time += elapsed_time;
  std::cout << "Finished image render in " << elapsed_time.count() << std::endl;
  speedResults << "Finished image render in " << elapsed_time.count() << std::endl;
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
  image = new Vec3f[width * height];//, *pixel = image;
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

  threadWorkload = totalPixels / 8;
  dividedHeight = height / 8;
  dividedWidth = width / 8;

  if (totalPixels % 8 != 0) {
    remainder = totalPixels - (threadWorkload * (8 - 1));
  }

  if (height % 8 != 0) {
    heightRemainder = height - (dividedHeight * (8 - 1));
  }

  if (width % 8 != 0) {
    widthRemainder = width - (dividedWidth * (8 - 1));
  }

  for (int i = 0; i < 8; i++) {
    int end = (i * threadWorkload) + threadWorkload;
    int heightEnd = (i * dividedHeight) + dividedHeight;
    int widthEnd = (i * dividedWidth) + dividedWidth;

    if (i == 8 - 1 && remainder > 0) {
      end = (i * threadWorkload) + remainder;
    }
    if (i == 8 - 1 && heightRemainder > 0) {
      heightEnd = (i * dividedHeight) + heightRemainder;
    }
    if (i == 8 - 1 && widthRemainder > 0) {
      widthEnd = (i * dividedWidth) + widthRemainder;
    }

    threads.push_back(std::thread(threadedRender, i * threadWorkload, end, width, i * dividedHeight, heightEnd, invWidth, invHeight, angle, aspectratio));
  }

  for (int i = 0; i < 8; i++) {
    threads[i].join();
  }

  // Trace rays
  //for (unsigned y = 0; y < height; ++y) {
  //  for (unsigned x = 0; x < width; ++x, ++pixel) {
  //    float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
  //    float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
  //    Vec3f raydir(xx, yy, -1);
  //    raydir.normalize();
  //    *pixel = trace(Vec3f(0), raydir, spheres, 0);
  //  }
  //}
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
    ofs << (unsigned char) (std::min(float(1), image[i].x) * 255) <<
      (unsigned char) (std::min(float(1), image[i].y) * 255) <<
      (unsigned char) (std::min(float(1), image[i].z) * 255);
  }
  ofs.close();
  delete[] image;

  endTime = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_time = endTime - start;
  total_elapsed_time += elapsed_time;
  std::cout << "Finished image render in " << elapsed_time.count() << std::endl;
  speedResults << "Finished image render in " << elapsed_time.count() << std::endl;
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

void doPass(int passOffset, int startIndex, int endIndex, std::vector<Move>& moves, std::string& directory) {
  //std::vector<Sphere> copy;

  //for (Sphere sphere : spheres) {
  //  copy.push_back(sphere);
  //}

  //for (auto move : moves) {
  //  move.doMove(startIndex, copy[move.getTargetSphere()]);
  //}
  //int i = startIndex;
  //while (i < endIndex) {
  //  for (auto move : moves) {
  //    move.doMove(copy[move.getTargetSphere()]);
  //  }
  //  render(copy, passOffset + i, directory);
  //  i++;
  //}

  //for (int i = 0; i < copy.size(); i++) {
  //  spheres[i] = copy[i];
  //}

  int i = startIndex;
  while (i < endIndex) {
    for (auto move : moves) {
      move.doMove(spheres[move.getTargetSphere()]);
    }
    render(spheres, passOffset + i, directory);
    i++;
  }
}