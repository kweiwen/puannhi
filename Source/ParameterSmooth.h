//
//  ParameterSmooth.h
//
//  Created by kweiwen tseng on 2020/6/25.
//

#ifndef ParameterSmooth_h
#define ParameterSmooth_h

const float c_twoPi = 6.283185307179586476925286766559f;

class ParameterSmooth
{

public:
    ParameterSmooth()
    {
        
    }
    
    ~ParameterSmooth()
    {
    }
    
    void createCoefficients(float smoothingTimeInMs, float samplingRate);
    
    float getSampleRate();
    float getSmoothingTimeInMs();
    float process(float input);
    
private:
    float a;
    float b;
    float z;
    float mSampleRate;
    float mSmoothingTimeInMs;
};

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

#endif /* ParameterSmooth_h */
