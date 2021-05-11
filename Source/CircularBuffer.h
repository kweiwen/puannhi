//
//  CircularBuffer.h
//  CircularBuffer
//
//  Created by kweiwen tseng on 2021/1/10.
//  Copyright © 2021 Sikhaa Electronics. All rights reserved.
//

#ifndef CircularBuffer_h
#define CircularBuffer_h

template <typename T>
class CircularBuffer
{

public:
    CircularBuffer()
    {
        mWriteIndex = 0;
        mBufferLength = 0;
        mWrapMask = 0;
    };
    
    ~CircularBuffer()
    {
    };
    
    void createCircularBuffer(unsigned int input);
    void flushBuffer();
    void writeBuffer(T input);
    
    T readBuffer(int delayInSamples);
    T readBuffer(double delayInFractionalSamples, bool interpolate = true);
    
    float doLinearInterpolation(float delayInFractionalSamples);
    float doHermitInterpolation(float delayInFractionalSamples);
    float doLagrangeInterpolation(float delayInFractionalSamples);
    
private:
    std::unique_ptr<T[]> mBuffer = nullptr;
    unsigned int mWriteIndex;
    unsigned int mBufferLength;
    unsigned int mWrapMask;
};

template <typename T>
void CircularBuffer<T>::createCircularBuffer(unsigned int input)
{
    // --- reset the to top
    mWriteIndex = 0;
    // --- init buffer length as power of 2
    mBufferLength = (unsigned int)(pow(2, ceil(logf(input) / logf(2))));
    // --- warp mask as (mBufferLength - 1) for binary &= calculation
    mWrapMask = mBufferLength - 1;
    // --- direct initialization object into mBufferLength size
    mBuffer.reset(new T[mBufferLength]);
    // --- clean the value inside mBuffer
    flushBuffer();
}

template <typename T>
void CircularBuffer<T>::flushBuffer()
{
    for (int i = 0; i < mBufferLength; i++)
    {
        mBuffer[i] = 0;
    }
}

template <typename T>
void CircularBuffer<T>::writeBuffer(T input)
{
    mBuffer[mWriteIndex++] = input;
    mWriteIndex &= mWrapMask;
}

template<typename T>
T CircularBuffer<T>::readBuffer(int delayInSamples)
{
    int readIndex = mWriteIndex - delayInSamples;
    readIndex &= mWrapMask;
    return mBuffer[readIndex];
}

template<typename T>
// --- read an arbitrary location that includes a fractional sample
T CircularBuffer<T>::readBuffer(double delayInFractionalSamples, bool interpolate /*= true*/)
{
    // --- truncate delayInFractionalSamples and read the int part
    T y1 = readBuffer((int)delayInFractionalSamples);
    // --- if no interpolation, just return value
    if (!interpolate)
    {
        return y1;
    }
    // --- else do interpolation
    else
    {
        return doHermitInterpolation(delayInFractionalSamples);
    }
}

template<typename T>
float CircularBuffer<T>::doLinearInterpolation(float delayInFractionalSamples)
{
    float y1 = readBuffer((int)delayInFractionalSamples);
    float y2 = readBuffer((int)delayInFractionalSamples + 1);
    float fraction = delayInFractionalSamples - (int)delayInFractionalSamples;

    if (fraction >= 1.0) return y2;
    return fraction * y2 + (1 - fraction) * y1;
}

template<typename T>
float CircularBuffer<T>::doHermitInterpolation(float delayInFractionalSamples)
{
    int index = (int)delayInFractionalSamples;
    float xm1 = readBuffer(index - 1);;
    float x0 = readBuffer(index);
    float x1 = readBuffer(index + 1);
    float x2 = readBuffer(index + 2);

    float frac_pos = delayInFractionalSamples - (int)delayInFractionalSamples;

    const float c = (x1 - xm1) * 0.5f;
    const float v = x0 - x1;
    const float w = c + v;
    const float a = w + v + (x2 - x0) * 0.5f;
    const float b_neg = w + a;
    return ((((a * frac_pos) - b_neg) * frac_pos + c) * frac_pos + x0);
}

template<typename T>
float CircularBuffer<T>::doLagrangeInterpolation(float delayInFractionalSamples)
{
    int n = 4;
    int index = (int)delayInFractionalSamples;
    float x[4] = { index - 1, index, index + 1, index + 2 };
    float y[4] = { readBuffer(index - 1), readBuffer(index), readBuffer(index + 1), readBuffer(index + 2) };

    float interpolation = 0;
    for (int i = 0; i < n; i++)
    {
        float term = y[i];
        for (int j = 0; j < n; j++)
        {
            if (j != i)
            {
                term = term * (delayInFractionalSamples - x[j]) / (float)(x[i] - x[j]);
            }
        }
        interpolation += term;
    }
    return interpolation;
}


#endif /* CircularBuffer_h */
