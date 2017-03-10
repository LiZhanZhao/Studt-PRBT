

#ifndef PBRT_CORE_RENDERER_H
#define PBRT_CORE_RENDERER_H

// core/renderer.h*
#include "pbrt.h"
#include "scene.h"
#include "rng.h"

// Renderer Declarations
class Renderer {
public:
    // Renderer Interface
    virtual ~Renderer();
    virtual void Render(const Scene *scene) = 0;
    virtual Spectrum Li(const Scene *scene, const RayDifferential &ray,
        const Sample *sample, RNG &rng, MemoryArena &arena,
        Intersection *isect = NULL, Spectrum *T = NULL) const = 0;
    virtual Spectrum Transmittance(const Scene *scene,
        const RayDifferential &ray, const Sample *sample,
        RNG &rng, MemoryArena &arena) const = 0;
};



#endif // PBRT_CORE_RENDERER_H
