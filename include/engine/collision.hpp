#pragma once

#include "object.hpp"

namespace engine {
    class Ray {
        public:
        vec3 origin;
        vec3 direction;
    };


    class Collision {
    public:
        static bool rayCast(Ray ray, vector<Object>) {
            
        };

    };
}
