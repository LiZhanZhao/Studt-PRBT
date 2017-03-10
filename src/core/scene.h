

#ifndef PBRT_CORE_SCENE_H
#define PBRT_CORE_SCENE_H

// core/scene.h*
#include "pbrt.h"
#include "primitive.h"
#include "light.h"

// Scene Declarations
class Scene {
public:
    // Scene Public Methods
    Scene(Primitive *accel, const vector<Light *> &lts, VolumeRegion *vr);
    ~Scene();
    bool Intersect(const Ray &ray, Intersection *isect) const {
        bool hit = aggregate->Intersect(ray, isect);
        return hit;
    }
    bool IntersectP(const Ray &ray) const {
        bool hit = aggregate->IntersectP(ray);
        return hit;
    }
    const BBox &WorldBound() const;

    // Scene Public Data
    Primitive *aggregate;
    vector<Light *> lights;
    VolumeRegion *volumeRegion;
    BBox bound;
};



#endif // PBRT_CORE_SCENE_H
