
/*
    pbrt source code Copyright(c) 1998-2012 Matt Pharr and Greg Humphreys.

    This file is part of pbrt.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PBRT_CORE_SHAPE_H
#define PBRT_CORE_SHAPE_H

// core/shape.h*
#include "pbrt.h"
#include "geometry.h"
#include "transform.h"
#include "diffgeom.h"
#include "memory.h"

/*
The Shape class
provides access to the raw geometric properties of the primitive, such as its surface
area and bounding box, and provides a ray intersection routine
*/

// Shape Declarations
class Shape : public ReferenceCounted {
public:
    // Shape Interface
    Shape(const Transform *o2w, const Transform *w2o, bool ro);
    virtual ~Shape();
    virtual BBox ObjectBound() const = 0;
    virtual BBox WorldBound() const;
	/*
	The default implementation of the Shape::CanIntersect() function indicates that a
	shape can compute ray intersections, so only shapes that are nonintersectable need to
	override this method.
	*/
    virtual bool CanIntersect() const;
	/*
	If the shape cannot be intersected directly, it must provide a Shape::Refine() method
	that splits the shape into a group of new shapes, some of which may be intersectable
	and some of which may need further refinement. The default implementation of the
	Shape::Refine() method issues an error message; thus, shapes that are intersectable
	(which is the common case) do not have to provide an empty instance of this method.
	pbrt will never call Shape::Refine() if Shape::CanIntersect() returns true.
	*/
    virtual void Refine(vector<Reference<Shape> > &refined) const;

	/*
	The rays passed into intersection routines are in world space, so shapes are responsible
	for transforming them to object space if needed for intersection tests. The
	differential geometry returned should be in world space.
	*/

	/*
	Shape::Intersect(), returns
	geometric information about a single ray-shape intersection corresponding to the
	first intersection, if any, in the [mint, maxt] parametric range along the ray
	*/
    virtual bool Intersect(const Ray &ray, float *tHit,
                           float *rayEpsilon, DifferentialGeometry *dg) const;

	/*
	Shape::IntersectP(), is a predicate function that determines whether or not an intersection
	occurs, without returning any details about the intersection itself
	*/
    virtual bool IntersectP(const Ray &ray) const;

	/*
	Some shapes (notably(”»∆‰) triangle meshes) support the idea of having two types of differential
	geometry at a point on the surface: the true geometry, which accurately reflects
	the local properties of the surface, and the shading geometry, which may have normals
	and tangents that are different than those in the true differential geometry. For triangle
	meshes, the user can provide normal vectors and primary tangents at the vertices of the
	mesh that are interpolated to give normals and tangents at points across the faces of triangles.
	Shading geometry with interpolated normals can make otherwise faceted triangle
	meshes appear to be more smooth than they geometrically are
	*/
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



#endif // PBRT_CORE_SHAPE_H
