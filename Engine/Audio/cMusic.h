#pragma once

// Includes
//=========
#if defined( EAE6320_PLATFORM_WINDOWS )
	#include "Windows/Includes.h"
#endif

#include "sEffect.h"

#include <Engine/Assets/ReferenceCountedAssets.h>
#include <Engine/Results/cResult.h>
#include <Engine/Asserts/Asserts.h>

#include <string>
#include <vector>
#include <cstdint>

namespace eae6320
{
	namespace Audio {
		class cMusic
		{
		public:
			static cResult Load(const char* p_MusicPath, cMusic*& o_Music);

			cResult Play(bool p_FromStart = false, bool p_IsLoop = false);
			cResult Stop();
			cResult ExitLoop();

			void SetVolume(float p_Volume);
			float GetVolume();
			void SetPitchRatio(float p_Ratio);
			float GetPitchRatio();

			~cMusic();

		private:
			cMusic() = default;
			
			cResult LoadMusicFile(std::wstring p_MusicPath, cMusic*& o_Music);
			cResult CleanUp();

			cResult SetUpEffect(sEffect p_Effect);

			IXAudio2SourceVoice* m_SourceVoice;	// XAudio2 source voice*
			std::vector<BYTE> m_AudioData;		// The audio data
			XAUDIO2_BUFFER m_AudioBuffer;		// The actual buffer with the audio data*

			float m_Fallof;						// Falloff distance
			unsigned int m_Priority;			// Music priority

			unsigned int m_Index;				// The index of the actual sound to play

		};
	}
}