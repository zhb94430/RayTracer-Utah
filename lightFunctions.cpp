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
    h.z = t_max;
    Ray jitteredRay = Ray(ray.p, ray.dir);
    
    
    if (Trace(jitteredRay, &rootNode, h)) {
        
        if (h.z < t_max && h.z > 0.001) {
            return 0.0;
        }
        
        return 1.0;
    }
    else {
        return 1.0;
    }
}
