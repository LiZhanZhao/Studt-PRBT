
// core/camera.cpp*
#include "camera.h"
#include "film.h"
//#include "montecarlo.h"
#include "sampler.h"

// Camera Method Definitions
Camera::~Camera() {
    delete film;
}


Camera::Camera(const AnimatedTransform &cam2world,
               float sopen, float sclose, Film *f)
    : CameraToWorld(cam2world), shutterOpen(sopen), shutterClose(sclose) {
    film = f;
    if (CameraToWorld.HasScale())
        Warning("Scaling detected in world-to-camera transformation!\n"
                "The system has numerous assumptions, implicit and explicit,\n"
                "that this transform will have no scale factors in it.\n"
                "Proceed at your own risk; your image may have errors or\n"
                "the system may crash as a result of this.");
}


float Camera::GenerateRayDifferential(const CameraSample &sample,
                                      RayDifferential *rd) const {
    float wt = GenerateRay(sample, rd);
    // Find ray after shifting one pixel in the $x$ direction
    CameraSample sshift = sample;
    ++(sshift.imageX);
    Ray rx;
    float wtx = GenerateRay(sshift, &rx);
    rd->rxOrigin = rx.o;
    rd->rxDirection = rx.d;

    // Find ray after shifting one pixel in the $y$ direction
    --(sshift.imageX);
    ++(sshift.imageY);
    Ray ry;
    float wty = GenerateRay(sshift, &ry);
    rd->ryOrigin = ry.o;
    rd->ryDirection = ry.d;
    if (wtx == 0.f || wty == 0.f) return 0.f;
    rd->hasDifferentials = true;
    return wt;
}


ProjectiveCamera::ProjectiveCamera(const AnimatedTransform &cam2world,
        const Transform &proj, const float screenWindow[4], float sopen,
        float sclose, float lensr, float focald, Film *f)
    : Camera(cam2world, sopen, sclose, f) {
    // Initialize depth of field parameters
    lensRadius = lensr;
    focalDistance = focald;

    // Compute projective camera transformations
    CameraToScreen = proj;

    // Compute projective camera screen transformations
    ScreenToRaster = Scale(float(film->xResolution),
                           float(film->yResolution), 1.f) *
        Scale(1.f / (screenWindow[1] - screenWindow[0]),
              1.f / (screenWindow[2] - screenWindow[3]), 1.f) *
        Translate(Vector(-screenWindow[0], -screenWindow[3], 0.f));
    RasterToScreen = Inverse(ScreenToRaster);
    RasterToCamera = Inverse(CameraToScreen) * RasterToScreen;
}


