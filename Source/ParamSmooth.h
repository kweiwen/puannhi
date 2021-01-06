//
//  ParaSmooth.h
//
//  Created by kweiwen tseng on 2020/6/25.
//

#ifndef ParamSmooth_h
#define ParamSmooth_h

const float c_twoPi = 6.283185307179586476925286766559f;

class ParamSmooth
{

public:
    ParamSmooth()
    {
        
    }
    
    ~ParamSmooth()
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

void ParamSmooth::createCoefficients(float smoothingTimeInMs, float sampleRate)
{
    a = exp(-c_twoPi / (smoothingTimeInMs * 0.001f * sampleRate));
    b = 1.0f - a;
    z = 0.0f;
    
    mSampleRate = sampleRate;
    mSmoothingTimeInMs = smoothingTimeInMs;
}

float ParamSmooth::getSampleRate()
{
    return mSampleRate;
}

float ParamSmooth::getSmoothingTimeInMs()
{
    return mSmoothingTimeInMs;
}

float ParamSmooth::process(float input)
{
    z = (input * b) + (z * a);
    return z;
}

#endif /* ParamSmooth_h */
