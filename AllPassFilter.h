#ifndef __AllPassFilter__
#define __AllPassFilter__

// code referenced:
// https://www.musicdsp.org/en/latest/Filters/197-rbj-audio-eq-cookbook.html
// https://www.musicdsp.org/en/latest/Filters/64-biquad-c-code.html
// https://www.musicdsp.org/en/latest/Filters/266-4th-order-linkwitz-riley-filters.html

class AllPassFilter {
private:

	// coefficient calculations need to be in double
	// for filter stability at low frequencies
	struct filterCoefficients {
		double c0, c1, c2, c3, c4;
	} co;

	struct {
		double xm1 = 0.0f;
		double xm2 = 0.0f;
		double ym1 = 0.0f, ym2 = 0.0f;
	} temp;

	float freq = 100.0f;
	float q;
public:
	void setup(float freq, float sr, float q);
	void copyCoefficientsFrom(const AllPassFilter& filter);
	void zeroBuffers();
	void processBlock(float * in, float * out, int numSamples);
	float getFreq() {
		return freq;
	}
	float getQ() {
		return q;
	}
};

#endif
