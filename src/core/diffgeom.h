

#ifndef PBRT_CORE_DIFFGEOM_H
#define PBRT_CORE_DIFFGEOM_H

// core/diffgeom.h*
#include "pbrt.h"
#include "geometry.h"

// DifferentialGeometry Declarations
struct DifferentialGeometry {
    DifferentialGeometry() { 
        u = v = dudx = dvdx = dudy = dvdy = 0.; 
        shape = NULL; 
    }
    // DifferentialGeometry Public Methods
    DifferentialGeometry(const Point &P, const Vector &DPDU,
            const Vector &DPDV, const Normal &DNDU,
            const Normal &DNDV, float uu, float vv,
            const Shape *sh);
    void ComputeDifferentials(const RayDifferential &r) const;

    // DifferentialGeometry Public Data
    Point p;
    Normal nn;
    float u, v;
    const Shape *shape;
    Vector dpdu, dpdv;
    Normal dndu, dndv;
    mutable Vector dpdx, dpdy;
    mutable float dudx, dvdx, dudy, dvdy;
};



#endif // PBRT_CORE_DIFFGEOM_H
