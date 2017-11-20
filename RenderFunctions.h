//
//  RenderFunctions.cpp
//  RayTracerXcode
//
//  Created by Peter Zhang on 9/05/17.
//  Copyright © 2017 Peter Zhang. All rights reserved.
//

#ifndef _RENDERFUNC_H_INCLUDED_
#define _RENDERFUNC_H_INCLUDED_

bool Trace(const Ray &r, Node* currentNode, HitInfo &hInfo);
bool ShadowTrace(const Ray& r, Node* currentNode, HitInfo& hInfo);
Point3 SampleSphere(Point3 origin, float radius);

#endif
