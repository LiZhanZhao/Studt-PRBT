
// core/primitive.cpp*

#include "primitive.h"
#include "intersection.h"

// Primitive Method Definitions
uint32_t Primitive::nextprimitiveId = 1;
Primitive::~Primitive() { }

bool Primitive::CanIntersect() const {
    return true;
}



void Primitive::Refine(vector<Reference<Primitive> > &refined) const {
    Severe("Unimplemented Primitive::Refine() method called!");
}


void
Primitive::FullyRefine(vector<Reference<Primitive> > &refined) const {
    vector<Reference<Primitive> > todo;
    todo.push_back(const_cast<Primitive *>(this));
    while (todo.size()) {
        // Refine last primitive in todo list
        Reference<Primitive> prim = todo.back();
        todo.pop_back();
        if (prim->CanIntersect())
            refined.push_back(prim);
        else
            prim->Refine(todo);
    }
}

