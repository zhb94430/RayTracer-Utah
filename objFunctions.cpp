//
//  objFunctions.cpp
//  RayTracerXcode
//
//  Created by Peter Zhang on 8/30/17.
//  Copyright © 2017 Peter Zhang. All rights reserved.
//


#include "ExternalLibrary/objects.h"
#include "ExternalLibrary/scene.h"

//Sphere intersection
bool Sphere::IntersectRay(const Ray &ray, HitInfo &hInfo, int hitSide) const
{
    float a,b,c,m,n,sqrtCheck;
    
    a = ray.dir.Dot(ray.dir);
    b = 2*((ray.p - Point3(0,0,0)).Dot(ray.dir));
    c = ray.p.Dot(ray.p) - 1;
    
    sqrtCheck = b*b-4*a*c;
    
    m = (-b+sqrt(sqrtCheck))/(2*a);
    n = (-b-sqrt(sqrtCheck))/(2*a);
    
    if (m == n && m < hInfo.z && m >= 0.001) {
            hInfo.z = m;
            hInfo.front = true;
            
            Point3 temp = ray.p + hInfo.z * ray.dir;
            
            hInfo.N = temp.GetNormalized();
            hInfo.p = temp;
            
            return true;
    }
    else if (m < n  && m < hInfo.z && (m >= 0.001 | n >= 0.001)) {
        if (m <= 0.001 && n > 0.001 && n < hInfo.z) {
            hInfo.z = n;
            hInfo.front = false;
        }
        else if (m > 0) {
            hInfo.z = m;
            hInfo.front = true;
        }
        
        Point3 temp = ray.p + hInfo.z * ray.dir;
        
        hInfo.N = temp.GetNormalized();
        hInfo.p = temp;
        
        return true;
    }
    else if (n < m && n < hInfo.z && (m >= 0.001 | n >= 0.001)) {
        if (n <= 0.001 && m > 0.001 && m < hInfo.z) {
            hInfo.z = m;
            hInfo.front = false;
        }
        else if (n > 0.001) {
            hInfo.z = n;
            hInfo.front = true;
        }
        
        Point3 temp = ray.p + hInfo.z * ray.dir;
        
        hInfo.N = temp.GetNormalized();
        hInfo.p = temp;
        
        return true;
    }
    
    return false;
}
