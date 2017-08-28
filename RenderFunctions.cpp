//
//  RenderFunctions.cpp
//  RayTracerXcode
//
//  Created by Peter Zhang on 8/25/17.
//  Copyright © 2017 Peter Zhang. All rights reserved.
//

#include "ExternalLibrary/scene.h"
#include "ExternalLibrary/objects.h"

//Variables
extern Node rootNode;
extern Camera camera;
extern RenderImage renderImage;
float actualHeight, actualWidth;

//Prototypes
Point3 CalculateImageOrigin(float distanceToImg);
Point3 CalculateCurrentPoint(int i, int j, float pixelOffsetX, float pixelOffsetY, Point3 origin);
void Trace(Ray &r, Node* currentNode, HitInfo &hInfo);

//Main Render Function
void Render()
{
    Point3 imgOrigin = CalculateImageOrigin(1.0);
    
    //For each pixel in img, generate ray from C
    //TODO -- MULTITHREADING
    for (int i = 0; i < renderImage.GetWidth(); i++) {
        for (int j = 0; j < renderImage.GetHeight(); j++) {
            
            //Find current point and generate ray
            Point3 currentPoint = CalculateCurrentPoint(i, j, 0.5, 0.5, imgOrigin);
            Ray r = Ray(camera.pos, (currentPoint - camera.pos).GetNormalized());
            
            HitInfo h = HitInfo();
            
            //Trace ray with each object
            Trace(r, &rootNode, h);
            
            //If hit, color white
            if (h.z > 0 && h.z < BIGFLOAT) {
                renderImage.GetPixels()[i+renderImage.GetWidth()*j].r = 255;
                renderImage.GetPixels()[i+renderImage.GetWidth()*j].g = 255;
                renderImage.GetPixels()[i+renderImage.GetWidth()*j].b = 255;
                renderImage.IncrementNumRenderPixel(1);
            }
            //Else, color black
            else {
                renderImage.GetPixels()[i+renderImage.GetWidth()*j].r = 100;
                renderImage.GetPixels()[i+renderImage.GetWidth()*j].g = 0;
                renderImage.GetPixels()[i+renderImage.GetWidth()*j].b = 0;
                renderImage.IncrementNumRenderPixel(1);
            }
            
        }
    }
}

//Ray Tracing Logic
//Recurse through all child nodes,
//If an object exist, intersect ray, fill in hitinfo
//TODO-- Recursion, Model Space transformation
void Trace(Ray& r, Node* currentNode, HitInfo& hInfo)
{
    //If current node has an object, intersect and check children
    if (currentNode->GetNodeObj() != nullptr) {
        //Save current hInfo value
        float previousZ = hInfo.z;
        
        currentNode->GetNodeObj()->IntersectRay(currentNode->ToNodeCoords(r), hInfo);
        
        float newZ = (hInfo.z*r.dir).Length();
        
        //If new distance is smaller than previous one, use the new distance
        if (newZ < previousZ) {
            hInfo.z = newZ;
            hInfo.node = currentNode;
        }
    }
    
    //Reached leaf
    if (currentNode->GetNumChild() <= 0) {
        return;
    }
    
    //Keep testing children
    else {
        for (int i = 0; i < currentNode->GetNumChild(); i++) {
            Trace(r, currentNode->GetChild(i), hInfo);
        }
    }
    
    return;
}

//Sphere intersection
bool Sphere::IntersectRay(const Ray &ray, HitInfo &hInfo, int hitSide) const
{
    float a,b,c,m,n,sqrtCheck;
    
    a = ray.dir.Dot(ray.dir);
    b = 2*((ray.p - Point3(0,0,0)).Dot(ray.dir));
    c = ray.p.Dot(ray.p) - 1;
    
    sqrtCheck = b*b-4*a*c;
    
//    //No Intersection
//    if (sqrtCheck < 0) {
//        return false;
//    }
//    else if (sqrtCheck == 0) {
//        hInfo.z = -b/(2*a);
//        hInfo.front = true;
//        
//        return true;
//    }
//    else {
//        m = (-b+sqrt(sqrtCheck))/(2*a);
//        n = (-b-sqrt(sqrtCheck))/(2*a);
//        
//        if (m < n) {
//            hInfo.z = m;
//            hInfo.front = true;
//        }
//        else {
//            hInfo.z = n;
//            hInfo.front = true;
//        }
//        
//        return true;
//    }
    
    m = (-b+sqrt(sqrtCheck))/(2*a);
    n = (-b-sqrt(sqrtCheck))/(2*a);
    
    if (m == n) {
        hInfo.z = m;
        hInfo.front = true;
        return true;
    }
    else if (m < n) {
        if (m <= 0) {
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
        if (n <= 0) {
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


//Calculate the origin of the image in world space
Point3 CalculateImageOrigin(float distanceToImg)
{
    Point3 result, topCenterPoint;

    actualHeight = tan((camera.fov/2)*M_PI/180.0)*2*distanceToImg;
    actualWidth = ((float)camera.imgWidth/(float)camera.imgHeight) * actualHeight;
    
    topCenterPoint = camera.pos + distanceToImg * camera.dir.GetNormalized() + (actualHeight/2)*camera.up.GetNormalized();
    
    result = topCenterPoint - (actualWidth/2)*(camera.dir.GetNormalized().Cross(camera.up.GetNormalized()).GetNormalized());
    
    return result;
}

//Calculate the coord of the current point in world space
Point3 CalculateCurrentPoint(int i, int j, float pixelOffsetX, float pixelOffsetY, Point3 origin)
{
    //U, V are verctors in direction of x and y at the length of one unit pixel
    Point3 result, u, v;
    
    u = (camera.dir.GetNormalized().Cross(camera.up.GetNormalized())).GetNormalized()*(actualWidth/(float)camera.imgWidth);
    v = (-1*(camera.up.GetNormalized()))*(actualHeight/(float)camera.imgHeight);
    
    result = origin + (i + pixelOffsetX) * u + (j + pixelOffsetY) * v;
    
    return result;
}
