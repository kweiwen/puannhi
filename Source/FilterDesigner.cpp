//
//  FilterDesigner.h
//  CircularBuffer
//
//  Created by kweiwen tseng on 2021/1/10.
//  Copyright © 2021 Sikhaa Electronics. All rights reserved.
//

#include "FilterDesigner.h"

inline void FilterDesigner::setParameter(float cut_off, float sample_rate, float Q, float slope, float magnitude)
{
	omega = TWO_PI * cut_off / sample_rate;
	sine_omega = sin(omega);
	cosine_omega = cos(omega);
	gain = pow(10, (magnitude / 20));
	q = Q;
	slope = slope;
	setCoefficients();
}

void FilterDesigner::setCoefficients()
{
	switch (type)
	{
	case E_FLAT:
		b0 = 1;
		b1 = 0;
		b2 = 0;
		a0 = 1;
		a1 = 0;
		a2 = 0;
		break;
	case E_LOW_PASS_1:
		break;
	case E_LOW_PASS_2:
		alpha = sine_omega / (2 * q);

		// denominator normalization
		b0 = 1 + alpha;
		b1 = (-2 * cosine_omega) / b0;
		b2 = (1 - alpha) / b0;

		// numerator normalization
		a0 = (1 - cosine_omega) * gain / 2 / b0;
		a1 = (1 - cosine_omega) * gain / b0;
		a2 = (1 - cosine_omega) * gain / 2 / b0;

		// set b0 into 1 after coefficients normalization 
		b0 = 1;
		break;
	case E_HIGH_PASS_1:
		break;
	case E_HIGH_PASS_2:
		break;
	case E_ALL_PASS_1:
		break;
	case E_ALL_PASS_2:
		break;
	case E_PEAK:
		break;
	case E_PARAMETRIC:
		break;
	case E_BAND_PASS:
		break;
	case E_BAND_REJECT:
		break;
	case E_LOW_SHELF:
		break;
	case E_HIGH_SHELF:
		break;
	}
}

float* FilterDesigner::getCoefficients()
{
	static float coefficients[6];
	coefficients[0] = a0;
	coefficients[1] = a1;
	coefficients[2] = a2;
	coefficients[3] = b0;
	coefficients[4] = b1;
	coefficients[5] = b2;
	return coefficients;
}