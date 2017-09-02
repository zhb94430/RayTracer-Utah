//
//  RenderFunctions.cpp
//  RayTracerXcode
//
//  Created by Peter Zhang on 8/25/17.
//  Copyright © 2017 Peter Zhang. All rights reserved.
//

#include "ExternalLibrary/scene.h"
#include "ExternalLibrary/objects.h"
#include "PixelIterator.h"

//Variables
extern Node rootNode;
extern Camera camera;
extern RenderImage renderImage;
extern MaterialList materials;
extern LightList lights;

float actualHeight, actualWidth;


//Prototypes
Point3 CalculateImageOrigin(float distanceToImg);
Point3 CalculateCurrentPoint(int i, int j, float pixelOffsetX, float pixelOffsetY, Point3 origin);
bool Trace(const Ray &r, Node* currentNode, HitInfo &hInfo);

//Main Render Function
void Render(PixelIterator& i)
{
    Point3 imgOrigin = CalculateImageOrigin(1.0);
    int x, y;
    
    while (i.GetPixelLocation(x, y)) {
        //Find current point and generate ray
        Point3 currentPoint = CalculateCurrentPoint(x, y, 0.5, 0.5, imgOrigin);
        Ray r = Ray(camera.pos, (currentPoint - camera.pos).GetNormalized());
        
        HitInfo h = HitInfo();
        
        //Trace ray with each object
        bool hitResult = Trace(r, &rootNode, h);
        
        int imgArrayIndex = x+renderImage.GetWidth()*y;
        renderImage.GetZBuffer()[imgArrayIndex] = h.z;
        
        //If hit, color white
        if (hitResult) {
            renderImage.GetPixels()[imgArrayIndex].r = 255;
            renderImage.GetPixels()[imgArrayIndex].g = 255;
            renderImage.GetPixels()[imgArrayIndex].b = 255;
            renderImage.IncrementNumRenderPixel(1);
        }
        //Else, color black
        else {
            renderImage.GetPixels()[imgArrayIndex].r = 0;
            renderImage.GetPixels()[imgArrayIndex].g = 0;
            renderImage.GetPixels()[imgArrayIndex].b = 0;
            renderImage.IncrementNumRenderPixel(1);
        }

    }
    
}

//Ray Tracing Logic
//Recurse through all child nodes,
//If an object exist, intersect ray, fill in hitinfo
//TODO-- Recursion, Model Space transformation
bool Trace(const Ray& r, Node* currentNode, HitInfo& hInfo)
{
    bool currentNodeIsHit = false;
    
    //If current node has an object, intersect and check children
    if (currentNode->GetNodeObj() != nullptr) {
        currentNodeIsHit = currentNode->GetNodeObj()->IntersectRay(currentNode->ToNodeCoords(r), hInfo);
        if (currentNodeIsHit) {
            hInfo.node = currentNode;
        }
    }
    
    //Keep testing children
    if (currentNode->GetNumChild() > 0) {
        for (int i = 0; i < currentNode->GetNumChild(); i++) {
            currentNodeIsHit = currentNodeIsHit | Trace(currentNode->ToNodeCoords(r), currentNode->GetChild(i), hInfo);
        }
    }
    
    return currentNodeIsHit;
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

