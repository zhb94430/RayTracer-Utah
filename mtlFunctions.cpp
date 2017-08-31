//
//  mtlFunctions.cpp
//  RayTracerXcode
//
//  Created by Peter Zhang on 8/30/17.
//  Copyright © 2017 Peter Zhang. All rights reserved.
//

#ifndef _MTL_FUNC_INCLUDED_
#define _MTL_FUNC_INCLUDED_

#include "ExternalLibrary/materials.h"
#include "ExternalLibrary/scene.h"

Color MtlBlinn::Shade(const Ray &ray, const HitInfo &hInfo, const LightList &lights) const
{
    return Color(0,180,180);
}

#endif
