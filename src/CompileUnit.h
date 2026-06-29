#ifndef COMPILEUNIT_H
#define COMPILEUNIT_H

#include "Basics.h"
#include "SourceBytes.h"

struct CompileUnit {
    SourceBytes src;
    MultiArray tokens;
};

#endif