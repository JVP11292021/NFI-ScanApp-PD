#ifndef MINMAP_DATABASE_H
#define MINMAP_DATABASE_H

#include "minmap_defs.hpp"

MM_NS_B

int RunDatabaseCleaner(int argc, char** argv);
int RunDatabaseCreator(int argc, char** argv);
int RunDatabaseMerger(int argc, char** argv);
int RunRigConfigurator(int argc, char** argv);

MM_NS_E

#endif // MINMAP_DATABASE_H