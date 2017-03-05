#ifndef PBRT_CORE_INTERSECTION_H
#define PBRT_CORE_INTERSECTION_H

// core/intersection.h*
#include "pbrt.h"
#include "diffgeom.h"
#include "transform.h"
#include "Primitive.h"

// Intersection Declarations
struct Intersection {
    // Intersection Public Methods
    Intersection() {
        primitive = NULL;
        shapeId = primitiveId = 0;
        rayEpsilon = 0.f;
    }
    /*BSDF *GetBSDF(const RayDifferential &ray, MemoryArena &arena) const;
    BSSRDF *GetBSSRDF(const RayDifferential &ray, MemoryArena &arena) const;
    Spectrum Le(const Vector &wo) const;*/

    // Intersection Public Data
    DifferentialGeometry dg;
    const Primitive *primitive;
    Transform WorldToObject, ObjectToWorld;
    uint32_t shapeId, primitiveId;
    float rayEpsilon;
};



#endif // PBRT_CORE_INTERSECTION_H
