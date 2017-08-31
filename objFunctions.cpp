//
//  objFunctions.cpp
//  RayTracerXcode
//
//  Created by Peter Zhang on 8/30/17.
//  Copyright © 2017 Peter Zhang. All rights reserved.
//

#ifndef _OBJ_FUNC_INCLUDED_
#define _OBJ_FUNC_INCLUDED_

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
    
    if (m == n) {
        hInfo.z = m;
        hInfo.front = true;
        return true;
    }
    else if (m < n) {
        if (m <= 0 && n > 0) {
            hInfo.z = n;
            hInfo.front = false;
            return true;
        }
        else if (m > 0) {
            hInfo.z = m;
            hInfo.front = true;
            return true;
        }
    }
    else if (n < m) {
        if (n <= 0 && m > 0) {
            hInfo.z = m;
            hInfo.front = false;
            return true;
        }
        else if (n > 0) {
            hInfo.z = n;
            hInfo.front = true;
            return true;
        }
    }
    
    return false;
}

#endif
