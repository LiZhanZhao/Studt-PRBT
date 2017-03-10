
// core/scene.cpp*
#include "scene.h"
#include "volume.h"

// Scene Method Definitions
Scene::~Scene() {
    delete aggregate;
    delete volumeRegion;
    for (uint32_t i = 0; i < lights.size(); ++i)
        delete lights[i];
}


Scene::Scene(Primitive *accel, const vector<Light *> &lts,
             VolumeRegion *vr) {
    lights = lts;
    aggregate = accel;
    volumeRegion = vr;
    // Scene Constructor Implementation
    bound = aggregate->WorldBound();
    if (volumeRegion) bound = Union(bound, volumeRegion->WorldBound());
}


const BBox &Scene::WorldBound() const {
    return bound;
}


