//
//  RenderFunctions.cpp
//  RayTracerXcode
//
//  Created by Peter Zhang on 8/25/17.
//  Copyright © 2017 Peter Zhang. All rights reserved.
//
#include "ExternalLibrary/scene.h"
#include "ExternalLibrary/objects.h"
#include "ExternalLibrary/cyPhotonMap.h"
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
const int photonMapSize = 1000000;
const int photonSampleSize = 100;
const int photonMaxBounce = 10;
const float photonEstRadius = 1;
const float photonEllipticity = 0.5;

//Sampling Variables
int HaltonIndex = 0;

cyPhotonMap pMap;

//Prototypes
Point3 CalculateImageOrigin(float distanceToImg);
Point3 CalculateCurrentPoint(int i, int j, float pixelOffsetX, float pixelOffsetY, Point3 origin);
Ray CalculateReflectedRay(const Ray incomingRay, const HitInfo &hInfo, const float reflectionGlossiness);
Ray CalculateRefractedRay(const Ray incomingRay, const HitInfo &hInfo, const float refractionGlossiness, const float ior);
Color PhotonMapping(const Ray &r, const HitInfo &hInfo);
Color MonteCarloPhoton(const HitInfo &hInfo, int x, int y, int numOfSamples);
//void MonteCarlo(LightList &copiedList, const Ray &r, const HitInfo &hInfo, int x, int y, int bounces, int numOfSamples);
void MonteCarlo(LightList &copiedList, const HitInfo &hInfo, int x, int y, int bounces, int numOfSamples);
Color PathTrace(const HitInfo &hInfo, int x, int y, int bounces);

//Main Render Function
void Render(PixelIterator& i)
{
    Point3 imgOrigin;
    int x, y;
    
    srand(time(NULL));
    
    while (i.GetPixelLocation(x, y)) {
//        if (x == 403 && y == 323) {
//            int j = 0;
//        }
        
        //Sample Array
        std::array<Ray, maxSampleSize> rayArray;
        std::array<HitInfo, maxSampleSize> hitInfoArray;
        std::array<bool, maxSampleSize> hitResult;
        float pixelIncrement = 1.0/maxSampleSize;
        bool hitResultSum = false;
        float zSum = 0.0;
        int numOfHits = 0;
        
        //Populate the two hitinfo array
        for (int index = 0; index < maxSampleSize; index++) {
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
//        renderImage.GetZBuffer()[imgArrayIndex] = zSum / (float)numOfHits;
        
        //If hit, perform Monte Carlo, then shade the pixel
        if (hitResultSum) {
            
            Color pixelValuesSum = Color(0.0, 0.0, 0.0);
            std::array<Color, maxSampleSize> pixelValueArray;
            
            // Shading Calculation
            for (int index = 0; index < maxSampleSize; index++) {
                Color currentResult = Color(0.0, 0.0, 0.0);
                
                if (hitResult[index]) {
                    // Copy and Construct a new light list for monte carlo
                        LightList monteCarloList;

                    // Monte Carlo
                        MonteCarlo(monteCarloList, hitInfoArray[index], x, y, monteCarloBounces, monteCarloSampleSize);

                        currentResult = hitInfoArray[index].node->GetMaterial()->Shade(rayArray[index], hitInfoArray[index], monteCarloList, 5);
                        currentResult += hitInfoArray[index].node->GetMaterial()->Shade(rayArray[index], hitInfoArray[index], lights, 5);
                    
                    // Photon Map + MonteCarlo
//                        currentResult += hitInfoArray[index].node->GetMaterial()->Shade(rayArray[index], hitInfoArray[index], lights, 5);
//                        currentResult += MonteCarloPhoton(hitInfoArray[index], x, y, monteCarloSampleSize);
                    
                    // Photon Map
//                    currentResult = PhotonMapping(rayArray[index], hitInfoArray[index]);
                }
                else {
                    currentResult = background.Sample(Point3((float)x/camera.imgWidth, (float)y/camera.imgHeight, 0));
                }
                
                pixelValuesSum += currentResult;
                pixelValueArray[index] = currentResult;
            }
            
            pixelValuesSum /= (float)maxSampleSize;
            
            // Gamma Correction
            pixelValuesSum.r = pow(pixelValuesSum.r, 1/2.2);
            pixelValuesSum.g = pow(pixelValuesSum.g, 1/2.2);
            pixelValuesSum.b = pow(pixelValuesSum.b, 1/2.2);
            
            renderImage.GetPixels()[imgArrayIndex] = Color24(pixelValuesSum);
            renderImage.IncrementNumRenderPixel(1);
        }
        //Else, Sample background
        else {
            Color result = background.Sample(Point3((float)x/camera.imgWidth, (float)y/camera.imgHeight, 0));

            result.r = pow(result.r, 1/2.2);
            result.g = pow(result.g, 1/2.2);
            result.b = pow(result.b, 1/2.2);
            
            renderImage.GetPixels()[imgArrayIndex] = Color24(result);
            renderImage.IncrementNumRenderPixel(1);
        }

    }
    
}

//Ray Tracing Logic
//Recurse through all child nodes,
//If an object exist, intersect ray, fill in hitinfo
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

// PhotonMapping

void GeneratePhotonMap()
{
    pMap.Resize(photonMapSize);
    
    int photonFromLight = 0;
    
    while (pMap.NumPhotons() < photonMapSize) {
        // TODO: Randomly decide on light source
        PointLight* currentLight = (PointLight*)lights[0];
        Color photonIntensity = currentLight->GetPhotonIntensity();
        
        // Generate Photon
        Ray photonRay = currentLight->RandomPhoton();
        HitInfo photonH = HitInfo();
        
        // Trace the first hit
        if (Trace(photonRay, &rootNode, photonH)) {
            photonFromLight++;
            
            if (photonH.node->GetMaterial()->IsPhotonSurface()) {
                // Record the first bounce
//                pMap.AddPhoton(photonH.p, photonRay.dir.GetNormalized(), photonIntensity);
            }
            
            Color currentIncomingIntensity = photonIntensity;
            Color currentOutgoingIntensity = photonIntensity;
            // Iteratively bounce the photon
            for (int i = 0; i < photonMaxBounce; i++) {
                
                currentIncomingIntensity = Color(currentOutgoingIntensity);

                if (photonH.node->GetMaterial()->RandomPhotonBounce(photonRay, currentOutgoingIntensity, photonH)) {
                    if (photonH.node->GetMaterial()->IsPhotonSurface()) {
                        pMap.AddPhoton(photonH.p, photonRay.dir.GetNormalized(), currentIncomingIntensity);
                    }
                }
                else {
                    break;
                }
            }
        }
    }
    
    float scaleFactor = (((PointLight*)lights[0])->GetPhotonIntensity() / photonFromLight).Gray();
    
    printf("Photon From Light: %i \n", photonFromLight);
    printf("Photon Scale Factor: %f \n", scaleFactor);
    
    pMap.ScalePhotonPowers(scaleFactor);
//    pMap.ScalePhotonPowers(0.00001);
    printf("Photon Map Generated\n");
    
    pMap.PrepareForIrradianceEstimation();
}

// Photon Mapping
Color PhotonMapping(const Ray &r, const HitInfo &hInfo)
{
    Color irradianceEst = Color(0.0, 0.0 ,0.0);
    Point3 irradianceDirection = Point3(0.0,0.0,0.0);
    LightList dummyLL;

    pMap.EstimateIrradiance<photonSampleSize>(irradianceEst, irradianceDirection, photonEstRadius, hInfo.p, &hInfo.N, photonEllipticity);
    
    PhotonLight* d = new PhotonLight();
    
    d->SetIntensity(irradianceEst);
    d->SetDirection(irradianceDirection);

    dummyLL.push_back(d);
    
    return hInfo.node->GetMaterial()->Shade(r, hInfo, dummyLL, 0);
}

//Monte Carlo + Photon Mapping
Color MonteCarloPhoton(const HitInfo &hInfo, int x, int y, int numOfSamples)
{
    Color c = Color(0.0, 0.0, 0.0);
    
    // First Bounce
    for (int index = 0; index < numOfSamples; index++) {
        HitInfo h = HitInfo();
        
        int actualBounces = 0;
        
        for (int b = 0; b < monteCarloBounces; b++) {
            // Create sample ray
            //            Point3 sampleOffset = SampleHemiSphere(hInfo.p, hInfo.N, 1.0);
            Point3 sampleOffset = SampleHemiSphereCosine(hInfo.p, hInfo.N, 1.0);
            Ray sampleRay = Ray(hInfo.p, sampleOffset.GetNormalized());
            
            actualBounces++;
            
            // Trace & Shade
            if (Trace(sampleRay, &rootNode, h)) {
                // Sample Photonmap
                c += PhotonMapping(sampleRay, h);
            }
            else {
                c += background.Sample(Point3((float)x/camera.imgWidth, (float)y/camera.imgHeight, 0));
                break;
            }
        }
        
        c /= (float)actualBounces;
    }
    
    c /= (float)monteCarloSampleSize;
    
    return c;
}

//Monte Carlo Sampling
void MonteCarlo(LightList &copiedList, const HitInfo &hInfo, int x, int y, int bounces, int numOfSamples)
{
//    Color cSum = Color(0.0, 0.0, 0.0);
//
//    for (int index = 0; index < numOfSamples; index++) {
//        LightList currentSampleList;
//
////        AmbientLight* a = new AmbientLight();
////        a->SetIntensity(Color(0.1,0.1,0.1));
////
////        currentSampleList.push_back(a);
//
////        Consider a stack implementation
//        std::array<HitInfo, monteCarloBounces> hInfoBounceArray;
//        std::array<Color, monteCarloBounces+1> directLightArray;
//
//        int actualBounces = 0;
//
//        // Forward Bounces
//        for (int b = 0; b < monteCarloBounces; b++) {
//            Color c = Color(0.0, 0.0, 0.0);
//            hInfoBounceArray[b] = HitInfo();
//            actualBounces++;
//
//            // Create sample ray
//            //            Point3 sampleOffset = SampleHemiSphere(hInfo.p, hInfo.N, 1.0);
//            Point3 sampleOffset = SampleHemiSphereCosine(hInfo.p, hInfo.N, 1.0);
//            Ray sampleRay = Ray(hInfo.p, sampleOffset.GetNormalized());
//
//            // Trace & Shade
//            if (Trace(sampleRay, &rootNode, hInfoBounceArray[b])) {
//
//                const Material* currentMaterial = hInfoBounceArray[b].node->GetMaterial();
//
//                c += currentMaterial->Shade(sampleRay, hInfoBounceArray[b], lights, 5);
//
//                // Store direct light
//                directLightArray[b] = c;
//
////                c += h.node->GetMaterial()->Shade(sampleRay, h, currentSampleList, 5);
//
////                AmbientLight* a = new AmbientLight();
////                a->SetIntensity(c);
////
////                currentSampleList.push_back(a);
//            }
//            else {
//                c += background.Sample(Point3((float)x/camera.imgWidth, (float)y/camera.imgHeight, 0));
//
//                directLightArray[b] = c;
//
////                AmbientLight* a = new AmbientLight();
////                a->SetIntensity(c);
////
////                currentSampleList.push_back(a);
//                break;
//            }
//        }
//
//        // Add constant ambient light
//        directLightArray[actualBounces] = Color(0.1,0.1,0.1);
//
//        // Backward loop calculating indirect light
//        for (int i = actualBounces-1; i >= 0; i--) {
//            // Get the previous light
//            Color prevI = directLightArray[i+1];
//
//            // Create Ambient Light Source
//            LightList dummy;
//            AmbientLight* a = new AmbientLight();
//            a->SetIntensity(prevI);
//            dummy.push_back(a);
//
//            if (hInfoBounceArray[i].node) {
//                directLightArray[i] += hInfoBounceArray[i].node->GetMaterial()->Shade(Ray(), hInfoBounceArray[i], dummy, 5);
//            }
//            else {
//                directLightArray[i] += prevI;
//            }
//        }
//
//        AmbientLight* a = new AmbientLight();
//        a->SetIntensity(directLightArray[0]);
//        currentSampleList.push_back(a);
//
//        cSum += hInfo.node->GetMaterial()->Shade(r, hInfo, currentSampleList, 5);
//    }
//
//    cSum /= (float)numOfSamples;
//
//    AmbientLight* a = new AmbientLight();
//    a->SetIntensity(cSum);
//
//    copiedList.push_back(a);
    
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

                MonteCarlo(currentSampleList, h, x, y, bounces-1, 1);
                c += currentMaterial->Shade(sampleRay, h, currentSampleList, 5);
                c += currentMaterial->Shade(sampleRay, h, lights, 5);
            }
            else {
//                c += background.Sample(Point3((float)x/camera.imgWidth, (float)y/camera.imgHeight, 0));
        
                c += environment.SampleEnvironment(sampleRay.dir);
                
//                c += Color(0.1,0.1,0.1);
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

Ray CalculateReflectedRay(const Ray incomingRay, const HitInfo &hInfo, const float reflectionGlossiness) {
    // Glossiness Sampling
    Point3 sampleOrigin = hInfo.p+hInfo.N;
    Point3 sampledOffset = SampleSphere(sampleOrigin, reflectionGlossiness);
    Point3 sampledNormal = (sampleOrigin + sampledOffset - hInfo.p).GetNormalized();
    
    //Calculate the reflected ray direction
    Point3 reflectedDirection = (incomingRay.dir - 2*incomingRay.dir.Dot(sampledNormal)*sampledNormal).GetNormalized();
    
    Ray result = Ray(hInfo.p, reflectedDirection);

    return result;
}

Ray CalculateRefractedRay(const Ray incomingRay, const HitInfo &hInfo, const float refractionGlossiness, const float ior) {
    //Calculate the reflected ray direction
    Point3 sampleOrigin = hInfo.p-hInfo.N;
    Point3 sampledOffset = SampleSphere(sampleOrigin, refractionGlossiness);
    Point3 sampledNormal = (sampleOrigin + sampledOffset - hInfo.p).GetNormalized();
    
    //Calcluate the refracted ray direction
    float cosTheta1 = sampledNormal.Dot(-incomingRay.dir);
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
    
    Point3 SVector = sampledNormal.Cross(sampledNormal.Cross(-incomingRay.dir).GetNormalized()).GetNormalized();
    
    Point3 refractedDirection = (-(sampledNormal)*cosTheta2 + SVector*sinTheta2).GetNormalized();
    
    Ray result = Ray(hInfo.p, refractedDirection);
    
    return result;
}

