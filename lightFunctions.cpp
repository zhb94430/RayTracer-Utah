//
//  lightFunctions.cpp
//  RayTracerXcode
//
//  Created by Peter Zhang on 9/1/17.
//  Copyright © 2017 Peter Zhang. All rights reserved.
//

#include "ExternalLibrary/lights.h"
#include "ExternalLibrary/scene.h"
#include "RenderFunctions.h"

extern Node rootNode;
extern LightList lights;

float GenLight::Shadow(Ray ray, float t_max) {
    HitInfo h;
    Ray jitteredRay = Ray(ray.p + Point3(0.001,0.001,0.001), ray.dir);
    
    if (Trace(jitteredRay, &rootNode, h)) {
        
        if (h.z < t_max && h.front) {
            return 0.0;
        }
        
        return 1.0;
    }
    else {
        return 1.0;
    }
}
