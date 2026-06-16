#include "AllPassPhase.h"

//-------------------------------------------------------------------------------------------------------
AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
	return new AllPassPhase(audioMaster);
}

