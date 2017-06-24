
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

#ifndef PBRT_CORE_PRIMITIVE_H
#define PBRT_CORE_PRIMITIVE_H

// core/primitive.h*
#include "pbrt.h"
#include "shape.h"
#include "material.h"

/*
The Primitive class
provides additional nongeometric information about the primitive, such as its material
properties.
The abstract Primitive base class is the bridge between the geometry processing and
shading(着色) subsystems of pbrt.
*/

// Primitive Declarations
class Primitive : public ReferenceCounted {
public:
    // Primitive Interface
    Primitive() : primitiveId(nextprimitiveId++) { }
    virtual ~Primitive();
    virtual BBox WorldBound() const = 0;
    virtual bool CanIntersect() const;

	/*
	Primitive intersection methods return
	Intersection structures rather than DifferentialGeometry. These Intersection
	structures hold more information about the intersection than just the local geometric
	information, such as information about the material properties at the hit point.

	Another difference is that Shape::Intersect() returns the parametric distance along the
	ray to the intersection in a float * output variable, while Primitive::Intersect() is
	responsible for updating Ray::maxt with this value if an intersection is found.
	*/
    virtual bool Intersect(const Ray &r, Intersection *in) const = 0;
    virtual bool IntersectP(const Ray &r) const = 0;
    virtual void Refine(vector<Reference<Primitive> > &refined) const;

	/*
	It may be necessary to repeatedly refine a primitive until all of the primitives it has returned
	are themselves intersectable. The Primitive::FullyRefine() utility method handles
	this task.
	*/
    void FullyRefine(vector<Reference<Primitive> > &refined) const;

	/*
	Primitive::GetAreaLight(), returns a pointer to the
	AreaLight that describes the primitive’s emission distribution, if the primitive is itself a
	light source. If the primitive is not emissive, this method should return NULL
	*/
    virtual const AreaLight *GetAreaLight() const = 0;

	/*
	The other two methods return representations of the light-scattering(光散射) properties of the
	material at the given point on the surface.
	*/

	/*
	The first, Primitive::GetBSDF(), returns a BSDF
	object (introduced in Section 9.1) that describes local light-scattering properties at the
	intersection point. In addition to(除...之外) the differential geometry at the hit point, this method
	takes the object-to-world-space transformation and a MemoryArena to allocate memory
	for the returned BSDF.
	*/
    virtual BSDF *GetBSDF(const DifferentialGeometry &dg,
        const Transform &ObjectToWorld, MemoryArena &arena) const = 0;

	/*
	Primitive::GetBSSRDF(), returns a BSSRDF, which describes subsurface
	scattering inside the primitive—light that enters the surface at points far from where
	it exits.While subsurface light transport has little effect on the appearance of objects like
	metal, cloth, or plastic, it is the dominant(当主导地位) light-scattering mechanism(机制) for biological(生物) materials
	like skin, thick liquids(液体) like milk, etc.
	*/
    virtual BSSRDF *GetBSSRDF(const DifferentialGeometry &dg,
        const Transform &ObjectToWorld, MemoryArena &arena) const = 0;

    // Primitive Public Data
    const uint32_t primitiveId;
protected:
    // Primitive Protected Data
    static uint32_t nextprimitiveId;
};

/*
Shapes to be rendered directly are represented by the GeometricPrimitive class. This class
combines a Shape with a description of its appearance properties. So that the geometric
and shading portions of pbrt can be cleanly separated, these appearance properties are
encapsulated in the Material class.

The GeometricPrimitive class represents a single shape (e.g., a sphere) in the scene. One
GeometricPrimitive is allocated for each shape in the scene description provided by the
user.

Each GeometricPrimitive holds a reference to a Shape and its Material. In addition,
because primitives in pbrt may be area light sources, it stores a pointer to an AreaLight
object that describes its emission characteristics

*/
// GeometricPrimitive Declarations
class GeometricPrimitive : public Primitive {
public:
    // GeometricPrimitive Public Methods
    bool CanIntersect() const;
    void Refine(vector<Reference<Primitive> > &refined) const;
    virtual BBox WorldBound() const;
    virtual bool Intersect(const Ray &r, Intersection *isect) const;
    virtual bool IntersectP(const Ray &r) const;
    GeometricPrimitive(const Reference<Shape> &s,
                       const Reference<Material> &m, AreaLight *a);
    const AreaLight *GetAreaLight() const;

	/*
	The GetBSDF() method uses the Primitive’s Shape to find the shading geometry at the
	point and forwards the request(请求) on to the Material.
	*/
    BSDF *GetBSDF(const DifferentialGeometry &dg,
                  const Transform &ObjectToWorld, MemoryArena &arena) const;

	/*
	The GetBSDF() method uses the Primitive’s Shape to find the shading geometry at the
	point and forwards the request(请求) on to the Material.
	*/
    BSSRDF *GetBSSRDF(const DifferentialGeometry &dg,
                      const Transform &ObjectToWorld, MemoryArena &arena) const;
private:
    // GeometricPrimitive Private Data
    Reference<Shape> shape;
    Reference<Material> material;
    AreaLight *areaLight;
};


/*
The TransformedPrimitive class handles two more general uses of Shapes in the scene:
shapes with animated transformation matrices and object instancing, which can greatly
reduce the memory requirements for scenes that contain many instances of the same
geometry at different locations.
Implementing each of
these features essentially requires injecting an additional transformation matrix between
the Shape’s notion of world space and the actual scene world space. Therefore, both are
handled by a single class
*/
// TransformedPrimitive Declarations
class TransformedPrimitive : public Primitive {
public:
    // TransformedPrimitive Public Methods
    TransformedPrimitive(Reference<Primitive> &prim,
                         const AnimatedTransform &w2p)
        : primitive(prim), WorldToPrimitive(w2p) { }
    bool Intersect(const Ray &r, Intersection *in) const;
    bool IntersectP(const Ray &r) const;
    const AreaLight *GetAreaLight() const { return NULL; }
    BSDF *GetBSDF(const DifferentialGeometry &dg,
                  const Transform &ObjectToWorld, MemoryArena &arena) const {
        return NULL;
    }
    BSSRDF *GetBSSRDF(const DifferentialGeometry &dg,
                  const Transform &ObjectToWorld, MemoryArena &arena) const {
        return NULL;
    }
    BBox WorldBound() const {
        return WorldToPrimitive.MotionBounds(primitive->WorldBound(), true);
    }
private:
    // TransformedPrimitive Private Data
    Reference<Primitive> primitive;
    const AnimatedTransform WorldToPrimitive;
};


/*
the Aggregate base class, which represents a container that
can hold many Primitives. pbrt uses this class to implement acceleration structures—
data structures that help reduce the otherwise O(n) complexity of testing a ray for
intersection with all n objects in a scene. Most rays will intersect only a few primitives
and miss the others by a large distance.If an intersection acceleration technique can reject(拒绝)
whole groups of primitives at once, there will be a substantial(实质) performance improvement
compared to simply testing each ray against each primitive in turn. One benefit from
reusing the Primitive interface for these acceleration structures is that pbrt can support
hybrid(混合) approaches where an accelerator of one type holds accelerators of other types.
*/
// Aggregate Declarations
class Aggregate : public Primitive {
public:
    // Aggregate Public Methods
    const AreaLight *GetAreaLight() const;
    BSDF *GetBSDF(const DifferentialGeometry &dg,
                  const Transform &, MemoryArena &) const;
    BSSRDF *GetBSSRDF(const DifferentialGeometry &dg,
                  const Transform &, MemoryArena &) const;
};



#endif // PBRT_CORE_PRIMITIVE_H
