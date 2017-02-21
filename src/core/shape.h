#ifndef PBRT_CORE_SHAPE_H
#define PBRT_CORE_SHAPE_H

#include"memory.h"
#include "geometry.h"
#include "transform.h"
#include "diffgeom.h"

// Shape Declarations
class Shape : public ReferenceCounted {
public:
	// Shape Interface
	Shape(const Transform *o2w, const Transform *w2o, bool ro);
	virtual ~Shape();
	virtual BBox ObjectBound() const = 0;
	virtual BBox WorldBound() const;
	virtual bool CanIntersect() const;
	virtual void Refine(vector<Reference<Shape> > &refined) const;
	virtual bool Intersect(const Ray &ray, float *tHit,
		float *rayEpsilon, DifferentialGeometry *dg) const;
	virtual bool IntersectP(const Ray &ray) const;
	virtual void GetShadingGeometry(const Transform &obj2world,
		const DifferentialGeometry &dg,
		DifferentialGeometry *dgShading) const {
		*dgShading = dg;
	}
	virtual float Area() const;
	virtual Point Sample(float u1, float u2, Normal *Ns) const {
		Severe("Unimplemented Shape::Sample() method called");
		return Point();
	}
	virtual float Pdf(const Point &Pshape) const {
		return 1.f / Area();
	}
	virtual Point Sample(const Point &P, float u1, float u2,
		Normal *Ns) const {
		return Sample(u1, u2, Ns);
	}
	virtual float Pdf(const Point &p, const Vector &wi) const;

	// Shape Public Data
	const Transform *ObjectToWorld, *WorldToObject;
	const bool ReverseOrientation, TransformSwapsHandedness;
	const uint32_t shapeId;
	static uint32_t nextshapeId;
};

#endif