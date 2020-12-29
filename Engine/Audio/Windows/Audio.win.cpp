#include "../Audio.h"

#include "Includes.h"
#include "../Context.h"

#include <Engine/Logging/Logging.h>
#include <Engine/UserOutput/UserOutput.h>
#include <Engine/Asserts/Asserts.h>


// Interface
//==========

eae6320::cResult eae6320::Audio::Initialize() {
	cResult result = Results::Success;

	HRESULT hr;

#if _WIN32
	hr = CoInitialize(NULL);
#endif

	// Create an instance of XAudio2 engine
	if (FAILED(hr = XAudio2Create(sAudioEngineInstances::GlobalAudioInstances.pXAudio.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR)))
	{
		result = Results::Failure;
		EAE6320_ASSERTF(false, "Can't initialize without IXAudio2.");
		Logging::OutputError("Can't initialize without IXAudio2.");
		UserOutput::Print("Can't initialize without IXAudio2.");
		return result;
	}

	// Create a mastering voice
	// The mastering voices encapsulates an audio device
	// It is the ultimate destination for all audio that passes through an audio graph
	if (FAILED(hr = sAudioEngineInstances::GlobalAudioInstances.pXAudio->CreateMasteringVoice(&sAudioEngineInstances::GlobalAudioInstances.pMasteringVoice)))
	{
		result = Results::Failure;
		EAE6320_ASSERTF(false, "Can't initialize without matering voice.");
		Logging::OutputError("Can't initialize without mastering voice.");
		UserOutput::Print("Can't initialize without mastering voice.");
		return result;
	}

	// Initialize media foundation
	hr = MFStartup(MF_VERSION);
	if (FAILED(hr))
	{
		result = Results::Failure;
		EAE6320_ASSERTF(false, "Can't initialize without WMF.");
		Logging::OutputError("Can't initialize without WMF.");
		UserOutput::Print("Can't initialize without WMF.");
		return result;
	}

	// Set media foundation reader to low latency
	hr = MFCreateAttributes(sAudioEngineInstances::GlobalAudioInstances.pSourceReaderConfiguration.GetAddressOf(), 1);
	if (FAILED(hr))
	{
		result = Results::Failure;
		EAE6320_ASSERTF(false, "Can't initialize without IMFAttributes.");
		Logging::OutputError("Can't initialize without IMFAttributes.");
		UserOutput::Print("Can't initialize without IMFAttributes.");
		return result;
	}

	hr = sAudioEngineInstances::GlobalAudioInstances.pSourceReaderConfiguration->SetUINT32(MF_LOW_LATENCY, true);
	if (FAILED(hr))
	{
		result = Results::Failure;
		EAE6320_ASSERTF(false, "Can't initialize without reader configuration.");
		Logging::OutputError("Can't initialize without reader configuration.");
		UserOutput::Print("Can't initialize without reader configuration.");
		return result;
	}


	return result;
}

eae6320::cResult eae6320::Audio::CleanUp() {
	MFShutdown();

	// TODO Done in struct
	sAudioEngineInstances::GlobalAudioInstances.pMasteringVoice->DestroyVoice();
	sAudioEngineInstances::GlobalAudioInstances.pXAudio->StopEngine();

	return Results::Success;
}