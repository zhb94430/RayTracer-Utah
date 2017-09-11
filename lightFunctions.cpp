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
    
    if (Trace(ray, &rootNode, h)) {
        if (h.z > 0) {
            return 0.0;
        }
    }

    return 1.0;
}
