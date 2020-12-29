#include "cMusic.h"

#include "Context.h"
#include "sMusicConfig.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/ScopeGuard/cScopeGuard.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Platform/Platform.h>

#include <string>

namespace eae6320
{
	namespace Audio
	{
		cResult cMusic::Load(const char* p_MusicPath, cMusic*& o_Music)
		{
			cResult result = Results::Success;
			EAE6320_ASSERT(o_Music == nullptr);

			o_Music = new (std::nothrow) cMusic();
			if (!o_Music)
			{
				result = Results::OutOfMemory;
				EAE6320_ASSERTF(false, "Couldn't allocate memory for an effect");
				Logging::OutputError("Failed to allocate memory for an effect");
				return result;
			}

			// Load binary file
			Platform::sDataFromFile data_from_file;
			std::string err_message;

			result = Platform::LoadBinaryFile(p_MusicPath, data_from_file, &err_message);

			if (!result) {
				Logging::OutputError(err_message.c_str());
				return result;
			}

			auto currentOffset = reinterpret_cast<uintptr_t>(data_from_file.data);
			const auto finalOffset = currentOffset + data_from_file.size;

			wchar_t* music_path = reinterpret_cast<wchar_t*>(currentOffset);
			currentOffset += wcslen(music_path) * sizeof(wchar_t);

			const auto music_config = reinterpret_cast<sMusicConfig*>(currentOffset);
			currentOffset += sizeof(sMusicConfig);

			const auto music_effect = reinterpret_cast<sEffect*>(currentOffset);
			currentOffset += sizeof(sEffect);

			if (!(result = o_Music->LoadMusicFile(std::wstring(music_path), o_Music))) {
				EAE6320_ASSERTF(false, "Initialization of music failed");
				return result;
			}

			if (music_config->m_Autoplay) o_Music->Play(true);
			if (music_config->m_IsLoop) o_Music->m_AudioBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
			o_Music->SetPitchRatio(music_config->m_pitch);
			o_Music->SetVolume(music_config->m_Volume);

			o_Music->SetUpEffect(*music_effect);

			return result;
		}
	}
}