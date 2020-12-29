#pragma once

#if defined( EAE6320_PLATFORM_WINDOWS )
	#include "Windows/Includes.h"
#endif

namespace eae6320
{
	namespace Audio
	{
		struct sAudioEngineInstances {
			Microsoft::WRL::ComPtr<IXAudio2> pXAudio = nullptr;
			IXAudio2MasteringVoice* pMasteringVoice = nullptr;
			Microsoft::WRL::ComPtr<IMFAttributes> pSourceReaderConfiguration = nullptr;

			static sAudioEngineInstances GlobalAudioInstances;

		private:
			sAudioEngineInstances() = default;
		};
		
	}
}