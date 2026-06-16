//-------------------------------------------------------------------------------------------------------
// $Date: 2020/05/24 00:00:00 $
//
// Filename     : AllPassPhase.cpp
// Created by   : enummusic
// Description  : Crossover filter phase dispersion
//
// (c) 2020 enummusic
// VST SDK (c) 2005, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "AllPassPhase.h"

//-----------------------------------------------------------------------------
AllPassPhaseProgram::AllPassPhaseProgram ()
{
	// default Program Values
	fFrequency = 0.5131f;
	fIterations = 0.5f;
	fQ = 0.5f;
	fMix = 1.0f;

	vst_strncpy (name, "Init", sizeof (name));
}

//-----------------------------------------------------------------------------
AllPassPhase::AllPassPhase (audioMasterCallback audioMaster)
	: AudioEffectX (audioMaster, kNumPrograms, kNumParams)
{
	// init

	size = 44100;
	buffer = new float[size];

	programs = new AllPassPhaseProgram[numPrograms];
	fFrequency = 0.3675f;
	fQ = 0.5f;
	fIterations = 0.5f;
	fMix = 1.0f;

	if (programs)
		setProgram (0);

	setNumInputs (2);
	setNumOutputs (2);

	setUniqueID ('aphs');

	curIterations = getIterationCount();
	freq = knobToFrequency(fFrequency);
	q = fQ * sqrt(2.0f);
	setupFilters();

	resume ();		// flush buffer
}

//------------------------------------------------------------------------
AllPassPhase::~AllPassPhase ()
{
	if (buffer)
		delete[] buffer;
	if (programs)
		delete[] programs;
}

//------------------------------------------------------------------------
void AllPassPhase::setProgram (long program)
{
	AllPassPhaseProgram* ap = &programs[program];

	curProgram = program;
	setParameter (kFrequency, ap->fFrequency);
	setParameter (kQ, ap->fQ);
	setParameter (kIterations, ap->fIterations);
	setParameter (kMix, ap->fMix);
}

//------------------------------------------------------------------------
void AllPassPhase::setProgramName (char *name)
{
	vst_strncpy (programs[curProgram].name, name, sizeof (programs[curProgram].name));
}

//------------------------------------------------------------------------
void AllPassPhase::getProgramName (char *name)
{
	if (!strcmp (programs[curProgram].name, "Init"))
		sprintf (name, "%s %d", programs[curProgram].name, curProgram + 1);
	else
		vst_strncpy (name, programs[curProgram].name, kVstMaxProgNameLen);
}

//-----------------------------------------------------------------------------------------
bool AllPassPhase::getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text)
{
	if (index < kNumPrograms)
	{
		vst_strncpy (text, programs[index].name, kVstMaxProgNameLen);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
void AllPassPhase::resume ()
{
	memset (buffer, 0, size * sizeof (float));
	AudioEffectX::resume ();
}

//------------------------------------------------------------------------
void AllPassPhase::setParameter (VstInt32 index, float value)
{
	AllPassPhaseProgram* ap = &programs[curProgram];

	switch (index)
	{
		case kFrequency:
			fFrequency = ap->fFrequency = value;
			setupFilters();
			lastfFreq = fFrequency;
			break;
		case kQ:
			fQ = ap->fQ = value;
			setupFilters();
			break;
		case kIterations:
			fIterations = ap->fIterations = value;
			break;
		case kMix:
			fMix = ap->fMix = value;
			break;
	}
}

void AllPassPhase::setupFilters()
{
	freq = knobToFrequency(fFrequency);
	q = fQ * sqrt(2.0f);
	if (q <= 0.005f)
		q = 0.005f;

	const bool resetFilterState = fabs(fFrequency - lastfFreq) > fFrequency / 10 && freq < 500;

	filterL[0].setup(freq, 44100.0f, q);
	filterR[0].setup(freq, 44100.0f, q);

	for (int i = 1; i < getIterationCount(); i++) {
		filterL[i].copyCoefficientsFrom(filterL[0]);
		filterR[i].copyCoefficientsFrom(filterR[0]);

		if (resetFilterState) {
			filterL[i].zeroBuffers();
			filterR[i].zeroBuffers();
		}
	}
}

//------------------------------------------------------------------------
float AllPassPhase::getParameter (VstInt32 index)
{
	float v = 0;

	switch (index)
	{
		case kFrequency:
			v = fFrequency;
			break;
		case kQ:
			v = fQ;
			break;
		case kIterations:
			v = fIterations;
			break;
		case kMix:
			v = fMix;
			break;
	}
	return v;
}

//------------------------------------------------------------------------
void AllPassPhase::getParameterName (VstInt32 index, char *label)
{
	switch (index)
	{
		case kFrequency:
			vst_strncpy (label, "Frequency", kVstMaxParamStrLen);
			break;
		case kQ:
			vst_strncpy (label, "Q", kVstMaxParamStrLen);
			break;
		case kIterations:
			vst_strncpy (label, "Intensity", kVstMaxParamStrLen);
			break;
		case kMix:
			vst_strncpy(label, "Mix", kVstMaxParamStrLen);
			break;
	}
}

// https://www.musicdsp.org/en/latest/Other/260-exponential-curve-for.html
int AllPassPhase::knobToFrequency(float x)
{
	return floor(exp((16 + x * 100 * 1.20103)*log(1.059))*8.17742);
}

int AllPassPhase::getIterationCount() const
{
	const int iterations = (int)(fIterations * kMaxFilters);

	if (iterations < 0)
		return 0;
	if (iterations > kMaxFilters)
		return kMaxFilters;
	return iterations;
}

//------------------------------------------------------------------------
void AllPassPhase::getParameterDisplay (VstInt32 index, char *text)
{
	switch (index)
	{
		case kFrequency:
			int2string(knobToFrequency(fFrequency), text, 5);
			break;
		case kQ:
			float2string(fQ * sqrt(2.0f), text, 4);
			break;
		case kIterations:
			if (getIterationCount() == 0) {
				vst_strncpy(text, "BYPASS", kVstMaxParamStrLen);
			}
			else {
				int2string(getIterationCount(), text, 5);
			}
			break;
		case kMix:
			if (fMix == 0) {
				vst_strncpy(text, "0", kVstMaxParamStrLen);
			}
			else {
				int2string(fMix * 100, text, 3);
			}
			break;
	}
}

//------------------------------------------------------------------------
void AllPassPhase::getParameterLabel (VstInt32 index, char *label)
{
	switch (index)
	{
		case kFrequency:
			vst_strncpy (label, "Hz", kVstMaxParamStrLen);
			break;
		case kQ:
			vst_strncpy(label, " ", kVstMaxParamStrLen);
			break;
		case kIterations:
			vst_strncpy (label, "iterations", kVstMaxParamStrLen);
			break;
		case kMix:
			vst_strncpy(label, "%", kVstMaxParamStrLen);
			break;
	}
}

//------------------------------------------------------------------------
bool AllPassPhase::getEffectName (char* name)
{
	vst_strncpy (name, "AllPassPhase", kVstMaxEffectNameLen);
	return true;
}

//------------------------------------------------------------------------
bool AllPassPhase::getProductString (char* text)
{
	vst_strncpy (text, "AllPassPhase", kVstMaxProductStrLen);
	return true;
}

//------------------------------------------------------------------------
bool AllPassPhase::getVendorString (char* text)
{
	vst_strncpy (text, "enummusic", kVstMaxVendorStrLen);
	return true;
}

//---------------------------------------------------------------------------
void AllPassPhase::processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames)
{
	const int targetIterations = getIterationCount();

	if (targetIterations > curIterations) {
		if (curIterations > 0) {
			for (int i = curIterations; i < targetIterations; i++) {
				filterL[i].copyCoefficientsFrom(filterL[curIterations - 1]);
				filterR[i].copyCoefficientsFrom(filterR[curIterations - 1]);
			}
		}
		else {
			setupFilters();
		}
		curIterations = targetIterations;
	}
	else if (targetIterations < curIterations) {
		curIterations = targetIterations;
	}

	float* in1 = inputs[0];
	float* in2 = inputs[1];
	float* out1 = outputs[0];
	float* out2 = outputs[1];

	if (curIterations == 0 || fMix <= 0) {
		int samples = sampleFrames;
		while (--samples >= 0) {
			*out1++ = *in1++;
			*out2++ = *in2++;
		}
		return;
	}

	float *temp1{ new float[sampleFrames] {} };
	float *temp2{ new float[sampleFrames] {} };
	int samples = sampleFrames;

	// copy input to temp array
	while (--samples >= 0) {
		*temp1 = *in1++;
		*temp2 = *in2++;
		// checks the whole buffer
		// if it sees anything that isn't silence, reset the silence counter
		// ensures the entire buffer is processed
		if (fabs(*temp1) >= noiseFloor || fabs(*temp2) >= noiseFloor) {
			samplesSinceSilence = 0;
		}
		temp1++;
		temp2++;
	}

	// reset pointers
	temp1 -= sampleFrames;
	temp2 -= sampleFrames;
	samples = sampleFrames;

	// filter the audio
	if (samplesSinceSilence < deactivateAfterSamples) {
		float *left{ new float[sampleFrames] {} };
		float *right{ new float[sampleFrames] {} };

		for (int i = 0; i < curIterations; i++) {
			filterL[i].processBlock(temp1, left, sampleFrames);
			filterR[i].processBlock(temp2, right, sampleFrames);
			while (--samples >= 0) {
				*temp1++ = *left++;
				*temp2++ = *right++;
			}
			samples = sampleFrames;
			temp1 -= sampleFrames;
			temp2 -= sampleFrames;
			left -= sampleFrames;
			right -= sampleFrames;
		}

		delete[] left;
		delete[] right;
	}

	in1 -= sampleFrames;
	in2 -= sampleFrames;

	const float dryMix = 1.0f - fMix;

	while (--samples >= 0) {

		// if it sees anything that isn't silence, reset the silence counter
		if (fabs(*temp1) >= noiseFloor || fabs(*temp2) >= noiseFloor) {
			samplesSinceSilence = 0;
		}
		else if (samplesSinceSilence < 32768) { // int overflow protection
			samplesSinceSilence++;
		}

		*out1 = (*temp1 * fMix + *in1 * dryMix);
		*out2 = (*temp2 * fMix + *in2 * dryMix);

		in1++;
		in2++;
		out1++;
		out2++;
		temp1++;
		temp2++;
	}

	// always reset pointers before deleting the array
	temp1 -= sampleFrames;
	temp2 -= sampleFrames;
	delete[] temp1;
	delete[] temp2;
}
