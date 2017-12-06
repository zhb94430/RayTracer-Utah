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

int shadowSampleMax = 1;
int shadowSampleMin = 1;

Ray PointLight::RandomPhoton() const {
    Point3 dir = SampleSphere(position, 1.0);
    
    Ray result = Ray(position, dir);
    
    return result;
}

float GenLight::Shadow(Ray ray, float t_max) {
    HitInfo h;
    h.z = t_max;
    
    if (ShadowTrace(ray, &rootNode, h)) {
        if (h.z > 0.0) {
            return 0.0;
        }
    }
    return 1.0;
}

Color PointLight::Illuminate(const Point3 &p, const Point3 &N) const {
    float shadowIntensity = 0.0;
    float result = 1.0;

    if (size > 0) {
        // Perform minimum shadow sample
//        for (int i = 0; i < shadowSampleMax; i++) {
            // Generate random sample
            float sampleR = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/size));
            float sampleTheta = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2 * M_PI)));
            float offsetX = sampleR * cos(sampleTheta);
            float offsetY = sampleR * sin(sampleTheta);
            
            Point3 samplePlaneNormal = (position - p).GetNormalized();
            
            // Find two random vectors perpendicular to N to construct coord sys
            Point3 v1 = samplePlaneNormal.Cross(Point3(0,0,1)).GetNormalized();
            Point3 v2 = v1.Cross(samplePlaneNormal).GetNormalized();
            
            Point3 currentSamplePos = position + v1*offsetX + v2*offsetY;
            
            Ray shadowRay = Ray(p, (currentSamplePos - p).GetNormalized());
            
            shadowIntensity += Shadow(shadowRay, (p-currentSamplePos).Length());
        
//        One Shadow ray per sample
        result = shadowIntensity;
        
//            if (i == shadowSampleMin) {
//                if (shadowIntensity == shadowSampleMin*1.0) {
//                    return intensity;
//                }
//            }
//        }
//        result = shadowIntensity/(float)shadowSampleMax;
    }
    else {
        Ray shadowRay = Ray(p, (position - p).GetNormalized());
        
        shadowIntensity += Shadow(shadowRay, (position-p).Length());
        
        result = shadowIntensity;
    }
    
    return result * intensity * (1/(position - p).LengthSquared());
}
