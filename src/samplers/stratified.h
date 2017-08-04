
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

#ifndef PBRT_SAMPLERS_STRATIFIED_H
#define PBRT_SAMPLERS_STRATIFIED_H

// samplers/stratified.h*
#include "sampler.h"
#include "film.h"

/*
The first sample generator that we will introduce divides the image plane into rectangular
regions and generates a single sample inside each region. These regions are commonly
called strata(层).

The key idea behind
stratification(层化) is that by subdividing(细分) the sampling domain into nonoverlapping regions
and taking a single sample from each one, we are less likely to(不太可能) miss important features
of the image entirely, since the samples are guaranteed(保证) not to all be close together. Put
another way(另一种方式), it does us no good(这对我们没有好处) if many samples are taken from nearby points in the
sample space, since each new sample doesn’t add much new information about the
behavior of the image function

The stratified sampler places each sample at a random point inside each stratum by
jittering the center point of the stratum by a random amount up to(直到) half the stratum’s
width and height. The nonuniformity(不平均) that results from this jittering helps turn aliasing
into noise,

The sampler also offers a nonjittered mode, which
gives uniform(均匀) sampling in the strata; this mode is mostly useful for comparisons between
different sampling techniques rather than for rendering final images

P346

*/

// StratifiedSampler Declarations
class StratifiedSampler : public Sampler {
public:

	/*
	(确定image的哪一些区域的像素需要取样，而且是每一个像素有xs * ys 个 strata(层))

	The constructor takes the range of pixels to generate samples
	for—[xstart,ystart] to [xend-1,yend-1], inclusive—the number of strata in x and y
	(xs and ys), a Boolean (jitter) that indicates whether the samples should be jittered,
	and the range of time values where the camera shutter is open, [sopen, sclose].
	*/
    // StratifiedSampler Public Methods
    StratifiedSampler(int xstart, int xend, int ystart, int yend,
                      int xs, int ys, bool jitter, float sopen, float sclose);
    ~StratifiedSampler();
    int RoundSize(int size) const { return size; }

	/*
	The GetSubSampler() method uses the ComputeSubwindow() utility routine to compute an
	image tile extent for the given subsampler number and then returns a new instance of a
	StratifiedSampler that generates samples in just that region.
	*/
    Sampler *GetSubSampler(int num, int count);

	/*
	P350

	we might
	want to take just four samples per pixel, but still have the samples be stratified over all
	dimensions.We independently generate four 2D stratified image samples, four 1D stratified
	time samples, and four 2D stratified lens samples. Then, we randomly associate a
	time and lens sample value with each image sample. The result is that each pixel has
	samples that together have good coverage of the sample space.
	*/
    int GetMoreSamples(Sample *sample, RNG &rng);

    int MaximumSampleCount() { return xPixelSamples * yPixelSamples; }
private:
    // StratifiedSampler Private Data
    int xPixelSamples, yPixelSamples;
    bool jitterSamples;
    int xPos, yPos;
    float *sampleBuf;
};


StratifiedSampler *CreateStratifiedSampler(const ParamSet &params, const Film *film,
    const Camera *camera);

#endif // PBRT_SAMPLERS_STRATIFIED_H
