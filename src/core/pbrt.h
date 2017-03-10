#ifndef PBRT_CORE_PBRT_H
#define PBRT_CORE_PBRT_H

#include <math.h>

#include <windows.h>
#include <stdint.h>

#include <vector>
using std::vector;

#include <string>
using std::string;

#include <algorithm>
using std::min;
using std::max;
using std::swap;
using std::sort;

#include "error.h"

#ifdef NDEBUG
#define Assert(expr) ((void)0)
#else
#define Assert(expr) \
	((expr) ? (void)0 : \
	Severe("Assertion \"%s\" failed in %s, line %d", \
#expr, __FILE__, __LINE__))
#endif // NDEBUG



#ifndef PBRT_L1_CACHE_LINE_SIZE
#define PBRT_L1_CACHE_LINE_SIZE 64
#endif

#define M_PI       3.14159265358979323846f
#define INV_PI     0.31830988618379067154f
#define INV_TWOPI  0.15915494309189533577f
#define INV_FOURPI 0.07957747154594766788f

#define ALLOCA(TYPE, COUNT) (TYPE *)alloca((COUNT) * sizeof(TYPE))

// Global Inline Functions
inline float Lerp(float t, float v1, float v2) {
	return (1.f - t) * v1 + t * v2;
}


inline float Clamp(float val, float low, float high) {
	if (val < low) return low;
	else if (val > high) return high;
	else return val;
}


inline int Clamp(int val, int low, int high) {
	if (val < low) return low;
	else if (val > high) return high;
	else return val;
}


inline int Mod(int a, int b) {
	int n = int(a / b);
	a -= n*b;
	if (a < 0) a += b;
	return a;
}


inline float Radians(float deg) {
	return ((float)M_PI / 180.f) * deg;
}


inline float Degrees(float rad) {
	return (180.f / (float)M_PI) * rad;
}


inline float Log2(float x) {
	static float invLog2 = 1.f / logf(2.f);
	return logf(x) * invLog2;
}


inline int Floor2Int(float val);
inline int Log2Int(float v) {
	return Floor2Int(Log2(v));
}


inline bool IsPowerOf2(int v) {
	return v && !(v & (v - 1));
}


inline uint32_t RoundUpPow2(uint32_t v) {
	v--;
	v |= v >> 1;    v |= v >> 2;
	v |= v >> 4;    v |= v >> 8;
	v |= v >> 16;
	return v + 1;
}


inline int Floor2Int(float val) {
	return (int)floorf(val);
}


inline int Round2Int(float val) {
	return Floor2Int(val + 0.5f);
}


inline int Float2Int(float val) {
	return (int)val;
}


inline int Ceil2Int(float val) {
	return (int)ceilf(val);
}

class Vector;
class Point;
class Normal;
class Ray;
class RayDifferential;
class BBox;
class Transform;
struct DifferentialGeometry;
class Shape;
class Intersection;
class Primitive;
template <int nSamples> class CoefficientSpectrum;
class RGBSpectrum;
typedef RGBSpectrum Spectrum;
class VolumeRegion;
class Sampler;
struct CameraSample;
struct LightSample;
struct LightSampleOffsets;
struct VisibilityTester;
struct Distribution1D;
struct Sample;
class Integrator;
class SurfaceIntegrator;
class VolumeIntegrator;


#endif