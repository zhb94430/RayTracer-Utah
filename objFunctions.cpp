//
//  objFunctions.cpp
//  RayTracerXcode
//
//  Created by Peter Zhang on 8/30/17.
//  Copyright © 2017 Peter Zhang. All rights reserved.
//


#include "ExternalLibrary/objects.h"
#include "ExternalLibrary/scene.h"
#include <vector>

//Sphere Intersection
bool Sphere::IntersectRay(const Ray &ray, HitInfo &hInfo, int hitSide) const
{
    if (GetBoundBox().IntersectRay(ray, BIGFLOAT)) {
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
            
            float u = 0.5-atan2(hInfo.N.x, hInfo.N.y)/(2*M_PI);
            float v = 0.5+asin(hInfo.N.z)/M_PI;
            
            hInfo.uvw = Point3(u,v,0);
            
            return true;
        }
        else if (m < n  && m < hInfo.z && (m >= 0.001 | n >= 0.001)) {
            if (m <= 0.001 && n > 0.001 && n < hInfo.z) {
                hInfo.z = n;
                hInfo.front = false;
            }
            else if (m > 0.001) {
                hInfo.z = m;
                hInfo.front = true;
            }
            
            Point3 temp = ray.p + hInfo.z * ray.dir;
            
            if (hInfo.front) {
                hInfo.N = temp.GetNormalized();
            }
            else {
                hInfo.N = -temp.GetNormalized();
            }
            
            hInfo.p = temp;
            
            float u = 0.5-atan2(hInfo.N.x, hInfo.N.y)/(2*M_PI);
            float v = 0.5+asin(hInfo.N.z)/M_PI;
            
            hInfo.uvw = Point3(u,v,0);
            
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
            
            if (hInfo.front) {
                hInfo.N = temp.GetNormalized();
            }
            else {
                hInfo.N = -temp.GetNormalized();
            }
            
            hInfo.p = temp;
            
            float u = 0.5-atan2(hInfo.N.x, hInfo.N.y)/(2*M_PI);
            float v = 0.5+asin(hInfo.N.z)/M_PI;
            
            hInfo.uvw = Point3(u,v,0);
            
            return true;
        }
    }
    
    return false;
}

//Plane Intersection
bool Plane::IntersectRay(const Ray &ray, HitInfo &hInfo, int hitSide) const
{
    if (GetBoundBox().IntersectRay(ray, BIGFLOAT)) {
        if (ray.dir.z != 0) {
            float t = (-ray.p.z)/(ray.dir.z);
            
            if (t > 0.001 && t < hInfo.z) {
                Point3 q = ray.p + ray.dir*t;
                
                if (q.x > -1 && q.x < 1 &&
                    q.y > -1 && q.y < 1) {
                    
                    if (ray.p.z > 0) {
                        hInfo.front = true;
                        hInfo.N = Point3(0,0,1);
                    }
                    else {
                        hInfo.front = false;
                        hInfo.N = Point3(0,0,-1);
                    }
                    
                    q = Point3(q.x, q.y, 0);
                    hInfo.z = t;
                    hInfo.p = q;
                    hInfo.uvw = Point3((q.x+1)/2, (q.y+1)/2, 0);
                    
                    return true;
                }
            }
        }
    }
    
    return false;
}

//Bounding Box Intersection
bool Box::IntersectRay(const Ray &r, float t_max) const {
    Point3 allMin = pmin;
    Point3 allMax = pmax;
    float tEntry;
    float tExit;
    
    //Special Cases
    if (IsEmpty()) {
        return false;
    }
    
    if (r.dir.x == 0) {
        //Parallel to X Plane
        float ty0 = (allMin.y - r.p.y)/r.dir.y;
        float ty1 = (allMax.y - r.p.y)/r.dir.y;
        float tz0 = (allMin.z - r.p.z)/r.dir.z;
        float tz1 = (allMax.z - r.p.z)/r.dir.z;
        
        if (ty0 > ty1) {
            float temp = ty1;
            ty1 = ty0;
            ty0 = temp;
        }
        if (tz0 > tz1) {
            float temp = tz1;
            tz1 = tz0;
            tz0 = temp;
        }
        
        tEntry = std::max(tz0, ty0);
        tExit = std::min(tz1, ty1);
    }
    else if (r.dir.y == 0) {
        //Parallel to Y Plane
        float tx0 = (allMin.x - r.p.x)/r.dir.x;
        float tx1 = (allMax.x - r.p.x)/r.dir.x;
        float tz0 = (allMin.z - r.p.z)/r.dir.z;
        float tz1 = (allMax.z - r.p.z)/r.dir.z;
        
        if (tx0 > tx1) {
            float temp = tx1;
            tx1 = tx0;
            tx0 = temp;
        }
        if (tz0 > tz1) {
            float temp = tz1;
            tz1 = tz0;
            tz0 = temp;
        }
        
        tEntry = std::max(tz0, tx0);
        tExit = std::min(tz1, tx1);
    }
    else if (r.dir.z == 0) {
        //Parallel to Z Plane
        float tx0 = (allMin.x - r.p.x)/r.dir.x;
        float tx1 = (allMax.x - r.p.x)/r.dir.x;
        float ty0 = (allMin.y - r.p.y)/r.dir.y;
        float ty1 = (allMax.y - r.p.y)/r.dir.y;
        
        if (tx0 > tx1) {
            float temp = tx1;
            tx1 = tx0;
            tx0 = temp;
        }
        if (ty0 > ty1) {
            float temp = ty1;
            ty1 = ty0;
            ty0 = temp;
        }
        
        tEntry = std::max(ty0, tx0);
        tExit = std::min(ty1, tx1);
    }
    else {
        //General Case
        float tx0 = (allMin.x - r.p.x)/r.dir.x;
        float tx1 = (allMax.x - r.p.x)/r.dir.x;
        float ty0 = (allMin.y - r.p.y)/r.dir.y;
        float ty1 = (allMax.y - r.p.y)/r.dir.y;
        float tz0 = (allMin.z - r.p.z)/r.dir.z;
        float tz1 = (allMax.z - r.p.z)/r.dir.z;
        
        //Make sure Pt0 is smaller than Pt1
        if (tx0 > tx1) {
            float temp = tx1;
            tx1 = tx0;
            tx0 = temp;
        }
        if (ty0 > ty1) {
            float temp = ty1;
            ty1 = ty0;
            ty0 = temp;
        }
        if (tz0 > tz1) {
            float temp = tz1;
            tz1 = tz0;
            tz0 = temp;
        }
        
        //Compute Entry and Exit Point
        tEntry = std::max(std::max(tx0, ty0), tz0);
        tExit = std::min(std::min(tx1, ty1), tz1);
    }

    if (tEntry <= tExit && tEntry < t_max) {
        return true;
    }
    else {
        return false;
    }
}

//Triangle Intersection
bool TriObj::IntersectTriangle(const Ray &ray, HitInfo &hInfo, int hitSide, unsigned int faceID) const {
    //Get the triangle
    Point3 A = V(F(faceID).v[0]);
    Point3 B = V(F(faceID).v[1]);
    Point3 C = V(F(faceID).v[2]);
    
    Point3 N = (B-A).Cross(C-A).GetNormalized();
    
    if (ray.dir.Dot(N) != 0) {
        
        float t = (A-ray.p).Dot(N) / ray.dir.Dot(N);
        
        //Calculate BaryCentric Coordinates
        if (t > 0.00001 && t < hInfo.z) {
            Point3 q = ray.p + ray.dir*t;
            
            //Project triangle into 2D
            float maxNormalAxis = std::max(std::max(fabsf(N.x), fabsf(N.y)), fabsf(N.z));
            
            Point2 ProjectedA, ProjectedB, ProjectedC, ProjectedQ;
            
            if (maxNormalAxis == fabsf(N.x)) {
                ProjectedA = Point2(A.y, A.z);
                ProjectedB = Point2(B.y, B.z);
                ProjectedC = Point2(C.y, C.z);
                ProjectedQ = Point2(q.y, q.z);
            }
            else if (maxNormalAxis == fabsf(N.y)) {
                ProjectedA = Point2(A.x, A.z);
                ProjectedB = Point2(B.x, B.z);
                ProjectedC = Point2(C.x, C.z);
                ProjectedQ = Point2(q.x, q.z);
            }
            else if (maxNormalAxis == fabsf(N.z)) {
                ProjectedA = Point2(A.x, A.y);
                ProjectedB = Point2(B.x, B.y);
                ProjectedC = Point2(C.x, C.y);
                ProjectedQ = Point2(q.x, q.y);
            }
            
            //Calculate BC based on area
            float TriABCArea = (ProjectedC-ProjectedA).Cross(ProjectedB-ProjectedA)/2.0;
            float TriAPCArea = (ProjectedC-ProjectedA).Cross(ProjectedQ-ProjectedA)/2.0;
            float TriABPArea = (ProjectedQ-ProjectedA).Cross(ProjectedB-ProjectedA)/2.0;
            
            float BC1 = TriAPCArea/TriABCArea;
            float BC2 = TriABPArea/TriABCArea;
            float BC3 = 1.0 - BC1 - BC2;
            
            if (BC1 > 0 && BC2 > 0 && BC3 > 0 &&
                BC1 < 1 && BC2 < 1 && BC3 < 1) {
                Point3 bc = Point3(BC3, BC1, BC2);
                
                if (ray.dir.Dot(N) < 0) {
                    hInfo.front = true;
                }
                else {
                    hInfo.front = false;
                }
                
                hInfo.uvw = GetTexCoord(faceID, bc);
                hInfo.N = GetNormal(faceID, bc).GetNormalized();
                hInfo.z = t;
                hInfo.p = GetPoint(faceID, bc);
                
                return true;
            }
        }
    }
    
    return false;
}

//BVHBox Intersection for BVH traversal
float BVHBoxIntersection(const Ray &r, Box bvhBox, float t_max);

bool TriObj::IntersectRay(const Ray &ray, HitInfo &hInfo, int hitSide) const
{
    bool hitResult = false;
    
    if (GetBoundBox().IntersectRay(ray, BIGFLOAT)){
        //Traverse Bounding Volume Hierarchy
        static const int STACK_MAX = 100;
        int stackTop = 0;
        unsigned int BVHTraceStack[STACK_MAX] = {0};
        
        BVHTraceStack[stackTop] = bvh.GetRootNodeID();
        
        //Start iteration
        while (stackTop >= 0) {
            unsigned int currentNodeIndex = BVHTraceStack[stackTop];
            stackTop--;

            //If not a leaf node
            //Check which child node is closer
            if (!bvh.IsLeafNode(currentNodeIndex)) {
                unsigned int firstChildIndex = bvh.GetFirstChildNode(currentNodeIndex);
                unsigned int secondChildIndex = bvh.GetSecondChildNode(currentNodeIndex);
                Box child1Box = Box(bvh.GetNodeBounds(firstChildIndex));
                Box child2Box = Box(bvh.GetNodeBounds(secondChildIndex));

                float child1TValue = BVHBoxIntersection(ray, child1Box, BIGFLOAT);
                float child2TValue = BVHBoxIntersection(ray, child2Box, BIGFLOAT);

                // if (child1TValue > 0) {
                //     if (child2TValue > 0) {
                //         if (child1TValue <= child2TValue) {
                //             stackTop++;
                //             BVHTraceStack[stackTop] = secondChildIndex;
                //             stackTop++;
                //             BVHTraceStack[stackTop] = firstChildIndex;
                //         }
                //         else {
                //             stackTop++;
                //             BVHTraceStack[stackTop] = firstChildIndex;
                //             stackTop++;
                //             BVHTraceStack[stackTop] = secondChildIndex;
                //         }
                //     }
                //     else {
                //         stackTop++;
                //         BVHTraceStack[stackTop] = firstChildIndex;
                //     }
                // }
                // else if (child2TValue > 0) {
                //     stackTop++;
                //     BVHTraceStack[stackTop] = secondChildIndex;
                // }

                // if (child1TValue <= child2TValue && child1TValue > 0)
                // {
                //     stackTop++;
                //     BVHTraceStack[stackTop] = secondChildIndex;
                //     stackTop++;
                //     BVHTraceStack[stackTop] = firstChildIndex;
                // }
                // else if (child2TValue <= child1TValue && child2TValue > 0)
                // {
                //     stackTop++;
                //     BVHTraceStack[stackTop] = firstChildIndex;
                //     stackTop++;
                //     BVHTraceStack[stackTop] = secondChildIndex;
                // }
                // else if (child1TValue > 0 && child2TValue <= 0)
                // {
                //     stackTop++;
                //     BVHTraceStack[stackTop] = firstChildIndex;
                // }
                // else if (child2TValue > 0 && child1TValue <= 0)
                // {
                //     stackTop++;
                //     BVHTraceStack[stackTop] = secondChildIndex;
                // }

                if (child1TValue <= child2TValue)
                {
                    if (child2TValue > 0)
                    {
                        stackTop++;
                        BVHTraceStack[stackTop] = secondChildIndex;
                    }

                    if (child1TValue > 0)
                    {
                        stackTop++;
                        BVHTraceStack[stackTop] = firstChildIndex;
                    }
                }

                else if (child1TValue > child2TValue)
                {
                    if (child1TValue > 0)
                    {
                        stackTop++;
                        BVHTraceStack[stackTop] = firstChildIndex;
                    }

                    if (child2TValue > 0)
                    {
                        stackTop++;
                        BVHTraceStack[stackTop] = secondChildIndex;
                    }
                }


                // if (child1TValue < child2TValue)
                // {
                //     stackTop++;
                //     BVHTraceStack[stackTop] = secondChildIndex;
                //     stackTop++;
                //     BVHTraceStack[stackTop] = firstChildIndex;
                // }
                // else {
                //     stackTop++;
                //     BVHTraceStack[stackTop] = firstChildIndex;
                //     stackTop++;
                //     BVHTraceStack[stackTop] = secondChildIndex;
                // }

                // stackTop++;
                // BVHTraceStack[stackTop] = secondChildIndex;
                // stackTop++;
                // BVHTraceStack[stackTop] = firstChildIndex;
            }
            //Intersect with leaf node
            else {
                //Iterate through all faces
                for (int i = 0; i < bvh.GetNodeElementCount(currentNodeIndex); i++) {
                    hitResult |= IntersectTriangle(ray, hInfo, hitSide, bvh.GetNodeElements(currentNodeIndex)[i]);
                }
            }
        }
        
        //Iterate through all faces
       // for (int i = 0; i < NF(); i++) {
       //     hitResult |= IntersectTriangle(ray, hInfo, hitSide, i);
       // }
    }
    return hitResult;
}

float BVHBoxIntersection(const Ray &r, Box bvhBox, float t_max) {
    Point3 allMin = bvhBox.pmin;
    Point3 allMax = bvhBox.pmax;
    float tEntry;
    float tExit;
    
    //Special Cases
    if (bvhBox.IsEmpty()) {
        return -t_max;
    }
    
    if (r.dir.x == 0) {
        //Parallel to X Plane
        float ty0 = (allMin.y - r.p.y)/r.dir.y;
        float ty1 = (allMax.y - r.p.y)/r.dir.y;
        float tz0 = (allMin.z - r.p.z)/r.dir.z;
        float tz1 = (allMax.z - r.p.z)/r.dir.z;
        
        //Make sure Pt0 is smaller than Pt1
        if (ty0 > ty1) {
            float temp = ty1;
            ty1 = ty0;
            ty0 = temp;
        }
        if (tz0 > tz1) {
            float temp = tz1;
            tz1 = tz0;
            tz0 = temp;
        }
        
        tEntry = std::max(tz0, ty0);
        tExit = std::min(tz1, ty1);
    }
    else if (r.dir.y == 0) {
        //Parallel to Y Plane
        float tx0 = (allMin.x - r.p.x)/r.dir.x;
        float tx1 = (allMax.x - r.p.x)/r.dir.x;
        float tz0 = (allMin.z - r.p.z)/r.dir.z;
        float tz1 = (allMax.z - r.p.z)/r.dir.z;
        
        //Make sure Pt0 is smaller than Pt1
        if (tx0 > tx1) {
            float temp = tx1;
            tx1 = tx0;
            tx0 = temp;
        }
        if (tz0 > tz1) {
            float temp = tz1;
            tz1 = tz0;
            tz0 = temp;
        }
        
        tEntry = std::max(tz0, tx0);
        tExit = std::min(tz1, tx1);
    }
    else if (r.dir.z == 0) {
        //Parallel to Z Plane
        float tx0 = (allMin.x - r.p.x)/r.dir.x;
        float tx1 = (allMax.x - r.p.x)/r.dir.x;
        float ty0 = (allMin.y - r.p.y)/r.dir.y;
        float ty1 = (allMax.y - r.p.y)/r.dir.y;
        
        //Make sure Pt0 is smaller than Pt1
        if (tx0 > tx1) {
            float temp = tx1;
            tx1 = tx0;
            tx0 = temp;
        }
        if (ty0 > ty1) {
            float temp = ty1;
            ty1 = ty0;
            ty0 = temp;
        }
        
        tEntry = std::max(ty0, tx0);
        tExit = std::min(ty1, tx1);
    }
    else {
        //General Case
        float tx0 = (allMin.x - r.p.x)/r.dir.x;
        float tx1 = (allMax.x - r.p.x)/r.dir.x;
        float ty0 = (allMin.y - r.p.y)/r.dir.y;
        float ty1 = (allMax.y - r.p.y)/r.dir.y;
        float tz0 = (allMin.z - r.p.z)/r.dir.z;
        float tz1 = (allMax.z - r.p.z)/r.dir.z;
        
        //Make sure Pt0 is smaller than Pt1
        if (tx0 > tx1) {
            float temp = tx1;
            tx1 = tx0;
            tx0 = temp;
        }
        if (ty0 > ty1) {
            float temp = ty1;
            ty1 = ty0;
            ty0 = temp;
        }
        if (tz0 > tz1) {
            float temp = tz1;
            tz1 = tz0;
            tz0 = temp;
        }
        
        //Compute Entry and Exit Point
        tEntry = std::max(std::max(tx0, ty0), tz0);
        tExit = std::min(std::min(tx1, ty1), tz1);
    }
    
    if (tEntry <= tExit && tEntry < t_max) {
        return tEntry + 0.001;
    }
    else {
        return -t_max;
    }
}
