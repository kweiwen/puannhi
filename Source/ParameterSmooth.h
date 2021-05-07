//
//  ParameterSmooth.h
//  CircularBuffer
//
//  Created by kweiwen tseng on 2020/6/25.
//  Copyright © 2021 Sikhaa Electronics. All rights reserved.
//

#ifndef ParameterSmooth_h
#define ParameterSmooth_h

class ParameterSmooth
{

public:
    ParameterSmooth()
    {
        a = 0;
        b = 0;
        z0 = 0;
        z1 = 0;
        mSampleRate = 0;
        mSmoothingTimeInMs = 0;
    };
    
    ~ParameterSmooth()
    {
    };
    
    void createCoefficients(float smoothingTimeInMs, float samplingRate);
    
    float getSampleRate();
    float getSmoothingTimeInMs();
    void setSampleRate(float input);
    void setSmoothingTimeInMs(float input);
    float process(float input);
    
private:
    float c_twoPi = 6.283185307179586476925286766559f;
    float a;
    float b;
    float z0;
    float z1;
    float mSampleRate;
    float mSmoothingTimeInMs;
};

#endif /* ParameterSmooth_h */
