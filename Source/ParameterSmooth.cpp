//
//  ParameterSmooth.h
//  CircularBuffer
//
//  Created by kweiwen tseng on 2020/6/25.
//  Copyright © 2021 Sikhaa Electronics. All rights reserved.
//

#include "ParameterSmooth.h"
#include <math.h> 

void ParameterSmooth::createCoefficients(float smoothingTimeInMs, float sampleRate)
{
    a = exp(-c_twoPi / (smoothingTimeInMs * 0.001f * sampleRate));
    b = 1.0f - a;
    z = 0.0f;
    
    mSampleRate = sampleRate;
    mSmoothingTimeInMs = smoothingTimeInMs;
}

float ParameterSmooth::getSampleRate()
{
    return mSampleRate;
}

float ParameterSmooth::getSmoothingTimeInMs()
{
    return mSmoothingTimeInMs;
}

float ParameterSmooth::process(float input)
{
    z = (input * b) + (z * a);
    return z;
}

void ParameterSmooth::setSampleRate(float input)
{
    mSampleRate = input;
}

void ParameterSmooth::setSmoothingTimeInMs(float input)
{
    mSmoothingTimeInMs = input;
}
