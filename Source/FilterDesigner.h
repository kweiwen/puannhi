//
//  FilterDesigner.h
//  CircularBuffer
//
//  Created by kweiwen tseng on 2021/1/10.
//  Copyright © 2021 Sikhaa Electronics. All rights reserved.
//

#ifndef FilterDesigner_h
#define FilterDesigner_h

#include <math.h>

enum E_FILTER_TYPE
{
	E_FLAT		  = 0,
	E_LOW_PASS_2  = 1,
	E_BAND_PASS   = 2,
	E_HIGH_PASS_2 = 3,
	E_LOW_PASS_1  = 4,
	E_HIGH_PASS_1 = 5,
	E_ALL_PASS_1  = 6,
	E_ALL_PASS_2  = 7,
	E_PEAK        = 8,
	E_PARAMETRIC  = 9,
	E_BAND_REJECT = 10,
	E_LOW_SHELF   = 11,
	E_HIGH_SHELF  = 12,
};

class FilterDesigner
{

public:
	FilterDesigner()
    {
		model = E_FLAT;
    };

    ~FilterDesigner()
    {
    };

	void setParameter(float cut_off = 1200, float sample_rate = 44100, float Q = 0.707, float slope = 0, float magnitude = 0);
    void setCoefficients();
	float* getCoefficients();
	int model;

private:
	double TWO_PI = 6.283185307179586476925286766559;
	double EULER = 2.71828182845904523536;

	// numerator of transfer function
	float b0 = 0;
	float b1 = 0;
	float b2 = 0;
	// denominator of transfer function
    float a0 = 0;
    float a1 = 0;
    float a2 = 0;

	float omega;
	float sine_omega;
	float cosine_omega;
	float gain;
	float q;
	float slope;
	float alpha;
};

#endif /* FilterDesigner_h */