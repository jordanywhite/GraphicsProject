
#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "vec.h"

struct materialStruct {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 emission;
	GLfloat shininess;
};

struct lightStruct {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 position;
};

#endif