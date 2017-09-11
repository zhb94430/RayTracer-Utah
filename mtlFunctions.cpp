//
//  mtlFunctions.cpp
//  RayTracerXcode
//
//  Created by Peter Zhang on 8/30/17.
//  Copyright © 2017 Peter Zhang. All rights reserved.
//


#include "ExternalLibrary/materials.h"
#include "ExternalLibrary/scene.h"
#include "RenderFunctions.h"
#include <math.h>

extern Camera camera;
extern Node rootNode;

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
    
    if (bounceCount > 0) {
        //If a reflection property exists
        if (reflection != Color(0,0,0)) {
            //Calculate the reflected ray direction
            Point3 reflectedDirection = (ray.dir - 2*ray.dir.Dot(hInfo.N)*hInfo.N).GetNormalized();
            
            Ray reflected = Ray(hInfo.p, reflectedDirection);
            HitInfo reflectedHInfo;
            
            if (Trace(reflected, &rootNode, reflectedHInfo)) {
                result += reflectedHInfo.node->GetMaterial()->Shade(reflected, reflectedHInfo, lights, bounceCount-1);
            }
        }
        
        //If a refraction property exists
        if (refraction != Color(0,0,0)) {
            //Calcluate the refracted ray direction
            float c1 = hInfo.N.Dot(ray.dir);
            float c2 = sqrt((1.0-ior*ior*(1-c1*c1)));
            
            Point3 refractedDirection = (ior*ray.dir + (ior*c1 - c2)*hInfo.N).GetNormalized();
            
            Ray refracted = Ray(hInfo.p, refractedDirection);
            HitInfo refractedHInfo;
            
            if (Trace(refracted, &rootNode, refractedHInfo)) {
                result += refractedHInfo.node->GetMaterial()->Shade(refracted, refractedHInfo, lights, bounceCount-1);
            }
        }
        
        //If an absorption property exists
        if (absorption != Color(0,0,0)) {
            result -= absorption;
        }
    }
    
    return result;
}


