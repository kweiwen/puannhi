//
//  DelayAPF.h
//  CircularBuffer
//
//  Created by kweiwen tseng on 2021/1/9.
//  Copyright © 2021 Sikhaa Electronics. All rights reserved.
//

#include "CircularBuffer.h"

#ifndef DelayAPF_h
#define DelayAPF_h

template <typename T>
class DelayAPF: public CircularBuffer<T>
{
    
public:
    DelayAPF()
    {
    
    };
    
    DelayAPF(T bufferLength)
    {
        digitalDelayLine.createCircularBuffer(bufferLength);
    };
    
    ~DelayAPF()
    {
    
    };
    
    T processSchroeder(T sample, T delaySample, float delayGain);
    T processGerzon(T sample, T delaySample, float delayGain);
    CircularBuffer<T> digitalDelayLine;
};

template <typename T>
inline T DelayAPF<T>::processSchroeder(T sample, T delaySample, float delayGain)
{
    auto delayedSample = digitalDelayLine.readBuffer(delaySample);
    digitalDelayLine.writeBuffer(sample + (delayedSample * delayGain));
    
    auto feedfordwardSample = sample * -delayGain;
    return (delayedSample + feedfordwardSample);
}


template <typename T>
inline T DelayAPF<T>::processGerzon(T sample, T delaySample, float delayGain)
{
    auto delayedSample = digitalDelayLine.readBuffer(delaySample);
    digitalDelayLine.writeBuffer(sample + (delayedSample * delayGain));
    
    delayedSample = delayedSample * (1 - (delayGain * delayGain));
    auto feedfordwardSample = sample * -delayGain;
    return (delayedSample + feedfordwardSample);
}


#endif /* DelayAPF_h */
