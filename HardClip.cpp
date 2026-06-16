#include "HardClip.h"

float HardClip::process(float in, float threshold)
{
	if (in > threshold)
		return threshold;

	if (in < -threshold)
		return -threshold;

	return in;
}
