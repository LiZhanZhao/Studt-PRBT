
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

#ifndef PBRT_CORE_VOLUME_H
#define PBRT_CORE_VOLUME_H

// core/volume.h*
#include "pbrt.h"
#include "spectrum.h"
#include "geometry.h"
#include "transform.h"
#include "integrator.h"

// Volume Scattering Declarations
float PhaseIsotropic(const Vector &w, const Vector &wp);
float PhaseRayleigh(const Vector &w, const Vector &wp);
float PhaseMieHazy(const Vector &w, const Vector &wp);
float PhaseMieMurky(const Vector &w, const Vector &wp);
float PhaseHG(const Vector &w, const Vector &wp, float g);
float PhaseSchlick(const Vector &w, const Vector &wp, float g);


/*
The key abstraction for describing volume scattering in pbrt is the abstract VolumeRegion
class〞the interface to describe volume scattering in a region of the scene. Multiple
VolumeRegions of different types can be used to describe different kinds of scattering in
different parts of the scene.
*/
class VolumeRegion {
public:
    // VolumeRegion Interface
    virtual ~VolumeRegion();
    virtual BBox WorldBound() const = 0;
    virtual bool IntersectP(const Ray &ray, float *t0, float *t1) const = 0;

	/*
	This interface has four methods corresponding to the spatially varying scattering properties
	introduced earlier in this chapter. Given a world space point, direction, and time,
	VolumeRegion::sigma_a(), VolumeRegion::sigma_s(), and VolumeRegion::Lve() return
	the corresponding absorption, scattering, and emission properties. Given a pair of directions,
	the VolumeRegion::p() method returns the value of the phase function at the
	given point and the given time.
	*/
    virtual Spectrum sigma_a(const Point &, const Vector &,
                             float time) const = 0;
    virtual Spectrum sigma_s(const Point &, const Vector &,
                             float time) const = 0;
    virtual Spectrum Lve(const Point &, const Vector &,
                         float time) const = 0;
    virtual float p(const Point &, const Vector &,
                    const Vector &, float time) const = 0;

	/*
	For convenience, there is also a VolumeRegion::sigma_t() method that returns the attenuation
	coefficient at a point. A default implementation returns the sum of the 考a and
	考s values, but most of the VolumeRegion implementations will override this method and
	compute 考t directly. For VolumeRegions that need to do some amount of computation
	to find the values of 考a and 考s at a particular point, it＊s usually possible to avoid some
	duplicated work when 考t is actually needed and to compute its value directly.
	*/
    virtual Spectrum sigma_t(const Point &p, const Vector &wo, float time) const;

	/*
	Finally, the VolumeRegion::tau() method computes the volume＊s optical thickness from
	the point ray(ray.mint) to ray(ray.maxt).
	*/
    virtual Spectrum tau(const Ray &ray, float step = 1.f,
                         float offset = 0.5) const = 0;
};

/*
The rest of the volume representations in this chapter are based on the assumption
that the underlying particles throughout the medium all have the same basic scattering
properties, but their density is spatially varying in the medium. One consequence of this
assumption is that it is possible to describe the volume scattering properties at a point as
the product of the density at that point and some baseline value. For example, we might
set the attenuation coefficient 考t to have a base value of 0.2. In regions where the particle
density was 1, a 考t value of 0.2 would be returned. If the particle density were 3, however,
a 考t value of 0.6 would be the result.
*/
class DensityRegion : public VolumeRegion {
public:
    // DensityRegion Public Methods
    DensityRegion(const Spectrum &sa, const Spectrum &ss, float gg,
                  const Spectrum &emit, const Transform &VolumeToWorld)
        : sig_a(sa), sig_s(ss), le(emit), g(gg),
          WorldToVolume(Inverse(VolumeToWorld)) { }
    virtual float Density(const Point &Pobj) const = 0;
    Spectrum sigma_a(const Point &p, const Vector &, float) const {
        return Density(WorldToVolume(p)) * sig_a;
    }
    Spectrum sigma_s(const Point &p, const Vector &, float) const {
        return Density(WorldToVolume(p)) * sig_s;
    }
    Spectrum sigma_t(const Point &p, const Vector &, float) const {
        return Density(WorldToVolume(p)) * (sig_a + sig_s);
    }
    Spectrum Lve(const Point &p, const Vector &, float) const {
        return Density(WorldToVolume(p)) * le;
    }
    float p(const Point &p, const Vector &w, const Vector &wp, float) const {
        return PhaseHG(w, wp, g);
    }
    Spectrum tau(const Ray &r, float stepSize, float offset) const;
protected:
    // DensityRegion Protected Data
    Spectrum sig_a, sig_s, le;
    float g;
    Transform WorldToVolume;
};


class AggregateVolume : public VolumeRegion {
public:
    // AggregateVolume Public Methods
    AggregateVolume(const vector<VolumeRegion *> &r);
    ~AggregateVolume();
    BBox WorldBound() const;
    bool IntersectP(const Ray &ray, float *t0, float *t1) const;
    Spectrum sigma_a(const Point &, const Vector &, float) const;
    Spectrum sigma_s(const Point &, const Vector &, float) const;
    Spectrum Lve(const Point &, const Vector &, float) const;
    float p(const Point &, const Vector &, const Vector &, float) const;
    Spectrum sigma_t(const Point &, const Vector &, float) const;
    Spectrum tau(const Ray &ray, float, float) const;
private:
    // AggregateVolume Private Data
    vector<VolumeRegion *> regions;
    BBox bound;
};


bool GetVolumeScatteringProperties(const string &name, Spectrum *sigma_a,
                                   Spectrum *sigma_prime_s);

// VolumeIntegrator handle the scattering from volumetric primitives
class VolumeIntegrator : public Integrator {
public:
    // VolumeIntegrator Interface
    virtual Spectrum Li(const Scene *scene, const Renderer *renderer,
        const RayDifferential &ray, const Sample *sample, RNG &rng,
        Spectrum *transmittance, MemoryArena &arena) const = 0;
    virtual Spectrum Transmittance(const Scene *scene,
        const Renderer *renderer, const RayDifferential &ray,
        const Sample *sample, RNG &rng, MemoryArena &arena) const = 0;
};


void SubsurfaceFromDiffuse(const Spectrum &Kd, float meanPathLength, float eta,
        Spectrum *sigma_a, Spectrum *sigma_prime_s);

#endif // PBRT_CORE_VOLUME_H
