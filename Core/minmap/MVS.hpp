#ifndef MINMAP_MVS_H
#define MINMAP_MVS_H

#include "minmap_defs.hpp"

MM_NS_B

int RunDelaunayMesher(int argc, char** argv);
int RunPatchMatchStereo(int argc, char** argv);
int RunPoissonMesher(int argc, char** argv);
int RunStereoFuser(int argc, char** argv);

MM_NS_E

#endif // MINMAP_MVS_H