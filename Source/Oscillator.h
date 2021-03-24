//
//  Oscillator.h
//  CircularBuffer
//
//  Created by kweiwen tseng on 2021/3/24.
//  Copyright © 2021 Sikhaa Electronics. All rights reserved.
//

#ifndef Oscillator_h
#define Oscillator_h

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
	};

	~Oscillator()
	{
	};

	/*float angleDelta(float frequency, float sampleRate);*/
	double getAngleDelta(float frequency, float sampleRate);

private:
	double c_twoPi = 6.283185307179586476925286766559f;

};

inline double Oscillator::getAngleDelta(float frequency, float sampleRate)
{
	auto angleDelta = c_twoPi * frequency / sampleRate;
	return angleDelta;
}


#endif /* Oscillator_h */