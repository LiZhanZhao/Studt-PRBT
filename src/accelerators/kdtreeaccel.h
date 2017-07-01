
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

#ifndef PBRT_ACCELERATORS_KDTREEACCEL_H
#define PBRT_ACCELERATORS_KDTREEACCEL_H

// accelerators/kdtreeaccel.h*
#include "pbrt.h"
#include "primitive.h"

// KdTreeAccel Declarations
struct KdAccelNode;
struct BoundEdge;

/*
KdTreeAccel) based on adaptive(����Ӧ) recursive(�ݹ�) spatial subdivision(�ռ�ϸ��).

Binary space partitioning (BSP) trees adaptively(����Ӧ) subdivide space into irregularly(������) sized
regions. The most important consequence(����) of this difference with regular grids is that they
can be a much more effective data structure for storing irregularly distributed collections
of geometry. A BSP tree starts with a bounding box that encompasses(��Χ) the entire scene.
If the number of primitives in the box is greater than some threshold(��ֵ), the box is split
in half by a plane. Primitives are then associated with whichever half they overlap and
primitives that lie in both halves are associated with both of them.

The splitting process continues recursively either until each leaf region in the resulting
tree contains a sufficiently small number of primitives or until a maximum depth is
reached. Because the splitting planes can be placed at arbitrary positions inside the overall
bound and because different parts of 3D space can be refined to different degrees, BSP
trees can easily handle uneven(������) distributions of geometry.

Two variations of BSP trees are kd-trees and octrees. A kd-tree simply restricts the splitting
plane to be perpendicular to one of the coordinate axes; this makes both traversal and
construction of the tree more efficient, at the cost of some flexibility in how space is
subdivided.

*/
class KdTreeAccel : public Aggregate {
public:
    // KdTreeAccel Public Methods
    KdTreeAccel(const vector<Reference<Primitive> > &p,
                int icost = 80, int scost = 1,  float ebonus = 0.5f, int maxp = 1,
                int maxDepth = -1);
    BBox WorldBound() const { return bounds; }
    bool CanIntersect() const { return true; }
    ~KdTreeAccel();
    bool Intersect(const Ray &ray, Intersection *isect) const;
    bool IntersectP(const Ray &ray) const;
private:
    // KdTreeAccel Private Methods
    void buildTree(int nodeNum, const BBox &bounds,
        const vector<BBox> &primBounds, uint32_t *primNums, int nprims, int depth,
        BoundEdge *edges[3], uint32_t *prims0, uint32_t *prims1, int badRefines = 0);

    // KdTreeAccel Private Data
    int isectCost, traversalCost, maxPrims, maxDepth;
    float emptyBonus;
    vector<Reference<Primitive> > primitives;
    KdAccelNode *nodes;
    int nAllocedNodes, nextFreeNode;
    BBox bounds;
    MemoryArena arena;
};


struct KdToDo {
    const KdAccelNode *node;
    float tmin, tmax;
};


KdTreeAccel *CreateKdTreeAccelerator(const vector<Reference<Primitive> > &prims,
        const ParamSet &ps);

#endif // PBRT_ACCELERATORS_KDTREEACCEL_H
