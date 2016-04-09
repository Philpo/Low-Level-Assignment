#include <stdio.h>
#include <stdlib.h>
#include <scebase.h>
#include <kernel.h>
#include <gnmx.h>
#include <video_out.h>

#include <cstdio>
#include <cmath>
#include <fstream>
#include <vector>
#include <iostream>
#include <cassert>

#include <algorithm>
#include <sstream>
#include <string.h>

// Time precision
#include <chrono>

// Threading
#include "rapidxml.hpp"
#include "rayTracer.h"
#include "StringUtils.h"
#include "Pass.h"
#include <fios2.h>

/*E The FIOS2 default maximum path is 1024, games can normally use a much smaller value. */
#define MAX_PATH_LENGTH 256

/*E Buffers for FIOS2 initialization.
* These are typical values that a game might use, but adjust them as needed. They are
* of type int64_t to avoid alignment issues. */

/* 64 ops: */
int64_t g_OpStorage[SCE_FIOS_DIVIDE_ROUNDING_UP(SCE_FIOS_OP_STORAGE_SIZE(64, MAX_PATH_LENGTH), sizeof(int64_t))];
/* 1024 chunks, 64KiB: */
int64_t g_ChunkStorage[SCE_FIOS_DIVIDE_ROUNDING_UP(SCE_FIOS_CHUNK_STORAGE_SIZE(1024), sizeof(int64_t))];
/* 16 file handles: */
int64_t g_FHStorage[SCE_FIOS_DIVIDE_ROUNDING_UP(SCE_FIOS_FH_STORAGE_SIZE(16, MAX_PATH_LENGTH), sizeof(int64_t))];
/* 1 directory handle: */
int64_t g_DHStorage[SCE_FIOS_DIVIDE_ROUNDING_UP(SCE_FIOS_DH_STORAGE_SIZE(1, MAX_PATH_LENGTH), sizeof(int64_t))];

static const size_t kOnionMemorySize = 64 * 1024 * 1024;

static const int num_threads = 10;

using namespace sce;
using namespace sce::Gnmx;
using namespace rapidxml;

int main(int argc, char **argv) {
  SceFiosParams params = SCE_FIOS_PARAMS_INITIALIZER;

  /*E Provide required storage buffers. */
  params.opStorage.pPtr = g_OpStorage;
  params.opStorage.length = sizeof(g_OpStorage);
  params.chunkStorage.pPtr = g_ChunkStorage;
  params.chunkStorage.length = sizeof(g_ChunkStorage);
  params.fhStorage.pPtr = g_FHStorage;
  params.fhStorage.length = sizeof(g_FHStorage);
  params.dhStorage.pPtr = g_DHStorage;
  params.dhStorage.length = sizeof(g_DHStorage);

  params.pathMax = MAX_PATH_LENGTH;

  params.pVprintf = vprintf;
  params.pMemcpy = memcpy;

  sceFiosInitialize(&params);

  std::string spheresFilePath, movesFilePath, threadMethod, ioMethod, outputFile;
  int numThreads = 1;

  if (argc == 9 || argc == 10) {
    numThreads = 0;
    for (int i = 1; i < 5; i += 2) {
      if (strcmp(argv[i], "-i") == 0) {
        spheresFilePath = argv[i + 1];
      }
      else if (strcmp(argv[i], "-m") == 0) {
        movesFilePath = argv[i + 1];
      }
    }

    if (argc == 9) {
      if (strcmp(argv[5], "-tf") == 0) {
        threadMethod = "tf";
      }
      else {
        return -1;
      }
      if (strcmp(argv[6], "-tio") == 0) {
        ioMethod = "tio";
      }
      else if (strcmp(argv[6], "-io") == 0) {
        ioMethod = "io";
      }
      else {
        return -1;
      }
      if (strcmp(argv[7], "-f") == 0) {
        outputFile == argv[8];
      }
      else {
        return -1;
      }
    }
    else if (argc == 10) {
      if (strcmp(argv[5], "-p") == 0) {
        threadMethod = "p";
        numThreads = convertStringToNumber<int>(argv[6]);
      }
      else if (strcmp(argv[5], "-tp") == 0) {
        threadMethod = "tp";
        numThreads = convertStringToNumber<int>(argv[6]);
      }
      else {
        return -1;
      }
      if (strcmp(argv[7], "-tio") == 0) {
        ioMethod = "tio";
      }
      else if (strcmp(argv[7], "-io") == 0) {
        ioMethod = "io";
      }
      else {
        return -1;
      }
      if (strcmp(argv[8], "-f") == 0) {
        outputFile == argv[9];
      }
      else {
        return -1;
      }
    }

    std::string directory;
    auto now = std::time(nullptr);
    std::ostringstream os;
    os << std::put_time(std::gmtime(&now), "%Y-%m-%d_%H%M%S");
    directory = "/app0/spheres" + os.str();

    std::vector<Pass> passes;
    int passCount = 0;

    char *pInput = NULL;
    SceFiosSize inputSize = 0;
    SceFiosSize result = 0;
    SceFiosFH handle = 0;
    SceFiosOp op = 0;
    SceFiosOpenParams openParams = SCE_FIOS_OPENPARAMS_INITIALIZER;

    op = sceFiosDirectoryCreate(NULL, directory.c_str());
    assert(op != SCE_FIOS_OP_INVALID);
    result = sceFiosOpWait(op);
    assert(result == SCE_OK);
    sceFiosOpDelete(op);

    try {
      op = sceFiosFHOpen(NULL, &handle, spheresFilePath.c_str(), &openParams);
      assert(op != SCE_FIOS_OP_INVALID);

      result = sceFiosOpSyncWait(op);
      assert(result == SCE_FIOS_OK);
      sceFiosOpDelete(op);

      inputSize = sceFiosFileGetSizeSync(NULL, spheresFilePath.c_str());

      pInput = (char*) malloc((size_t) inputSize);
      result = sceFiosFileReadSync(NULL, spheresFilePath.c_str(), pInput, inputSize, 0);

      string temp(pInput, inputSize);

      xml_document<> doc;
      doc.parse<0>((char*) &temp[0]);
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

      free(pInput);

      op = sceFiosFHClose(NULL, handle);
      result = sceFiosOpSyncWait(op);
      sceFiosOpDelete(op);
    }
    catch (parse_error& e) {
      ofstream errorFile;
      errorFile.open("error_file.txt");
      errorFile << "Error reading scene file " << ": " << e.what() << endl;
      errorFile.close();

      free(pInput);

      op = sceFiosFHClose(NULL, handle);
      result = sceFiosOpSyncWait(op);
      sceFiosOpDelete(op);
      return 0;
    }

    //speedResults.open(outputFile);

    try {
      op = sceFiosFHOpen(NULL, &handle, movesFilePath.c_str(), &openParams);
      assert(op != SCE_FIOS_OP_INVALID);

      result = sceFiosOpSyncWait(op);
      assert(result == SCE_FIOS_OK);
      sceFiosOpDelete(op);

      inputSize = sceFiosFileGetSizeSync(NULL, movesFilePath.c_str());

      pInput = (char*) malloc((size_t) inputSize);
      result = sceFiosFileReadSync(NULL, movesFilePath.c_str(), pInput, inputSize, 0);

      string temp(pInput, inputSize);

      xml_document<> doc;
      doc.parse<0>((char*) &temp[0]);
      xml_node<>* rootNode = doc.first_node();

      for (xml_node<>* passNode = rootNode->first_node(); passNode; passNode = passNode->next_sibling()) {
        passes.push_back(Pass(passNode, directory, passCount++, threadMethod, numThreads, ioMethod));
      }
      free(pInput);

      op = sceFiosFHClose(NULL, handle);
      result = sceFiosOpSyncWait(op);
      sceFiosOpDelete(op);
    }
    catch (parse_error& e) {
      ofstream errorFile;
      errorFile.open("error_file.txt");
      errorFile << "Error reading pass file " << ": " << e.what() << endl;
      errorFile.close();

      free(pInput);

      op = sceFiosFHClose(NULL, handle);
      result = sceFiosOpSyncWait(op);
      sceFiosOpDelete(op);
      return 0;
    }

    for (Pass pass : passes) {
      pass.render();
    }

    //op = sceFiosFileDelete(NULL, "/app0/spheres000.ppm");
    //result = sceFiosOpSyncWait(op);
    //sceFiosOpDelete(op);
  }
  return 0;
}