#include "../cMusic.h"

#include "../Context.h"
#include "../sMusicConfig.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/ScopeGuard/cScopeGuard.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Platform/Platform.h>

#include <vector>
#include <xaudio2fx.h>

namespace eae6320
{
	namespace Audio
	{
		cResult cMusic::LoadMusicFile(std::wstring p_MusicPath, cMusic*& o_Music)
		{
			auto result = eae6320::Results::Success;
			// stream index
			DWORD streamIndex = (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM;

			HRESULT hr = S_OK;
			Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader;
			hr = MFCreateSourceReaderFromURL(p_MusicPath.c_str(), sAudioEngineInstances::GlobalAudioInstances.pSourceReaderConfiguration.Get(), sourceReader.GetAddressOf());
			// select the first audio stream, and deselect all other streams
			hr = sourceReader->SetStreamSelection((DWORD)MF_SOURCE_READER_ALL_STREAMS, false);
			if (FAILED(hr))
			{
				result = Results::Failure;
				EAE6320_ASSERTF(false, "Can't deselect streams.");
				return result;
			}

			hr = sourceReader->SetStreamSelection(streamIndex, true);
			if (FAILED(hr))
			{
				result = Results::Failure;
				EAE6320_ASSERTF(false, "Can't select first.");
				return result;
			}

			// Query information about the media file
			Microsoft::WRL::ComPtr<IMFMediaType> nativeMediaType;
			hr = sourceReader->GetNativeMediaType(streamIndex, 0, nativeMediaType.GetAddressOf());
			if (FAILED(hr))
			{
				result = Results::Failure;
				EAE6320_ASSERTF(false, "Can't query media information.");
				return result;
			}

			// Make sure that this is really an audio file
			GUID majorType{};
			hr = nativeMediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
			if (majorType != MFMediaType_Audio)
			{
				result = Results::Failure;
				EAE6320_ASSERTF(false, "Not audio file.");
				return result;
			}

			// Check whether the audio file is compressed or uncompressed
			GUID subType{};
			hr = nativeMediaType->GetGUID(MF_MT_MAJOR_TYPE, &subType);
			if (!(subType == MFAudioFormat_Float || subType == MFAudioFormat_PCM))
			{
				// Audio file is compressed
				// SourceReader looks for decoders to perform our request
				Microsoft::WRL::ComPtr<IMFMediaType> partialType = nullptr;
				hr = MFCreateMediaType(partialType.GetAddressOf());
				if (FAILED(hr))
				{
					result = Results::Failure;
					EAE6320_ASSERTF(false, "Unable create media type.");
					return result;
				}

				// Set the media type to "audio"
				hr = partialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
				if (FAILED(hr))
				{
					result = Results::Failure;
					EAE6320_ASSERTF(false, "Unable to set media type to audio.");
					return result;
				}

				// Request uncompressed data
				hr = partialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
				if (FAILED(hr))
				{
					result = Results::Failure;
					EAE6320_ASSERTF(false, "Unable to set guid of media type to uncompressed.");
					return result;
				}

				hr = sourceReader->SetCurrentMediaType(streamIndex, NULL, partialType.Get());
				if (FAILED(hr))
				{
					result = Results::Failure;
					EAE6320_ASSERTF(false, "Unable to set current media type.");
					return result;
				}

				// partialType->();
			}


			// Uncompress the data and load it into an XAudio2 Buffer
			Microsoft::WRL::ComPtr<IMFMediaType> uncompressedAudioType = nullptr;
			hr = sourceReader->GetCurrentMediaType(streamIndex, uncompressedAudioType.GetAddressOf());
			if (FAILED(hr))
			{
				result = Results::Failure;
				EAE6320_ASSERTF(false, "Unable to retrieve the current media type.");
				return result;
			}

			// TODO decide what should be member variables
			WAVEFORMATEX* waveFormatEx;
			unsigned int waveLength;

			hr = MFCreateWaveFormatExFromMFMediaType(uncompressedAudioType.Get(), &waveFormatEx, &(waveLength));
			if (FAILED(hr))
			{
				result = Results::Failure;
				EAE6320_ASSERTF(false, "Unable to create the wave format.");
				return result;
			}

			// Ensure the stream is selected
			hr = sourceReader->SetStreamSelection(streamIndex, true);
			if (FAILED(hr))
			{
				result = Results::Failure;
				EAE6320_ASSERTF(false, "Unable to select audio stream.");
				return result;
			}

			// Copy data into byte vector
			Microsoft::WRL::ComPtr<IMFSample> sample = nullptr;
			Microsoft::WRL::ComPtr<IMFMediaBuffer> buffer = nullptr;
			BYTE* localAudioData = NULL;
			DWORD localAudioDataLength = 0;

			while (true)
			{
				DWORD flags = 0;
				hr = sourceReader->ReadSample(streamIndex, 0, nullptr, &flags, nullptr, sample.GetAddressOf());
				if (FAILED(hr))
				{
					result = Results::Failure;
					EAE6320_ASSERTF(false, "Unable to read audio sample.");
					return result;
				}

				// Check whether the data is still valid
				if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
					break;

				// Check for eof
				if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
					break;

				if (sample == nullptr)
					continue;

				// Convert data to contiguous buffer
				hr = sample->ConvertToContiguousBuffer(buffer.GetAddressOf());
				if (FAILED(hr))
				{
					result = Results::Failure;
					EAE6320_ASSERTF(false, "Unable to convert audio sample to contiguous buffer.");
					return result;
				}

				// lock buffer and copy data to local memory
				hr = buffer->Lock(&localAudioData, nullptr, &localAudioDataLength);
				if (FAILED(hr))
				{
					result = Results::Failure;
					EAE6320_ASSERTF(false, "Unable to lock the audio buffer.");
					return result;
				}

				for (size_t i = 0; i < localAudioDataLength; i++)
					o_Music->m_AudioData.push_back(localAudioData[i]);

				// unlock the buffer
				hr = buffer->Unlock();
				localAudioData = nullptr;

				if (FAILED(hr))
				{
					result = Results::Failure;
					EAE6320_ASSERTF(false, "Unable to lock the audio buffer.");
					return result;
				}
			}

			// Create source voice
			hr = sAudioEngineInstances::GlobalAudioInstances.pXAudio->CreateSourceVoice(&(o_Music->m_SourceVoice), waveFormatEx);
			if (FAILED(hr))
			{
				result = Results::Failure;
				EAE6320_ASSERTF(false, "Unable to create source voice.");
				return result;
			}

			ZeroMemory(&(o_Music->m_AudioBuffer), sizeof(XAUDIO2_BUFFER));
			o_Music->m_AudioBuffer.AudioBytes = (UINT32)o_Music->m_AudioData.size();
			o_Music->m_AudioBuffer.pAudioData = (BYTE* const)&o_Music->m_AudioData[0];
			o_Music->m_AudioBuffer.pContext = nullptr;

			// if (p_IsAutoplay) o_Music->Play(true);
			// if (p_IsLoop) o_Music->m_AudioBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;

			return result;
		}

		cResult cMusic::Play(bool p_FromStart, bool p_IsLoop) {
			cResult result = Results::Success;

			HRESULT hr = S_OK;
			// Submit the audio buffer to the source voice
			if (p_FromStart) {
				hr = m_SourceVoice->FlushSourceBuffers();
				if (FAILED(hr))
				{
					result = Results::Failure;
					EAE6320_ASSERTF(false, "Unable to flush source buffer.");
					return result;
				}
			}
			if (p_IsLoop) {
				m_AudioBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
			}
			hr = m_SourceVoice->SubmitSourceBuffer(&m_AudioBuffer);
			if (FAILED(hr))
			{
				result = Results::Failure;
				EAE6320_ASSERTF(false, "Unable to submit source buffer.");
				return result;
			}

			// Start the source voice
			m_SourceVoice->Start();

			return result;
		}

		cResult cMusic::Stop() {
			cResult result = Results::Success;

			HRESULT hr = m_SourceVoice->Stop();
			if (FAILED(hr))
			{
				result = Results::Failure;
				EAE6320_ASSERTF(false, "Unable to stop source voice.");
				return result;
			}

			return result;
		}

		void cMusic::SetVolume(float p_Volume) {
			m_SourceVoice->SetVolume(p_Volume);
		}

		void cMusic::SetPitchRatio(float p_Ratio) {
			m_SourceVoice->SetFrequencyRatio(p_Ratio);
		}

		float cMusic::GetVolume() {
			float volume = 0;
			m_SourceVoice->GetVolume(&volume);
			return volume;
		}

		float cMusic::GetPitchRatio() {
			float pitch = 1;
			m_SourceVoice->GetFrequencyRatio(&pitch);
			return pitch;
		}

		cResult cMusic::ExitLoop() {
			cResult result = Results::Success;

			HRESULT hr = m_SourceVoice->ExitLoop();;
			if (FAILED(hr))
			{
				result = Results::Failure;
				EAE6320_ASSERTF(false, "Unable to stop looping.");
				return result;
			}

			return result;
		}

		cResult cMusic::SetUpEffect(sEffect p_Effect) {
			cResult result = Results::Success;

			if (p_Effect.m_Reverb) {
				m_SourceVoice->EnableEffect(0);

				IUnknown* pXAPO;
				HRESULT hr = XAudio2CreateReverb(&pXAPO);
				if (FAILED(hr))
				{
					result = Results::Failure;
					EAE6320_ASSERTF(false, "Unable to create reverb.");
					return result;
				}

				XAUDIO2_EFFECT_DESCRIPTOR descriptor;
				descriptor.InitialState = true;
				descriptor.OutputChannels = 2;
				descriptor.pEffect = pXAPO;

				XAUDIO2_EFFECT_CHAIN chain;
				chain.EffectCount = 1;
				chain.pEffectDescriptors = &descriptor;

				hr = m_SourceVoice->SetEffectChain(&chain);
				if (FAILED(hr))
				{
					result = Results::Failure;
					EAE6320_ASSERTF(false, "Unable to set effect chain.");
					return result;
				}

				pXAPO->Release();

				XAUDIO2FX_REVERB_PARAMETERS reverbParameters;
				reverbParameters.ReflectionsDelay = p_Effect.m_ReflectionDel;
				reverbParameters.ReverbDelay = XAUDIO2FX_REVERB_DEFAULT_REVERB_DELAY;
				reverbParameters.RearDelay = XAUDIO2FX_REVERB_DEFAULT_REAR_DELAY;
				reverbParameters.PositionLeft = p_Effect.m_PositionLeft;
				reverbParameters.PositionRight = p_Effect.m_PositionRight;
				reverbParameters.PositionMatrixLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX;
				reverbParameters.PositionMatrixRight = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX;
				reverbParameters.EarlyDiffusion = XAUDIO2FX_REVERB_DEFAULT_EARLY_DIFFUSION;
				reverbParameters.LateDiffusion = XAUDIO2FX_REVERB_DEFAULT_LATE_DIFFUSION;
				reverbParameters.LowEQGain = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_GAIN;
				reverbParameters.LowEQCutoff = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_CUTOFF;
				reverbParameters.HighEQGain = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_GAIN;
				reverbParameters.HighEQCutoff = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_CUTOFF;
				reverbParameters.RoomFilterFreq = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_FREQ;
				reverbParameters.RoomFilterMain = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_MAIN;
				reverbParameters.RoomFilterHF = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_HF;
				reverbParameters.ReflectionsGain = XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_GAIN;
				reverbParameters.ReverbGain = XAUDIO2FX_REVERB_DEFAULT_REVERB_GAIN;
				reverbParameters.DecayTime = p_Effect.m_DecayTime;
				reverbParameters.Density = p_Effect.m_Density;
				reverbParameters.RoomSize = p_Effect.m_RoomSize;
				reverbParameters.WetDryMix = p_Effect.m_WetDryMix;

				hr = m_SourceVoice->SetEffectParameters(0, &reverbParameters, sizeof(reverbParameters));
				if (FAILED(hr))
				{
					result = Results::Failure;
					EAE6320_ASSERTF(false, "Unable to set reverb parameters.");
					return result;
				}
			}

			return result;
		}

		cMusic::~cMusic() {
			const auto result = CleanUp();
			EAE6320_ASSERT(result);
		}

		cResult cMusic::CleanUp() {
			cResult result = Results::Success;
			m_SourceVoice->Stop();
			m_SourceVoice->FlushSourceBuffers();
			m_SourceVoice->DestroyVoice();
			return result;
			
		}
	}
}