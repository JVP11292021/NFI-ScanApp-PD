#ifndef MINMAP_IMAGE_H
#define MINMAP_IMAGE_H

#include "minmap_defs.hpp"

MM_NS_B

int RunImageDeleter(int argc, char** argv);
int RunImageFilterer(int argc, char** argv);
int RunImageRectifier(int argc, char** argv);
int RunImageRegistrator(int argc, char** argv);
int RunImageUndistorter(int argc, char** argv);
int RunImageUndistorterStandalone(int argc, char** argv);

MM_NS_E

#endif // MINMAP_IMAGE_H