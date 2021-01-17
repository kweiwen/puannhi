//
//  DelayFeedback.h
//  CircularBuffer
//
//  Created by kweiwen tseng on 2021/1/10.
//  Copyright © 2021 Sikhaa Electronics. All rights reserved.
//

#include "CircularBuffer.h"

#ifndef DelayFeedback_h
#define DelayFeedback_h

template <typename T>
class DelayFeedback: public CircularBuffer<T>
{
    
public:
    DelayFeedback()
    {

    };
    
    DelayFeedback(T bufferLength)
    {
        digitalDelayLine.createCircularBuffer(bufferLength);
    };
    
    ~DelayFeedback()
    {
    
    };
    
    T process(double input, float timeCtrl, float feedbackCtrl, float mixCtrl);
    CircularBuffer<T> digitalDelayLine;
};

template <typename T>
T DelayFeedback<T>::process(double input, float timeCtrl, float feedbackCtrl, float mixCtrl)
{
    auto drySignal = input;
    auto wetSignal = digitalDelayLine.readBuffer(timeCtrl);
    digitalDelayLine.writeBuffer(drySignal + wetSignal * feedbackCtrl);
    return wetSignal * mixCtrl + drySignal * (1 - mixCtrl);
}

#endif /* DelayFeedback_h */
