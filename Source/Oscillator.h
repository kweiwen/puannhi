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

enum E_OSCILLATOR_TYPE
{
	E_SINE	        = 0,
	E_TRIANGLE      = 1,
	E_SAWTOOTH      = 2,
	E_TRAPEZOID     = 3,
	E_SQUARE        = 4,
    E_PHASOR        = 5,
    E_PHASOR_INV    = 6,
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
	double sine(double angle);
	double triangle(double angle);
	double sawtooth(double angle);
	double trapezoid(double angle);
	double square(double angle);
    double phasor(double angle);

private:
	double TWO_PI = 6.283185307179586476925286766559;
	double currentSample;
};

inline double Oscillator::process(double frequency, double sampleRate, int model)
{
	switch (model)
	{
	case E_SINE:
		currentSample = sine(currentAngle);
		break;
	case E_TRIANGLE:
		currentSample = triangle(currentAngle);
		break;
	case E_SAWTOOTH:
		currentSample = sawtooth(currentAngle);
		break;
	case E_TRAPEZOID:
		currentSample = trapezoid(currentAngle);
		break;
	case E_SQUARE:
		currentSample = square(currentAngle);
		break;
    case E_PHASOR:
        currentSample = phasor(currentAngle);
        break;
    case E_PHASOR_INV:
        currentSample = phasor(currentAngle+TWO_PI/2);
        break;
	}
	currentAngle = currentAngle + TWO_PI * frequency / sampleRate;
	return currentSample;
}


inline double Oscillator::sine(double angle)
{
	return sin(angle);
}

inline double Oscillator::triangle(double angle)
{
	return (4 / TWO_PI) * asin(sin(angle));
}

inline double Oscillator::sawtooth(double angle)
{
	double param, fractpart, intpart, output_data;
	param = angle / TWO_PI;
	fractpart = modf(param, &intpart);
	output_data = (fractpart - 0.5) * 2;
	return output_data;
}

inline double Oscillator::trapezoid(double angle)
{
	double output_data = 0;
	for (int index = 0; index < 16; index++)
	{
		auto coefficeint = (index * 2) + 1;
		output_data = output_data + sin(angle * coefficeint) / coefficeint;
	}
	return output_data;
}

inline double Oscillator::square(double angle)
{
	double output_data;
	if (sin(angle) > 0)
	{
		output_data = 1;
	}
	else if (sin(angle) < 0)
	{
		output_data = -1;
	}
	else
	{
		output_data = 0;
	}
	return output_data;
}

inline double Oscillator::phasor(double angle)
{
    double param, intpart, output_data;
    param = angle / TWO_PI;
    output_data = modf(param, &intpart);
    return output_data;
}

#endif /* Oscillator_h */
