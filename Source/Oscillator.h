//
//  Oscillator.h
//  CircularBuffer
//
//  Created by kweiwen tseng on 2021/3/24.
//  Copyright © 2021 Sikhaa Electronics. All rights reserved.
//

#ifndef Oscillator_h
#define Oscillator_h

#include <math.h>

enum E_FILTER_TYPE
{
	E_SINE	    = 0,
	E_TRIANGLE  = 1,
	E_SAWTOOTH  = 2,
	E_SQUARE    = 3,
	E_TRAPEZOID = 4,
};

class Oscillator
{
public:
	Oscillator()
	{
		currentAngle = 0;
	};

	~Oscillator()
	{
	};

	double getAngleDelta(double frequency, double sampleRate);
	double process(double frequency, double sampleRate);
	double currentAngle;


private:
	double c_twoPi = 6.283185307179586476925286766559f;
};

inline double Oscillator::getAngleDelta(double frequency, double sampleRate)
{
	auto angleDelta = c_twoPi * frequency / sampleRate;
	return angleDelta;
}

inline double Oscillator::process(double frequency, double sampleRate)
{
	auto currentSample = sin(currentAngle);
	currentAngle = currentAngle + c_twoPi* frequency / sampleRate;
	return currentSample;
}

#endif /* Oscillator_h */