
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

#ifndef PBRT_CORE_SAMPLER_H
#define PBRT_CORE_SAMPLER_H

// core/sampler.h*
#include "pbrt.h"
#include "geometry.h"
#include "rng.h"
#include "memory.h"

// The Sampler is responsible for choosing the points on the image plane from which ray 
// are traced,and it is responsible for supplying the sample position used by the integrators
// in their light transport computations.

/*
The task of Samplers is to generate a sequence of multidimensional sample
positions. Two dimensions give the raster space image sample position, and another gives
the time at which the sample should be taken; this ranges from zero to one, and is scaled
by the camera to cover the time period that the shutter is open. Two more sample values
give a (u, v) lens position for depth of field; these also vary from zero to one.

Just as well-placed sample points can help conquer(克服) the complexity of the 2D image
function, most of the light transport algorithms in Chapters 15 and 16 use sample points
for tasks like choosing positions on area light sources when estimating(估计) illumination(照光).
Choosing these points is also the job of the Sampler, since it is able to take the sample
points chosen for adjacent image samples into account when selecting samples at new
points. Doing so can improve the quality of the results of the light transport algorithms.

*/

// Sampling Declarations
class Sampler {
public:
    // Sampler Interface
    virtual ~Sampler();
	/*
	All of the Sampler implementations take a few common parameters that must be passed
	to the base class’s constructor. These include the overall image resolution in the x and
	y dimensions, the number of samples the implementation expects to generate for each
	pixel in the final image, and the range of time over which the camera’s shutter is open.
	These values are stored in member variables for later use.
	*/
    Sampler(int xstart, int xend, int ystart, int yend,
            int spp, float sopen, float sclose);

	/*
	The SamplerRendererTask::Run() method calls this function until it
	returns zero, which signifies that all samples have been generated. Otherwise, it generates
	one or more samples, returning the number of samples generated and filling
	in sample values in the array pointed to by the sample parameter. All of the dimensions
	of the sample values it generates should have values in the range [0, 1], except for
	CameraSample::imageX and CameraSample::imageY, which are specified with respect to
	the image size in raster coordinates.
	*/
    virtual int GetMoreSamples(Sample *sample, RNG &rng) = 0;

	/*
	The MaximumSampleCount() method returns the maximum number of sample values
	that the Sampler will ever return from its implementation of the GetMoreSamples()
	method. It allows the renderer to preallocate memory for the array of Samples passed
	into GetMoreSamples().
	*/
    virtual int MaximumSampleCount() = 0;

	/*
	Samplers may implement the ReportResults() method; it allows the renderer to report
	back to the sampler which rays were generated, what radiance values were computed,
	and the intersection points found for a collection of samples originally from
	GetMoreSamples(). The sampler may use this information for adaptive(自适应) sampling algorithms,
	deciding to take more samples close to the ones that were returned here.

	The return value indicates whether or not the sample values should be added to the image
	being generated. For some adaptive sampling algorithms, the sampler may want to cause
	an initial collection of samples (and their results) to be discarded, generating a new set to
	replace them completely. Because most of the Samplers in this chapter do not implement
	adaptive sampling, a default implementation of this method just returns true.
	*/
    virtual bool ReportResults(Sample *samples, const RayDifferential *rays,
        const Spectrum *Ls, const Intersection *isects, int count);

	/*
	The GetSubSampler() method returns a new Sampler that is responsible for
	generating samples for a subset of the image; its num parameter ranges from 0 to count-1,
	where count is the total number of subsamplers being used. Implementations of this
	method must ensure that the aggregate(聚合) set of generated samples by all of the subsamplers
	covers the image; how they decompose(分解) the image is not defined.
	*/
    virtual Sampler *GetSubSampler(int num, int count) = 0;

	/*
	Most Samplers can do a better job of generating particular quantities of these additional
	samples than others. For example, the LDSampler can generate extremely good patterns,
	although theymust have a size that is a power of two. The Sampler::RoundSize() method
	helps communicate this information. Integrators should call this method with the desired
	number of samples to be taken, giving the Sampler an opportunity to adjust the
	number of samples to a more convenient one. The integrator should then use the returned
	value as the number of samples to request from the Sampler.
	*/
    virtual int RoundSize(int size) const = 0;

    // Sampler Public Data
    const int xPixelStart, xPixelEnd, yPixelStart, yPixelEnd;
    const int samplesPerPixel;
    const float shutterOpen, shutterClose;
protected:

	/*
	Most implementations of the GetSubSampler() method decompose(分解) the image into rectangular
	tiles and have each subsampler generate samples for a single tile. Compute
	SubWindow() is a utility function that computes a pixel sampling range given a tile number
	num and a total number of tiles count.

	简单理解就是: 计算某一个tile对应的 pixel 区域
	*/

    // Sampler Protected Methods
    void ComputeSubWindow(int num, int count, int *xstart, int *xend, int *ystart, int *yend) const;
};

/*
the CameraSample represents just the
sample values that are needed for generating camera rays. This separation allows us to
just pass a CameraSample to the Camera::GenerateRay() method, not exposing all of the
details of the rest of the Sample structure to Cameras.
*/
struct CameraSample {
    float imageX, imageY;
    float lensU, lensV;
    float time;
};

/*
The Sample structure is used by Samplers to store a single sample. After one or more
Samples is initialized by a call to the Sampler’s GetMoreSamples() method, the Sampler
RendererTask::Run() method passes each of the Samples to the camera and integrators,
which read values from it to construct the camera ray and perform lighting calculations.
*/
struct Sample : public CameraSample {
    // Sample Public Methods
    Sample(Sampler *sampler, SurfaceIntegrator *surf, VolumeIntegrator *vol,
        const Scene *scene);

	/*
	The implementations of the Sample::Add1D() and Sample::Add2D() methods record the
	number of samples asked for in an array and return an index that the integrator can later
	use to access the desired sample values in the Sample.
	*/
    uint32_t Add1D(uint32_t num) {
        n1D.push_back(num);
        return n1D.size()-1;
    }
    uint32_t Add2D(uint32_t num) {
        n2D.push_back(num);
        return n2D.size()-1;
    }
    ~Sample() {
        if (oneD != NULL) {
            FreeAligned(oneD[0]);
            FreeAligned(oneD);
        }
    }
    Sample *Duplicate(int count) const;

	/*
	n1D,n2D只是存储个数，oneD，twoD存储的是数值

	For 1D sample patterns, it needs to generate
	n1D.size() independent patterns, where the ith pattern has n1D[i] sample values. These
	values are stored in oneD[i][0] through oneD[i][n1D[i]-1].
	*/
    // Sample Public Data
    vector<uint32_t> n1D, n2D;
    float **oneD, **twoD;
private:
    // Sample Private Methods
    void AllocateSampleMemory();
    Sample() { oneD = twoD = NULL; }
};



#endif // PBRT_CORE_SAMPLER_H
