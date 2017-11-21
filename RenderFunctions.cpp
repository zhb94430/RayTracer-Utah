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
const int maxSampleSize = 1024;
const float targetVariance = 0.005;
const int sampleIncrement = 1;
const int monteCarloSampleSize = 1;
const int monteCarloBounces = 4;

//Sampling Variables
int HaltonIndex = 0;

//Prototypes
Point3 CalculateImageOrigin(float distanceToImg);
Point3 CalculateCurrentPoint(int i, int j, float pixelOffsetX, float pixelOffsetY, Point3 origin);
void MonteCarlo(LightList &copiedList, const HitInfo &hInfo, int x, int y, int bounces, int numOfSamples);
Color PathTrace(const HitInfo &hInfo, int x, int y, int bounces);

//Main Render Function
void Render(PixelIterator& i)
{
    Point3 imgOrigin;
    int x, y;
    
    srand(time(NULL));
    
    while (i.GetPixelLocation(x, y)) {
        
        
        if (x == 403 && y == 323) {
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
            float sampleX = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            float sampleTheta = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2 * M_PI)));
            float camOffsetX = sqrt(sampleX * camera.dof * camera.dof) * cos(sampleTheta);
            float camOffsetY = sqrt(sampleX * camera.dof * camera.dof) * sin(sampleTheta);
            
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
        
        //If hit, perform Monte Carlo, then shade the pixel
        if (hitResultSum) {
            
            Color pixelValuesSum = Color(0.0, 0.0, 0.0);
            std::array<Color, maxSampleSize> pixelValueArray;

            int currentStartIndex = 0;
            int currentSampleCount = minSampleSize;
            float currentVariance = 100.0;
            
            // Adaptive Sampling
            while (currentSampleCount < maxSampleSize /*&& currentVariance > targetVariance*/) {
                // Shading Calculation
                for (int index = currentStartIndex; index < currentSampleCount; index++) {
                    Color currentResult = Color(0.0, 0.0, 0.0);
                    
                    if (hitResult[index]) {
                        // Copy and Construct a new light list for monte carlo
                        LightList monteCarloList;

                        // Monte Carlo
                        MonteCarlo(monteCarloList, hitInfoArray[index], x, y, monteCarloBounces, monteCarloSampleSize);

                        currentResult = hitInfoArray[index].node->GetMaterial()->Shade(rayArray[index], hitInfoArray[index], monteCarloList, 5);
                        currentResult += hitInfoArray[index].node->GetMaterial()->Shade(rayArray[index], hitInfoArray[index], lights, 5);
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
            
            // Gamma Correction
            pixelValuesSum.r = pow(pixelValuesSum.r, 1/2.2);
            pixelValuesSum.g = pow(pixelValuesSum.g, 1/2.2);
            pixelValuesSum.b = pow(pixelValuesSum.b, 1/2.2);
            
            renderImage.GetSampleCount()[imgArrayIndex] = currentSampleCount;
            renderImage.GetPixels()[imgArrayIndex] = Color24(pixelValuesSum);
            renderImage.IncrementNumRenderPixel(1);
        }
        //Else, Sample background
        else {
            Color result = background.Sample(Point3((float)x/camera.imgWidth, (float)y/camera.imgHeight, 0));

            result.r = pow(result.r, 1/2.2);
            result.g = pow(result.g, 1/2.2);
            result.b = pow(result.b, 1/2.2);
            
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

//Sample a disk
Point2 SampleDisk(float radius)
{
    float sampleX = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/1.0));
    float sampleTheta = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2 * M_PI)));
    
    return Point2(sqrt(sampleX * camera.dof * camera.dof) * cos(sampleTheta),
                  sqrt(sampleX * camera.dof * camera.dof) * sin(sampleTheta));
}

//Sample a sphere
Point3 SampleSphere(Point3 origin, float radius)
{
//    float rand1 = Halton(HaltonIndex, 4) * radius;
//    HaltonIndex++;
//    float rand2 = Halton(HaltonIndex, 5) * radius;
//    HaltonIndex++;
//    float rand3 = Halton(HaltonIndex, 6) * radius;
//    HaltonIndex++;
    
    float rand1 = -radius + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(radius*2)));;
    float rand2 = -radius + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(radius*2)));;
    float rand3 = -radius + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(radius*2)));;

    Point3 offset = Point3(rand1,rand2,rand3);
    
    if (offset.Length() > radius) {
        offset = SampleSphere(origin, radius);
    }
    
    return offset;
}

//Sample a hemisphere

Point3 SampleHemiSphere(Point3 origin, Point3 normal, float radius)
{
    Point3 offset = SampleSphere(origin, radius);
    
    Point3 direction = offset.GetNormalized();
    
    if (direction.Dot(normal) < 0) {
        offset = -offset;
    }
    
    return offset;
}

//Cosine Weighted Hemisphere
Point3 SampleHemiSphereCosine(Point3 origin, Point3 normal, float radius)
{
    Point3 offset;
    
    float sampleX = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float samplePhi = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(2 * M_PI)));
    float sampleTheta = 0.5 * acos(1 - 2*sampleX);
    
    // Find two random vectors
    Point3 v1 = normal.Cross(Point3(sampleX)).GetNormalized();
    Point3 v2 = v1.Cross(normal).GetNormalized();
    
    offset = normal * (radius * cos(sampleTheta)) +
             v1 * (radius * sin(sampleTheta) * cos(samplePhi)) +
             v2 * (radius * sin(sampleTheta) * sin(samplePhi));
    
    return offset;
}

//Monte Carlo Sampling
void MonteCarlo(LightList &copiedList, const HitInfo &hInfo, int x, int y, int bounces, int numOfSamples)
{
    Color c = Color(0.0, 0.0, 0.0);
    
    if (bounces > 0)
    {
        // Sample a hemisphere on the hitpoint
        for (int index = 0; index < numOfSamples; index++) {
            // Data Structures
            HitInfo h = HitInfo();
            LightList currentSampleList;
            
            // Create sample ray
//            Point3 sampleOffset = SampleHemiSphere(hInfo.p, hInfo.N, 1.0);
            Point3 sampleOffset = SampleHemiSphereCosine(hInfo.p, hInfo.N, 1.0);
            Ray sampleRay = Ray(hInfo.p, sampleOffset.GetNormalized());
            
            // Trace & Shade
            if (Trace(sampleRay, &rootNode, h)) {
                const Material* currentMaterial = h.node->GetMaterial();
                
                //Stop when hit a light
                const char* sresult = currentMaterial->GetName();
                const char* compare = "light";
                if (strcmp(sresult, compare )!=0) {
                    MonteCarlo(currentSampleList, h, x, y, bounces-1, 1);
                    c += h.node->GetMaterial()->Shade(sampleRay, h, currentSampleList, 5);
                }
                
                c += h.node->GetMaterial()->Shade(sampleRay, h, lights, 5);
            }
            else {
                c += background.Sample(Point3((float)x/camera.imgWidth, (float)y/camera.imgHeight, 0));
            }
        }
        
        c /= (float)monteCarloSampleSize;
    }
    else {
        c = Color(0.1, 0.1, 0.1);
    }
    
    AmbientLight* a = new AmbientLight();
    a->SetIntensity(c);
    
    copiedList.push_back(a);
}
