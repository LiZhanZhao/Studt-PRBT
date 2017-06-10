
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


// integrators/whitted.cpp*
#include "stdafx.h"
#include "integrators/whitted.h"
#include "intersection.h"
#include "paramset.h"

// WhittedIntegrator Method Definitions
Spectrum WhittedIntegrator::Li(const Scene *scene,
        const Renderer *renderer, const RayDifferential &ray,
        const Intersection &isect, const Sample *sample, RNG &rng,
        MemoryArena &arena) const {
    Spectrum L(0.);
    // Compute emitted and reflected light at ray intersection point

    // Evaluate BSDF at hit point
    BSDF *bsdf = isect.GetBSDF(ray, arena);

    // Initialize common variables for Whitted integrator
    const Point &p = bsdf->dgShading.p;
    const Normal &n = bsdf->dgShading.nn;
    Vector wo = -ray.d;

	/*
	In case the ray happened to hit geometry that is emissive(自发光) (such as an area light source),
	the integrator computes the emitted radiance by calling the Intersection::Le() method
	*/
    // Compute emitted light if ray hit an area light source
    L += isect.Le(wo);

	/*
	The integrator has computed the total contribution of direct lighting—light that arrives at
	the surface directly from emissive(自发光) objects (as opposed to(与..相反) light that has reflected off other
	objects in the scene before arriving at the point).
	*/
    // Add contribution of each light source
    for (uint32_t i = 0; i < scene->lights.size(); ++i) {
        Vector wi;
        float pdf;
        VisibilityTester visibility;
		/*
		The Light::Sample_L() method to compute the radiance
		from that light falling on the surface at the point being shaded.

		wi: the direction vector from the point being shaded to the light source
		visibility : can be used to determine if any primitives block(阻挡) the surface point from the light source

		pdf : the probability density(概率密度) for the light to have sampled the direction wi,
		This value is used for Monte Carlo integration with
		complex area light sources where light is arriving at the point from many directions even
		though just one direction is sampled here;

		*/
        Spectrum Li = scene->lights[i]->Sample_L(p, isect.rayEpsilon,
            LightSample(rng), ray.time, &wi, &pdf, &visibility);
        if (Li.IsBlack() || pdf == 0.f) continue;
        Spectrum f = bsdf->f(wo, wi);
		if (!f.IsBlack() && visibility.Unoccluded(scene))
		{
			// This product represents the light’s contribution to the light transport equation integral(积分),Equation (1.1),(P11)
			L += f * Li * AbsDot(wi, n) *
				visibility.Transmittance(scene, renderer,
				sample, rng, arena) / pdf;
		}
            
    }

	// Handles light scattered by perfectly specular surfaces like mirrors or glass.
    if (ray.depth + 1 < maxDepth) {
        // Trace rays for specular reflection and refraction
        L += SpecularReflect(ray, bsdf, rng, isect, renderer, scene, sample,
                             arena);
        L += SpecularTransmit(ray, bsdf, rng, isect, renderer, scene, sample,
                              arena);
    }
    return L;
}


WhittedIntegrator *CreateWhittedSurfaceIntegrator(const ParamSet &params)
{
    int maxDepth = params.FindOneInt("maxdepth", 5);
    return new WhittedIntegrator(maxDepth);
}


