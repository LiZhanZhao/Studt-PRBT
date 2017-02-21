
// core/shape.cpp*
#include "shape.h"

// Shape Method Definitions
Shape::~Shape() {
}


Shape::Shape(const Transform *o2w, const Transform *w2o, bool ro)
    : ObjectToWorld(o2w), WorldToObject(w2o), ReverseOrientation(ro),
      TransformSwapsHandedness(o2w->SwapsHandedness()),
      shapeId(nextshapeId++) {
    // Update shape creation statistics
    //PBRT_CREATED_SHAPE(this);
}


uint32_t Shape::nextshapeId = 1;
BBox Shape::WorldBound() const {
    return (*ObjectToWorld)(ObjectBound());
}


bool Shape::CanIntersect() const {
    return true;
}


void Shape::Refine(vector<Reference<Shape> > &refined) const {
    Severe("Unimplemented Shape::Refine() method called");
}


bool Shape::Intersect(const Ray &ray, float *tHit, float *rayEpsilon,
                      DifferentialGeometry *dg) const {
    Severe("Unimplemented Shape::Intersect() method called");
    return false;
}


bool Shape::IntersectP(const Ray &ray) const {
    Severe("Unimplemented Shape::IntersectP() method called");
    return false;
}


float Shape::Area() const {
    Severe("Unimplemented Shape::Area() method called");
    return 0.;
}


float Shape::Pdf(const Point &p, const Vector &wi) const {
    // Intersect sample ray with area light geometry
    DifferentialGeometry dgLight;
    Ray ray(p, wi, 1e-3f);
    ray.depth = -1; // temporary hack to ignore alpha mask
    float thit, rayEpsilon;
    if (!Intersect(ray, &thit, &rayEpsilon, &dgLight)) return 0.;

    // Convert light sample weight to solid angle measure
    float pdf = DistanceSquared(p, ray(thit)) /
                (AbsDot(dgLight.nn, -wi) * Area());
    if (isinf(pdf)) pdf = 0.f;
    return pdf;
}


