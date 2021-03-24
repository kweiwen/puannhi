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

	double process(double frequency, double sampleRate, int model);
	double currentAngle;
	//double sineWave(double angle);
	//double squareWave(double angle);
	//double triangleWave(double angle);
	//double trapezoidWave(double angle);

private:
	double c_twoPi = 6.283185307179586476925286766559f;
	double currentSample;
};

inline double Oscillator::process(double frequency, double sampleRate, int model)
{
	switch (model)
	{
	case E_SINE:
		currentSample = sin(currentAngle);
		break;
	case E_TRIANGLE:
		currentSample = (4 / c_twoPi) * asin(sin(currentAngle));
		break;
	case E_SAWTOOTH:
		double param, fractpart, intpart;
		param = currentAngle / c_twoPi;
		fractpart = modf(param, &intpart);
		currentSample = (fractpart - 0.5)*2;
		break;
	case E_SQUARE:
		if (sin(currentAngle) > 0)
		{
			currentSample = 1;
		}
		else if (sin(currentAngle) < 0)
		{
			currentSample = -1;
		}
		else
		{
			currentSample = 0;
		}
		break;
	case E_TRAPEZOID:
		double buffer = 0;
		for (int index = 0; index < 16; index++)
		{
			auto coefficeint = (index * 2) + 1;
			buffer = buffer + sin(currentAngle * coefficeint) / coefficeint;
		};
		currentSample = buffer;
		break;
	}
	currentAngle = currentAngle + c_twoPi * frequency / sampleRate;
	return currentSample;
}


#endif /* Oscillator_h */