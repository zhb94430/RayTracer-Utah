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
    
    //Only shade front faces
    if (hInfo.front) {
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
    }
    
    
    //Reflection & Refraction
    if (bounceCount > 0) {
        
        //If a refraction property exists
        if (refraction != Color(0,0,0)) {
            //Calcluate the refracted ray direction
            float cosTheta1 = hInfo.N.Dot(-ray.dir);
            float sinTheta1 = sqrt(1-pow(cosTheta1,2));
            
            //Account for floating point precision
            if (sinTheta1 > 1) {
                sinTheta1 = 1.0;
            }
            
            if (sinTheta1 < -1) {
                sinTheta1 = -1.0;
            }
            
            if (cosTheta1 > 1) {
                cosTheta1 = 1.0;
            }
            
            if (cosTheta1 < -1) {
                cosTheta1 = -1.0;
            }
            
            //If front face hit, n2 = object ior, else n1 = object ior
            float n1 = ior;
            float n2 = 1.0;
            if (hInfo.front) {
                n1 = 1.0;
                n2 = ior;
            }
            
            float sinTheta2 = (n1/n2) * sinTheta1;
            float cosTheta2 = sqrt(1 - sinTheta2 * sinTheta2);
            
            if (cosTheta2 > 1) {
                cosTheta2 = 1.0;
            }
            
            Point3 SVector = hInfo.N.Cross(hInfo.N.Cross(-ray.dir).GetNormalized()).GetNormalized();
            
            if (sinTheta2 > 1) {
                //Total Internal Reflection
                Point3 reflectedDirection = (ray.dir - 2*ray.dir.Dot(hInfo.N)*hInfo.N).GetNormalized();
                
                Ray reflected = Ray(hInfo.p, reflectedDirection);
                HitInfo reflectedHInfo;
                
                //Calculate refraction absorption
                Color absorptionV = Color(exp((-reflectedHInfo.z)*absorption.r),
                                          exp((-reflectedHInfo.z)*absorption.g),
                                          exp((-reflectedHInfo.z)*absorption.b));
                
                if (Trace(reflected, &rootNode, reflectedHInfo)) {
                    Color TIRResult = absorptionV * reflectedHInfo.node->GetMaterial()->Shade(reflected, reflectedHInfo, lights, bounceCount-1);
                    
                    result += TIRResult;
                }
            }
            else {
                Point3 refractedDirection = (-(hInfo.N)*cosTheta2 + SVector*sinTheta2).GetNormalized();
                
                Ray refracted = Ray(hInfo.p, refractedDirection);
                HitInfo refractedHInfo;
                
                if (Trace(refracted, &rootNode, refractedHInfo)) {
                    //Fresnel Reflection
                    float R0 = pow((n1-n2)/(n1+n2), 2);
                    float ShlicksApprox = R0 + (1.0-R0)*pow((1.0-cosTheta1), 5);
                    
                    Point3 reflectedDirection = (ray.dir - 2*ray.dir.Dot(hInfo.N)*hInfo.N).GetNormalized();
                    
                    Ray reflected = Ray(hInfo.p, reflectedDirection);
                    HitInfo reflectedHInfo;
                    
                    Color frenselResult = Color(0.0, 0.0, 0.0);
                    
                    if (Trace(reflected, &rootNode, reflectedHInfo)) {
                        frenselResult = refraction * reflectedHInfo.node->GetMaterial()->Shade(reflected, reflectedHInfo, lights, bounceCount-1);
                    }
                    
                    //Refraction Result
                    Color refractionResult = refractedHInfo.node->GetMaterial()->Shade(refracted, refractedHInfo, lights, bounceCount-1);
                    
                    Color absorptionV = Color(1,1,1);
                    
                    if (!refractedHInfo.front) {
                        absorptionV = Color(exp((-refractedHInfo.z)*absorption.r),
                                            exp((-refractedHInfo.z)*absorption.g),
                                            exp((-refractedHInfo.z)*absorption.b));
                    }
                    
                    result += absorptionV * refraction * refractionResult * (1.0-ShlicksApprox) + frenselResult * ShlicksApprox;
                }
            }
        }
        
        //If a reflection property exists
        if (reflection != Color(0,0,0)) {
            //Calculate the reflected ray direction
            Point3 reflectedDirection = (ray.dir - 2*ray.dir.Dot(hInfo.N)*hInfo.N).GetNormalized();
            
            Ray reflected = Ray(hInfo.p, reflectedDirection);
            HitInfo reflectedHInfo;
            
            if (Trace(reflected, &rootNode, reflectedHInfo)) {
                result += reflection * reflectedHInfo.node->GetMaterial()->Shade(reflected, reflectedHInfo, lights, bounceCount-1);
            }
        }
    }

    return result;
}


