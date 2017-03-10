
#ifndef PBRT_CORE_FILM_H
#define PBRT_CORE_FILM_H

// core/film.h*
#include "pbrt.h"

// Film Declarations
class Film {
public:
    // Film Interface
    Film(int xres, int yres)
        : xResolution(xres), yResolution(yres) { }
    virtual ~Film();
    virtual void AddSample(const CameraSample &sample,
                           const Spectrum &L) = 0;
    virtual void Splat(const CameraSample &sample, const Spectrum &L) = 0;
    virtual void GetSampleExtent(int *xstart, int *xend,
                                 int *ystart, int *yend) const = 0;
    virtual void GetPixelExtent(int *xstart, int *xend,
                                int *ystart, int *yend) const = 0;
    virtual void UpdateDisplay(int x0, int y0, int x1, int y1, float splatScale = 1.f);
    virtual void WriteImage(float splatScale = 1.f) = 0;

    // Film Public Data
    const int xResolution, yResolution;
};



#endif // PBRT_CORE_FILM_H
