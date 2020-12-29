/*
	This file declares the external interface for the audio system
*/

#ifndef EAE6320_AUDIO_H
#define EAE6320_AUDIO_H

// Includes
//=========
#include <Engine/Results/Results.h>

// Interface
//==========

namespace eae6320
{
	namespace Audio{
		cResult Initialize();
		cResult CleanUp();
	}
}

#endif	// EAE6320_AUDIO_H