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

Color MtlBlinn::Shade(const Ray &ray, const HitInfo &hInfo, const LightList &lights, int bounceCount) const
{
    Color result = Color(0,0,0);
    
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
            Point3 lightDirection = (-(currentLight->Direction(hInfo.p))).GetNormalized();
            Point3 halfVector = (viewDirection+lightDirection).GetNormalized();
            
            float NDotL = hInfo.N.Dot(lightDirection);
            float NDotH = hInfo.N.Dot(halfVector);
            
            if (NDotL < 0.0) {
                NDotL = 0.0;
            }
            
            if (NDotH < 0.0) {
                NDotH = 0.0;
            }
            
            result += currentLight->Illuminate(hInfo.p, hInfo.N)*NDotL*(diffuse+specular*pow(NDotH, glossiness));
        }
    }
    
    return result;
}

