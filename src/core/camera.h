
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

#ifndef PBRT_CORE_CAMERA_H
#define PBRT_CORE_CAMERA_H

// core/camera.h*
#include "pbrt.h"
#include "geometry.h"
#include "transform.h"

// The Camera object controls the viewing and lens parameters such as position,orientation,
// focus(焦点) and fided of view(视角).

// Camera Declarations
class Camera {
public:
    // Camera Interface
    Camera(const AnimatedTransform &cam2world, float sopen, float sclose,
           Film *film);
    virtual ~Camera();

	// return the Ray for a given image sample position

	/*
	computes the world space ray corresponding to a sample position on the image plane。
	*/

	/*
	This method also returns a floating-point value that gives a weight for how much light
	arriving at the film plane along the generated ray will contribute to the final image.
	*/
    virtual float GenerateRay(const CameraSample &sample,
                              Ray *ray) const = 0;

	// return the RayDifferential for a given image sample position

	/*
	not only generates this ray but also computes information about the image area that the ray
	is sampling; this information is used for anti-aliasing computations in Chapter 10
	*/

	/*
	computes a main ray like GenerateRay() but
	also computes the corresponding rays for pixels shifted one pixel in the x and y directions
	on the image plane. This information helps give the rest of the system a notion of how
	much of the image area a particular camera ray’s sample represents
	*/
    virtual float GenerateRayDifferential(const CameraSample &sample, RayDifferential *rd) const;

    // Camera Public Data
    AnimatedTransform CameraToWorld;
	/*
	Real-world cameras have a shutter(快门) that opens for a short period of time to expose(暴露) the
	film to light. One result of this nonzero exposure time is motion blur(运动模糊): objects that move
	during the film exposure time are blurred. If time values between the shutter open time
	and the shutter close time are associated with each ray, it is possible to compute images
	that exhibit motion blur. All Cameras therefore store a shutter open and shutter close time
	and are responsible for generating rays with appropriately set times.
	*/
    const float shutterOpen, shutterClose;
    Film *film;
};


class ProjectiveCamera : public Camera {
public:
    // ProjectiveCamera Public Methods
    ProjectiveCamera(const AnimatedTransform &cam2world,
        const Transform &proj, const float screenWindow[4],
        float sopen, float sclose, float lensr, float focald, Film *film);
protected:
    // ProjectiveCamera Protected Data
    Transform CameraToScreen, RasterToCamera;
    Transform ScreenToRaster, RasterToScreen;
    float lensRadius, focalDistance;
};



#endif // PBRT_CORE_CAMERA_H
