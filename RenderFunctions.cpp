//
//  RenderFunctions.cpp
//  RayTracerXcode
//
//  Created by Peter Zhang on 8/25/17.
//  Copyright © 2017 Peter Zhang. All rights reserved.
//

#ifndef _RENDERF_INCLUDED_
#define _RENDERF_INCLUDED_

#include "ExternalLibrary/scene.h"
#include "ExternalLibrary/objects.h"

//Variables
extern Node rootNode;
extern Camera camera;
extern RenderImage renderImage;

//Prototypes
Point3 CalculateImageOrigin(float distanceToImg);
Point3 CalculateCurrentPoint(int i, int j, int pixelOffsetX, int pixelOffsetY, Point3 origin);
HitInfo Trace(Ray r, Node* currentNode);

//Main Render Function
void Render()
{
    Point3 imgOrigin = CalculateImageOrigin(1.0);
    
    //For each pixel in img, generate ray from C
    for (int i = 0; i < renderImage.GetWidth(); i++) {
        for (int j = 0; j < renderImage.GetHeight(); j++) {
            
            //Find current point and generate ray
            Point3 currentPoint = CalculateCurrentPoint(i, j, 0.5, 0.5, imgOrigin);
            Ray r = Ray(imgOrigin, (currentPoint - camera.pos).GetNormalized());
            
            //Trace ray with each object
            //If hit, color white
            if (Trace(r, &rootNode).node != nullptr) {
                renderImage.GetPixels()[(i+1)*(j+1)].r = 255;
                renderImage.GetPixels()[(i+1)*(j+1)].g = 255;
                renderImage.GetPixels()[(i+1)*(j+1)].b = 255;
            }
            //Else, color black
            else {
                renderImage.GetPixels()[(i+1)*(j+1)].r = 0;
                renderImage.GetPixels()[(i+1)*(j+1)].g = 0;
                renderImage.GetPixels()[(i+1)*(j+1)].b = 0;
            }
        }
    }
}

//Ray Tracing Logic

//TODO-- Recursion, Model Space transformation
HitInfo Trace(Ray r, Node* currentNode)
{
    HitInfo h;
    
    //If not at leaf, recurse
    if (currentNode->GetNumChild() > 0) {
        for (int i = 0; i < currentNode->GetNumChild(); i++) {
            
        }
    }
    else {
        currentNode->GetNodeObj()->IntersectRay(r, h);
    }
    
    
    return h;
}

//Sphere intersection
bool Sphere::IntersectRay(const Ray &ray, HitInfo &hInfo, int hitSide) const
{
    float a,b,c,m,n,sqrtCheck;
    
    a = ray.dir.Dot(ray.dir);
    b = 2*(ray.p - Point3(0,0,0)).Dot(ray.dir);
    c = (ray.p - Point3(0,0,0)).Dot(ray.p - Point3(0,0,0)) - 1;
    
    sqrtCheck = b*b-4*a*c;
    
    //No Intersection
    if (sqrtCheck < 0) {
        return false;
    }
    else if (sqrtCheck == 0) {
        hInfo.z = (-b)/(2*a);
        hInfo.front = true;
        return true;
    }
    else {
        m = (-b+sqrtCheck)/(2*a);
        n = (-b-sqrtCheck)/(2*a);
        
        if (m < n) {
            hInfo.z = m;
        }
        else {
            hInfo.z = n;
        }
        hInfo.front = true;
        return true;
    }
    
    return false;
}


//Calculate the origin of the image in world space
Point3 CalculateImageOrigin(float distanceToImg)
{
    Point3 result, topCenterPoint;
    
    topCenterPoint = camera.pos + distanceToImg * camera.dir + (camera.imgHeight/2)*camera.up.GetNormalized();
    
    result = topCenterPoint + (camera.imgWidth/2)*(-1*(camera.dir.Cross(camera.up)).GetNormalized());
    
    return result;
}

//Calculate the coord of the current point in world space
Point3 CalculateCurrentPoint(int i, int j, int pixelOffsetX, int pixelOffsetY, Point3 origin)
{
    //U, V are verctors in direction of x and y at the length of one unit pixel
    Point3 result, u, v;
    
    u = (camera.dir.Cross(camera.up)).GetNormalized()*(camera.imgWidth/renderImage.GetWidth());
    v = -1*camera.up*(camera.imgHeight/renderImage.GetHeight());
    
    result = origin + (i + pixelOffsetX) * u + (j + pixelOffsetY) * v;
    
    return result;
}

#endif
