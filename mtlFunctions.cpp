//
//  mtlFunctions.cpp
//  RayTracerXcode
//
//  Created by Peter Zhang on 8/30/17.
//  Copyright © 2017 Peter Zhang. All rights reserved.
//


#include "ExternalLibrary/materials.h"
#include "ExternalLibrary/scene.h"
#include <math.h>

extern Camera camera;

Color MtlBlinn::Shade(const Ray &ray, const HitInfo &hInfo, const LightList &lights) const
{
    Color result;
    
    //Iterate through each light
    for (int i = 0; i < lights.size(); i++) {
        Light* currentLight = lights[i];
        
        //Ambient Light
        if (currentLight->IsAmbient()) {
            result += diffuse*currentLight->Illuminate(hInfo.p, hInfo.N);
        }
        
        //Shading Happens in World Space
        else {
            Point3 viewDirection = (camera.pos - hInfo.p).GetNormalized();
            Point3 halfVector = (viewDirection+currentLight->Direction(hInfo.p)).GetNormalized();
            
            result += currentLight->Illuminate(hInfo.p, hInfo.N)*(hInfo.N.Dot(currentLight->Direction(hInfo.p)))*(diffuse+specular*pow(hInfo.N.Dot(halfVector), glossiness));
        }
    }
    
    return result;
}

