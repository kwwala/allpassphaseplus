//-------------------------------------------------------------------------------------------------------
// $Date: 2020/05/24 00:00:00 $
//
// Filename     : AllPassPhase.h
// Created by   : enummusic
// Description  : Crossover filter phase dispersion
//
// (c) 2020 enummusic
// VST SDK (c) 2005, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#ifndef __AllPassPhase__
#define __AllPassPhase__

#include "public.sdk/source/vst2.x/audioeffectx.h"
#include "AllPassFilter.h"

enum
{
	// Global
	kNumPrograms = 16,

	// Parameters Tags
	kFrequency = 0,
	kQ,
	kIterations,
	kMix,

	kNumParams,
	kMaxFilters = 50
};

class AllPassPhase;

//------------------------------------------------------------------------
class AllPassPhaseProgram
{
friend class AllPassPhase;
public:
	AllPassPhaseProgram ();
	~AllPassPhaseProgram () {}

private:
	float fFrequency;
	float fQ;
	float fIterations;
	float fMix;
	char name[24];
};

//------------------------------------------------------------------------
class AllPassPhase : public AudioEffectX
{
public:
	AllPassPhase (audioMasterCallback audioMaster);
	~AllPassPhase ();

	virtual void processReplacing (float **inputs, float **outputs, VstInt32 sampleFrames);

	virtual void setProgram (long program);
	virtual void setProgramName (char *name);
	virtual void getProgramName (char *name);
	virtual bool getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text);

	virtual void setParameter (VstInt32 index, float value);
	void setupFilters();
	virtual float getParameter (VstInt32 index);
	virtual void getParameterLabel (VstInt32 index, char *label);
	virtual void getParameterDisplay (VstInt32 index, char *text);
	virtual void getParameterName (VstInt32 index, char *text);

	int knobToFrequency(float x);
	int getIterationCount() const;

	virtual void resume ();

	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual VstInt32 getVendorVersion () { return 1000; }

	virtual VstPlugCategory getPlugCategory () { return kPlugCategEffect; }

protected:
	AllPassPhaseProgram *programs;

	float *buffer;
	float fFrequency, fIterations, fQ, fMix;

	long size;

	AllPassFilter filterL[kMaxFilters];
	AllPassFilter filterR[kMaxFilters];
	int freq;
	int curIterations;
	float q;

	int samplesSinceSilence = 1;
	const int deactivateAfterSamples = 16384;
	const float noiseFloor = 0.000007f; // Approximately -103 dBFS.
	float lastfFreq = 0;
};

#endif
