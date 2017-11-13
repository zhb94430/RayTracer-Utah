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
#include "RenderFunctions.h"
#include <array>

//Variables
extern Node rootNode;
extern Camera camera;
extern RenderImage renderImage;
extern MaterialList materials;
extern LightList lights;
extern TexturedColor background;

float actualHeight, actualWidth;

//Render Parameters
const int minSampleSize = 8;
const int maxSampleSize = 128;
const float targetVariance = 0.0005;
const int sampleIncrement = 1;

//Sampling Variables
int HaltonIndex = 0;

//Prototypes
Point3 CalculateImageOrigin(float distanceToImg);
Point3 CalculateCurrentPoint(int i, int j, float pixelOffsetX, float pixelOffsetY, Point3 origin);

//Main Render Function
void Render(PixelIterator& i)
{
    Point3 imgOrigin;
    int x, y;
    
    while (i.GetPixelLocation(x, y)) {
        srand(time(NULL));
        
        if (x == 396 && y == 239) {
            int j = 0;
        }
        
        //Sample Array
        std::array<Ray, maxSampleSize> rayArray;
        std::array<HitInfo, maxSampleSize> hitInfoArray;
        std::array<bool, maxSampleSize> hitResult;
        float pixelIncrement = 1.0/minSampleSize;
        bool hitResultSum = false;
        float zSum = 0.0;
        int numOfHits = 0;
        
        //Populate the two hitinfo array
        for (int index = 0; index < minSampleSize; index++) {
            imgOrigin = CalculateImageOrigin(camera.focaldist);
            
            //Generate offset for current sample
            float currentOffset = index * pixelIncrement;
//            float offsetX = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/pixelIncrement));
//            float offsetY = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/pixelIncrement));
            float offsetX = Halton(index, 4);
            float offsetY = Halton(index, 5);
            
            //Generate random sample
            float sampleR = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/camera.dof));
            float sampleTheta = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2 * M_PI)));
            float camOffsetX = sampleR * cos(sampleTheta);
            float camOffsetY = sampleR * sin(sampleTheta);
            
            Point3 sampledPosition = camera.pos + camera.up * camOffsetY + camera.dir.GetNormalized().Cross(camera.up.GetNormalized()).GetNormalized() * camOffsetX;
            
            //Find current point and generate ray
            Point3 currentPoint = CalculateCurrentPoint(x, y, currentOffset+offsetX, currentOffset+offsetY, imgOrigin);
            rayArray[index] = Ray(sampledPosition, (currentPoint - sampledPosition).GetNormalized());
            
            //Everything is stored in World Coordinate
            hitInfoArray[index] = HitInfo();
            
            //Trace ray with each object
            bool currentResult = Trace(rayArray[index], &rootNode, hitInfoArray[index]);
            
            hitResult[index] = currentResult;
            hitResultSum |= currentResult;
            
            if (currentResult) {
                zSum += hitInfoArray[index].z;
                numOfHits++;
            }
        }

        int imgArrayIndex = x+renderImage.GetWidth()*y;
        renderImage.GetZBuffer()[imgArrayIndex] = zSum / (float)numOfHits;
        
        //If hit, shade the pixel
        if (hitResultSum) {
            Color pixelValuesSum = Color(0.0, 0.0, 0.0);
            std::array<Color, maxSampleSize> pixelValueArray;

            int currentStartIndex = 0;
            int currentSampleCount = minSampleSize;
            float currentVariance = 100.0;
            
            // Adaptive Sampling
            while (currentSampleCount < maxSampleSize && currentVariance > targetVariance) {
                // Shading Calculation
                for (int index = currentStartIndex; index < currentSampleCount; index++) {
                    Color currentResult = Color(0.0, 0.0, 0.0);
                    
                    if (hitResult[index]) {
                        currentResult = hitInfoArray[index].node->GetMaterial()->Shade(rayArray[index], hitInfoArray[index], lights, 10);
                    }
                    else {
                        currentResult = background.Sample(Point3((float)x/camera.imgWidth, (float)y/camera.imgHeight, 0));
                    }
                    
                    pixelValuesSum += currentResult;
                    pixelValueArray[index] = currentResult;
                }
                
                //Variance Calculation
                float currentMean = pixelValuesSum.Gray() / (float) currentSampleCount;
                
                float sum = 0.0;
                for (int index = 0; index < currentSampleCount; index++) {
                    sum += pow((pixelValueArray[index].Gray() - currentMean), 2);
                }
                
                currentVariance = sum / currentSampleCount;
                
                //Generate more samples
                if (currentVariance > targetVariance) {
                    currentStartIndex = currentSampleCount;
                    currentSampleCount += sampleIncrement;
                    
                    for (int index = currentStartIndex; index < currentSampleCount; index++) {
                        //Generate sample
                        float offsetX = Halton(index, 4);
                        float offsetY = Halton(index, 5);
                        
                        //Generate lens sample
                        float sampleR = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/camera.dof));
                        float sampleTheta = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2 * M_PI)));
                        float camOffsetX = sampleR * cos(sampleTheta);
                        float camOffsetY = sampleR * sin(sampleTheta);
                        
                        Point3 sampledPosition = camera.pos + camera.up * camOffsetY + camera.dir.GetNormalized().Cross(camera.up.GetNormalized()).GetNormalized() * camOffsetX;
                        
                        Point3 currentPoint = CalculateCurrentPoint(x, y, offsetX, offsetY, imgOrigin);
                        rayArray[index] = Ray(sampledPosition, (currentPoint - sampledPosition).GetNormalized());
                        
                        //Everything is stored in World Coordinate
                        hitInfoArray[index] = HitInfo();
                        
                        //Trace ray with each object
                        hitResult[index] = Trace(rayArray[index], &rootNode, hitInfoArray[index]);
                    }
                }
            }
            
            pixelValuesSum /= (float)currentSampleCount;
            
            renderImage.GetSampleCount()[imgArrayIndex] = currentSampleCount;
            renderImage.GetPixels()[imgArrayIndex] = Color24(pixelValuesSum);
            renderImage.IncrementNumRenderPixel(1);
        }
        //Else, Sample background
        else {
            Color result = background.Sample(Point3((float)x/camera.imgWidth, (float)y/camera.imgHeight, 0));

            renderImage.GetSampleCount()[imgArrayIndex] = minSampleSize;
            renderImage.GetPixels()[imgArrayIndex] = Color24(result);
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
//            const char* sresult = currentNode->GetName();
//            const char* compare = "/Users/Peter/GitRepos/RayTracer-Utah/SceneFiles/Project5/teapot-low.obj";
//            if (strcmp(sresult, compare )==0) {
//                int x = 1;
//            }
            
            hInfo.node = currentNode;
            //Convert everything back to world coord
            currentNode->FromNodeCoords(hInfo);
        }
    }
    
    //Keep testing children
    if (currentNode->GetNumChild() > 0) {
        for (int i = 0; i < currentNode->GetNumChild(); i++) {
            bool childIsHit = Trace(currentNode->ToNodeCoords(r), currentNode->GetChild(i), hInfo);
            if (childIsHit) {
                currentNode->FromNodeCoords(hInfo);
            }
            currentNodeIsHit = currentNodeIsHit | childIsHit;
        }
    }
    
    return currentNodeIsHit;
}

//Shadow Trace function
bool ShadowTrace(const Ray& r, Node* currentNode, HitInfo& hInfo)
{
    bool currentNodeIsHit = false;
    
    //If current node has an object, intersect and check children
    if (currentNode->GetNodeObj() != nullptr) {
        currentNodeIsHit = currentNode->GetNodeObj()->IntersectRay(currentNode->ToNodeCoords(r), hInfo);
        if (currentNodeIsHit) {
            return true;
        }
    }
    
    //Keep testing children
    if (currentNode->GetNumChild() > 0) {
        for (int i = 0; i < currentNode->GetNumChild(); i++) {
            bool childIsHit = ShadowTrace(currentNode->ToNodeCoords(r), currentNode->GetChild(i), hInfo);
            if (childIsHit) {
                return true;
            }
            currentNodeIsHit = currentNodeIsHit | childIsHit;
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

//Sample a sphere
Point3 SampleSphereHalton(Point3 origin, float radius)
{
    float rand1 = Halton(HaltonIndex, 4) * radius;
    HaltonIndex++;
    float rand2 = Halton(HaltonIndex, 5) * radius;
    HaltonIndex++;
    float rand3 = Halton(HaltonIndex, 6) * radius;
    HaltonIndex++;
    
//    float rand1 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/radius));
//    float rand2 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/radius));
//    float rand3 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/radius));

    Point3 offset = Point3(rand1,rand2,rand3);
    
    if (offset.Length() > radius) {
        offset = SampleSphereHalton(origin, radius);
    }
    
    return offset;
}

//Sample a circle

