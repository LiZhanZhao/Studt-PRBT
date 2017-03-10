

#ifndef PBRT_CORE_CAMERA_H
#define PBRT_CORE_CAMERA_H

// core/camera.h*
#include "pbrt.h"
#include "geometry.h"
#include "transform.h"
#include "film.h"

// Camera Declarations
class Camera {
public:
    // Camera Interface
    Camera(const AnimatedTransform &cam2world, float sopen, float sclose,
           Film *film);
    virtual ~Camera();
    virtual float GenerateRay(const CameraSample &sample,
                              Ray *ray) const = 0;
    virtual float GenerateRayDifferential(const CameraSample &sample, RayDifferential *rd) const;

    // Camera Public Data
    AnimatedTransform CameraToWorld;
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
