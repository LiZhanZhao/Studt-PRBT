
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


// renderers/samplerrenderer.cpp*
#include "stdafx.h"
#include "renderers/samplerrenderer.h"
#include "scene.h"
#include "film.h"
#include "volume.h"
#include "sampler.h"
#include "integrator.h"
#include "progressreporter.h"
#include "camera.h"
#include "intersection.h"

static uint32_t hash(char *key, uint32_t len)
{
    uint32_t hash = 0, i;
    for (hash=0, i=0; i<len; ++i) {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
} 

// SamplerRendererTask Definitions
void SamplerRendererTask::Run() {
    PBRT_STARTED_RENDERTASK(taskNum);
    // Get sub-_Sampler_ for _SamplerRendererTask_

	// GetSubSampler() method to get a new Sampler that only generates samples for the subset
	// of the image that the SamplerRenderer is responsible for
	// The GetSubSampler() method uses the task number and the total number of tasks passed to the SamplerRendererTask
	// constructor to determine which part of the image the returned subsampler should generate
	// samples for.
    Sampler *sampler = mainSampler->GetSubSampler(taskNum, taskCount);
    if (!sampler)
    {
        reporter.Update();
        PBRT_FINISHED_RENDERTASK(taskNum);
        return;
    }

    // Declare local variables used for rendering loop

	// The MemoryArena is passed to Integrators for allocating small anounts of temporary
	// memory during rendering.
    MemoryArena arena;

	// The RNG is made available to Integrators for generating psedudo-random numbers for
	// Monte Carlo Sampling
    RNG rng(taskNum);

    // Allocate space for samples and intersections

	// MaximumSampleCount  method returns an upper bound on the number of samples it will
	// return at once.(返回一个上限值)
    int maxSamples = sampler->MaximumSampleCount();
    Sample *samples = origSample->Duplicate(maxSamples);
    RayDifferential *rays = new RayDifferential[maxSamples];
    Spectrum *Ls = new Spectrum[maxSamples];
    Spectrum *Ts = new Spectrum[maxSamples];
    Intersection *isects = new Intersection[maxSamples];

    // Get samples from _Sampler_ and update image
    int sampleCount;

	// GetMoreSamples method is called to initialize the samples array(上文的samples数组) with one or more image sample values
	// the method returns the number of samples it initialized ,or zero when it has finish generating all of the samples for the
	// region of the image that is is responsible for.
    while ((sampleCount = sampler->GetMoreSamples(samples, rng)) > 0) {
        // Generate camera rays and compute radiance along rays
        for (int i = 0; i < sampleCount; ++i) {
            // Find camera ray for _sample[i]_
            PBRT_STARTED_GENERATING_CAMERA_RAY(&samples[i]);

            float rayWeight = camera->GenerateRayDifferential(samples[i], &rays[i]);

			// however, when rendering high-quality images ,many samples are often
			// averaged together to compute each pixel value, therefore, the ScaleDifferentials
			// method scales the differential ray to account for the actual spacing between 
			// samples on the film plane
            rays[i].ScaleDifferentials(1.f / sqrtf(sampler->samplesPerPixel));
            PBRT_FINISHED_GENERATING_CAMERA_RAY(&samples[i], &rays[i], rayWeight);


			// we have a ray, the next task is to determine the amount of light arriving at the
			// image plane along that ray(its radiance)

            // Evaluate radiance along camera ray
            PBRT_STARTED_CAMERA_RAY_INTEGRATION(&rays[i], &samples[i]);
            if (visualizeObjectIds) {
                if (rayWeight > 0.f && scene->Intersect(rays[i], &isects[i])) {
                    // random shading based on shape id...
                    uint32_t ids[2] = { isects[i].shapeId, isects[i].primitiveId };
                    uint32_t h = hash((char *)ids, sizeof(ids));
                    float rgb[3] = { float(h & 0xff), float((h >> 8) & 0xff),
                                     float((h >> 16) & 0xff) };
                    Ls[i] = Spectrum::FromRGB(rgb);
                    Ls[i] /= 255.f;
                }
                else
                    Ls[i] = 0.f;
            }
            else {
			// renderer->Li() method to compute the radiance along the given ray
			// Radiance values are represented here with the Spectrum class,which is pbrt's abstraction for energy
			// distributions thar vary over wavelength -- in other words, color
            if (rayWeight > 0.f)
                Ls[i] = rayWeight * renderer->Li(scene, rays[i], &samples[i], rng,
                                                 arena, &isects[i], &Ts[i]);
            else {
                Ls[i] = 0.f;
                Ts[i] = 1.f;
            }

            // Issue warning if unexpected radiance value returned
            if (Ls[i].HasNaNs()) {
                Error("Not-a-number radiance value returned "
                      "for image sample.  Setting to black.");
                Ls[i] = Spectrum(0.f);
            }
            else if (Ls[i].y() < -1e-5) {
                Error("Negative luminance value, %f, returned "
                      "for image sample.  Setting to black.", Ls[i].y());
                Ls[i] = Spectrum(0.f);
            }
            else if (isinf(Ls[i].y())) {
                Error("Infinite luminance value returned "
                      "for image sample.  Setting to black.");
                Ls[i] = Spectrum(0.f);
            }
            }
            PBRT_FINISHED_CAMERA_RAY_INTEGRATION(&rays[i], &samples[i], &Ls[i]);
        }

		// sampler->ReportResults method is used to pass the radiance values and information about
		// the intersecions found back to te sampler; this gives the sampler a chance
		// to incorporate(结合) information from the results of these samples into the 
		// samples it generates later.(for example, it could generate extra samples in pixels that have
		// a lot of detail), This method returns true if this group of samples should be added to the
		// image,or false if it shouldbe discarded.

        // Report sample results to _Sampler_, add contributions to image
        if (sampler->ReportResults(samples, rays, Ls, isects, sampleCount))
        {
            for (int i = 0; i < sampleCount; ++i)
            {
                PBRT_STARTED_ADDING_IMAGE_SAMPLE(&samples[i], &rays[i], &Ls[i], &Ts[i]);
                camera->film->AddSample(samples[i], Ls[i]);
                PBRT_FINISHED_ADDING_IMAGE_SAMPLE();
            }
        }

        // Free _MemoryArena_ memory from computing image sample values
        arena.FreeAll();
    }

    // Clean up after _SamplerRendererTask_ is done with its image region
    camera->film->UpdateDisplay(sampler->xPixelStart,
        sampler->yPixelStart, sampler->xPixelEnd+1, sampler->yPixelEnd+1);
    delete sampler;
    delete[] samples;
    delete[] rays;
    delete[] Ls;
    delete[] Ts;
    delete[] isects;
    reporter.Update();
    PBRT_FINISHED_RENDERTASK(taskNum);
}



// SamplerRenderer Method Definitions
SamplerRenderer::SamplerRenderer(Sampler *s, Camera *c,
                                 SurfaceIntegrator *si, VolumeIntegrator *vi,
                                 bool visIds) {
    sampler = s;
    camera = c;
    surfaceIntegrator = si;
    volumeIntegrator = vi;
    visualizeObjectIds = visIds;
}


SamplerRenderer::~SamplerRenderer() {
    delete sampler;
    delete camera;
    delete surfaceIntegrator;
    delete volumeIntegrator;
}


void SamplerRenderer::Render(const Scene *scene) {
    PBRT_FINISHED_PARSING();
    // Allow integrators to do preprocessing for the scene
    PBRT_STARTED_PREPROCESSING();
    surfaceIntegrator->Preprocess(scene, camera, this);
    volumeIntegrator->Preprocess(scene, camera, this);
    PBRT_FINISHED_PREPROCESSING();
    PBRT_STARTED_RENDERING();
    // Allocate and initialize _sample_
    Sample *sample = new Sample(sampler, surfaceIntegrator,
                                volumeIntegrator, scene);

    // Create and launch _SamplerRendererTask_s for rendering image

    // Compute number of _SamplerRendererTask_s to create for rendering
    int nPixels = camera->film->xResolution * camera->film->yResolution;
    int nTasks = max(32 * NumSystemCores(), nPixels / (16*16));
    nTasks = RoundUpPow2(nTasks);

    ProgressReporter reporter(nTasks, "Rendering");

	// each SamplerRendererTask is responsible for computing the samples in a small 
	// rectangular subset of the image
    vector<Task *> renderTasks;
    for (int i = 0; i < nTasks; ++i)
        renderTasks.push_back(new SamplerRendererTask(scene, this, camera,
                                                      reporter, sampler, sample, 
                                                      visualizeObjectIds, 
                                                      nTasks-1-i, nTasks));
	
	// The EnqueueTasks function takes an array of tasks and runs them on all of the
	// processors in the system.
    EnqueueTasks(renderTasks);

	// suspends(推迟) the calling thread of execution until all of the enqueued tasks have finished
    WaitForAllTasks();
    for (uint32_t i = 0; i < renderTasks.size(); ++i)
        delete renderTasks[i];
    reporter.Done();
    PBRT_FINISHED_RENDERING();
    // Clean up after rendering and store final image
    delete sample;
    camera->film->WriteImage();
}


Spectrum SamplerRenderer::Li(const Scene *scene,
        const RayDifferential &ray, const Sample *sample, RNG &rng,
        MemoryArena &arena, Intersection *isect, Spectrum *T) const {
    Assert(ray.time == sample->time);
    Assert(!ray.HasNaNs());
    // Allocate local variables for _isect_ and _T_ if needed
    Spectrum localT;
    if (!T) T = &localT;
    Intersection localIsect;
    if (!isect) isect = &localIsect;
    Spectrum Li = 0.f;

	// The Scene::Intersect() method finds the first surface that the ray intersects by passing
	// the request on to an accelerator Primitive.

	// SurfaceIntegrator::Li() to compute the outgoing radiance Lo from
	// the first surface that the ray intersects and then stores the result in Li.

	// SurfaceIntegrator::Li() then invokes VolumeIntegrator::Transmittance() to compute the fraction of light T that is
	// extinguished(熄灭) between the surface and the camera due to participating media.
    if (scene->Intersect(ray, isect))
        Li = surfaceIntegrator->Li(scene, this, ray, *isect, sample,
                                   rng, arena);
    else {

		// for rays that do not hit anything, each light’s Light::Le() method is called so that
		// these particular lights can contribute to the ray’s radiance

        // Handle ray that doesn't intersect any geometry
        for (uint32_t i = 0; i < scene->lights.size(); ++i)
           Li += scene->lights[i]->Le(ray);
    }
	
	// VolumeIntegrator::Li() determines the radiance Lv added along the ray due to interactions
	// with participating media.
    Spectrum Lvi = volumeIntegrator->Li(scene, this, ray, sample, rng,
                                        T, arena);
    return *T * Li + Lvi;
}

// It is also useful to be able to independently compute the attenuation along a ray due
// to participating media; the SamplerRenderer::Transmittance() method computes this
// quantity by forwarding the request to(将请求转发到) the VolumeIntegrator::Transmittance() method
Spectrum SamplerRenderer::Transmittance(const Scene *scene,
        const RayDifferential &ray, const Sample *sample, RNG &rng,
        MemoryArena &arena) const {
    return volumeIntegrator->Transmittance(scene, this, ray, sample,
                                           rng, arena);
}


