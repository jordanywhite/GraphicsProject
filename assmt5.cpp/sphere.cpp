// Sphere approximation by recursive subdivision of a tetrahedron, adapted from
// code in Angel, 6th edition.

#include "cs432.h"
#include "vec.h"
#include "sphere.h"

typedef vec4 color4;
typedef vec4 point4;

// four equally spaced points on the unit sphere
const point4 v[4]= {
	vec4(0.0, 0.0, 1.0, 1.0),
	vec4(0.0, 0.942809, -0.333333, 1.0),
	vec4(-0.816497, -0.471405, -0.333333, 1.0),
	vec4(0.816497, -0.471405, -0.333333, 1.0)
};

// move a point to unit circle
static point4 unit(const point4 &p) {
	point4 c;
	double d=0.0;
	for(int i=0; i<3; i++) d+=p[i]*p[i];
	d=sqrt(d);
	if(d > 0.0) for(int i=0; i<3; i++) c[i] = p[i]/d;
	c[3] = 1.0;
	return c;
}

// maps a point on the unit sphere to a texture coordinate in the unit square with x and
// y in the range [0,1]. The front/center of the sphere is considered to be (0,0,1), which
// maps to the (s,t) coordinate (0.5,0.5) the back of the sphere (where t=0 and t=1 segments
// meet) is the half circle starting at (0,1,0), running through (0,0,-1), and ending at
// (0,-1,0).
static vec2 mapSphereToTextureSpot(vec4 v) {
	float tVal = (1 + v.y)/2; // y value just maps from [-1,1] to [0,1]
	float xzVecLength = sqrt(v.x*v.x + v.z*v.z);
	float sVal;
	if (xzVecLength == 0) {
		sVal = 0.5;
	}
	else {
		// if z value is 1, then were pointing in the direction of the z axis, which would
		// correspond to s=0.5. Otherwise, if the x component is > 0, then the s-value is mapped
		// from the arc-cosine 0..PI to 0.5..1.0; if the x component is < 0, then the s-value is
		// mapped from the arc-cosine 0..PI to 0.5..0.0
		float zNormal = v.z/xzVecLength;
		float zMappedDist = acos(zNormal)/M_PI/2; // 0 and 1
		if (v.x < 0) {
			sVal = 0.5-zMappedDist;
		}
		else {
			sVal = 0.5+zMappedDist;
		}
	}
	return vec2(sVal, tVal);
}

static void triangle(point4  a, point4 b, point4 c, color4 color,
			  int* idxSpot, point4* pointArray, color4* colorArray,
				vec3* normalArray, vec2* texArray, bool flatShading) {
	if (idxSpot[0]+3 > idxSpot[1]) {
		*idxSpot += 3;
	}
	else {
		if (pointArray != NULL) pointArray[*idxSpot]= a;
		if (colorArray != NULL) colorArray[*idxSpot] = color;
		vec3 flatNormal;
		if (normalArray != NULL) {
			if (flatShading) {
				flatNormal = normalize(cross(b-a,b-c));
				normalArray[*idxSpot] = flatNormal;
			}
			else {
				vec4 nma = normalize(a);
				normalArray[*idxSpot] = vec3(nma.x,nma.y,nma.z);
			}
		}
		if (texArray != NULL) {
			vec2 texValue = mapSphereToTextureSpot(a);
			texArray[*idxSpot] = texValue;
		}		
		++*idxSpot;
		if (pointArray != NULL) pointArray[*idxSpot] = b;
		if (colorArray != NULL) colorArray[*idxSpot] = color;
		if (normalArray != NULL) {
			if (flatShading) {
				normalArray[*idxSpot] = flatNormal;	
			}
			else {
				vec4 nmb = normalize(b);
				normalArray[*idxSpot] = vec3(nmb.x,nmb.y,nmb.z);
			}
		}
		if (texArray != NULL) {
			vec2 texValue = mapSphereToTextureSpot(b);
			texArray[*idxSpot] = texValue;
		}		
		++*idxSpot;
		if (pointArray != NULL) pointArray[*idxSpot] = c;
		if (colorArray != NULL) colorArray[*idxSpot] = color;
		if (normalArray != NULL) {
			if (flatShading) {
				normalArray[*idxSpot] = flatNormal;
			}
			else {
				vec4 nmc = normalize(c);
				normalArray[*idxSpot] = vec3(nmc.x,nmc.y,nmc.z);
			}
		}
		if (texArray != NULL) {
			vec2 texValue = mapSphereToTextureSpot(c);
			texArray[*idxSpot] = texValue;
		}
		++*idxSpot;
	}
}


static void divide_triangle(point4 a, point4 b, point4 c, color4 color, int n,
							int* idxSpot, point4* pointArray, color4* colorArray,
							vec3* normalArray, vec2* texArray, bool flatShading = false) {
	point4 v1, v2, v3;
	if(n>0) {
		v1 = unit(a + b);
		v2 = unit(a + c);
		v3 = unit(b + c);   
		divide_triangle(a ,v2, v1, color, n-1, idxSpot, pointArray, colorArray, normalArray, texArray, flatShading);
		divide_triangle(c ,v3, v2, color, n-1, idxSpot, pointArray, colorArray, normalArray, texArray, flatShading);
		divide_triangle(b ,v1, v3, color, n-1, idxSpot, pointArray, colorArray, normalArray, texArray, flatShading);
		divide_triangle(v1 ,v2, v3, color, n-1, idxSpot, pointArray, colorArray, normalArray, texArray, flatShading);
	}
	else {
		triangle(a, c, b, color, idxSpot, pointArray, colorArray, normalArray, texArray, flatShading);
	}
}

ObjRef genSphere(color4 color, int n, int* idxSpot,
					   point4* pointArray, color4* colorArray, vec3* normalArray=NULL, vec2* texArray) {
	int startIdx = *idxSpot;
	divide_triangle(v[0], v[1], v[2], color, n, idxSpot, pointArray, colorArray, normalArray, texArray);
	divide_triangle(v[3], v[2], v[1], color, n, idxSpot, pointArray, colorArray, normalArray, texArray);
	divide_triangle(v[0], v[3], v[1], color, n, idxSpot, pointArray, colorArray, normalArray, texArray);
	divide_triangle(v[0], v[2], v[3], color, n, idxSpot, pointArray, colorArray, normalArray, texArray);
	return ObjRef(startIdx, *idxSpot);
}

ObjRef genFlatSphere(color4 color, int n, int* idxSpot,
				 point4* pointArray, color4* colorArray, vec3* normalArray=NULL, vec2* texArray) {
	int startIdx = *idxSpot;
	divide_triangle(v[0], v[1], v[2], color, n, idxSpot, pointArray, colorArray, normalArray, texArray, true);
	divide_triangle(v[3], v[2], v[1], color, n, idxSpot, pointArray, colorArray, normalArray, texArray, true);
	divide_triangle(v[0], v[3], v[1], color, n, idxSpot, pointArray, colorArray, normalArray, texArray, true);
	divide_triangle(v[0], v[2], v[3], color, n, idxSpot, pointArray, colorArray, normalArray, texArray, true);
	return ObjRef(startIdx, *idxSpot);
}

