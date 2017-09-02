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
            
            
            result += currentLight->Illuminate(hInfo.p, hInfo.N)*(hInfo.N.Dot(lightDirection))*(diffuse+specular*pow(hInfo.N.Dot(halfVector), glossiness));
        }
    }
    
//    if (strcmp(hInfo.node->GetName(), "sphere1") == 0) {
//        result = Color(255,0,0);
//    }
//    
//    if (strcmp(hInfo.node->GetName(), "sphere2") == 0) {
//        result = Color(0,255,0);
//    }
//    
//    if (strcmp(hInfo.node->GetName(), "sphere3") == 0) {
//        result = Color(0,0,255);
//    }
    
    return result;
}

