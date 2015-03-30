#ifndef __OBJPARSE_H__
#define __OBJPARSE_H__
#include "cs432.h"
#include "vec.h"
#include "mat.h"
#include "matStack.h"
#include <fstream>
#include <sstream>
#include <istream>
#include <assert.h>
#include <stdio.h>
#include <ctime>
#include <vector>
#include <string>

extern ObjRef genObject(std::string path, int* idxVar, vec4* pointsArray,
				 vec4* colorsArray, vec3* normalArray, vec2*texArray);

#endif
