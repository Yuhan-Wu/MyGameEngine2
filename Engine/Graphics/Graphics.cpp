#include "Graphics.h"

#include "cConstantBuffer.h"
#include "ConstantBufferFormats.h"
#include "sContext.h"


#include <Engine/Concurrency/cEvent.h>
#include <Engine/Logging/Logging.h>
#include <Engine/UserOutput/UserOutput.h>
#include <math.h> 

#define MAX_TO_BE_DRAWN 20
// Static Data
//============

namespace
{
	// Constant buffer object
	eae6320::Graphics::cConstantBuffer s_constantBuffer_frame(eae6320::Graphics::ConstantBufferTypes::Frame);
	eae6320::Graphics::cConstantBuffer s_drawCall_data(eae6320::Graphics::ConstantBufferTypes::DrawCall);

	// Submission Data
	//----------------

	// This struct's data is populated at submission time;
	// it must cache whatever is necessary in order to render a frame
	struct sDataRequiredToRenderAFrame
	{
		eae6320::Graphics::ConstantBufferFormats::sFrame constantData_frame;
		eae6320::Graphics::ConstantBufferFormats::sDrawCall drawCall_data[MAX_TO_BE_DRAWN];
		eae6320::Graphics::cEffect* effect[MAX_TO_BE_DRAWN] = { nullptr };
		eae6320::Graphics::cMesh* mesh[MAX_TO_BE_DRAWN] = { nullptr };
		float background_color[4] = { -1.0f };
		uint8_t CurIndex = 0;

		void SetBackgroundColor(float p_Background[4]) {
			std::copy(p_Background, p_Background + 4, background_color);
		}

		void SetMeshEffectDrawCallPair(eae6320::Graphics::cEffect* p_Effect, eae6320::Graphics::cMesh* p_Mesh, eae6320::Graphics::ConstantBufferFormats::sDrawCall p_DrawCall) {
			p_Effect->IncrementReferenceCount();
			p_Mesh->IncrementReferenceCount();
			effect[CurIndex] = p_Effect;
			mesh[CurIndex] = p_Mesh;
			drawCall_data[CurIndex] = p_DrawCall;
			CurIndex++;
		}

		void Reset() {
			std::fill(background_color, background_color + 4, -1.0f);

			for (int i = 0; i < MAX_TO_BE_DRAWN; i++) {
				if (effect[i]) {
					effect[i]->DecrementReferenceCount();
					effect[i] = nullptr;
				}
				if (mesh[i]) {
					mesh[i]->DecrementReferenceCount();
					mesh[i] = nullptr;
				}
			}

			CurIndex = 0;
		}
	};
	// In our class there will be two copies of the data required to render a frame:
	//	* One of them will be in the process of being populated by the data currently being submitted by the application loop thread
	//	* One of them will be fully populated and in the process of being rendered from in the render thread
	// (In other words, one is being produced while the other is being consumed)
	sDataRequiredToRenderAFrame s_dataRequiredToRenderAFrame[2];
	auto* s_dataBeingSubmittedByApplicationThread = &s_dataRequiredToRenderAFrame[0];
	auto* s_dataBeingRenderedByRenderThread = &s_dataRequiredToRenderAFrame[1];
	// The following two events work together to make sure that
	// the main/render thread and the application loop thread can work in parallel but stay in sync:
	// This event is signaled by the application loop thread when it has finished submitting render data for a frame
	// (the main/render thread waits for the signal)
	eae6320::Concurrency::cEvent s_whenAllDataHasBeenSubmittedFromApplicationThread;
	// This event is signaled by the main/render thread when it has swapped render data pointers.
	// This means that the renderer is now working with all the submitted data it needs to render the next frame,
	// and the application loop thread can start submitting data for the following frame
	// (the application loop thread waits for the signal)
	eae6320::Concurrency::cEvent s_whenDataForANewFrameCanBeSubmittedFromApplicationThread;

}

// Interface
//==========

// Submission
//-----------

void eae6320::Graphics::SubmitElapsedTime(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_simulationTime)
{
	EAE6320_ASSERT(s_dataBeingSubmittedByApplicationThread);
	auto& constantData_frame = s_dataBeingSubmittedByApplicationThread->constantData_frame;
	constantData_frame.g_elapsedSecondCount_systemTime = i_elapsedSecondCount_systemTime;
	constantData_frame.g_elapsedSecondCount_simulationTime = i_elapsedSecondCount_simulationTime;
}

eae6320::cResult eae6320::Graphics::WaitUntilDataForANewFrameCanBeSubmitted(const unsigned int i_timeToWait_inMilliseconds)
{
	return Concurrency::WaitForEvent(s_whenDataForANewFrameCanBeSubmittedFromApplicationThread, i_timeToWait_inMilliseconds);
}

eae6320::cResult eae6320::Graphics::SignalThatAllDataForAFrameHasBeenSubmitted()
{
	return s_whenAllDataHasBeenSubmittedFromApplicationThread.Signal();
}

void eae6320::Graphics::SubmitBackgroundColor(sColor p_Background) {
	float background[4] = { p_Background.R, p_Background.G,p_Background.B,p_Background.A };
	s_dataBeingSubmittedByApplicationThread->SetBackgroundColor(background);
}

void eae6320::Graphics::SubmitMeshEffectDrawCallPair(cEffect* p_Effect, cMesh* p_Mesh, eae6320::Graphics::ConstantBufferFormats::sDrawCall p_DrawCall) {
	uint8_t cur_index = s_dataBeingSubmittedByApplicationThread->CurIndex;
#ifdef DEBUG
	EAE6320_ASSERT(1 + cur_capacity <= MAX_TO_BE_DRAWN)
#endif // DEBUG
	if (1 + cur_index <= MAX_TO_BE_DRAWN) {
		s_dataBeingSubmittedByApplicationThread->SetMeshEffectDrawCallPair(p_Effect, p_Mesh, p_DrawCall);
	}
}

void eae6320::Graphics::SubmitCameraInfo(Math::cMatrix_transformation p_WorldToCamera, Math::cMatrix_transformation p_CameraToProjected) {
	s_dataBeingSubmittedByApplicationThread->constantData_frame.g_transform_worldToCamera = p_WorldToCamera;
	s_dataBeingSubmittedByApplicationThread->constantData_frame.g_transform_cameraToProjected = p_CameraToProjected;
}

// Render
//-------

void eae6320::Graphics::RenderFrame()
{
	// Wait for the application loop to submit data to be rendered
	{
		if (Concurrency::WaitForEvent(s_whenAllDataHasBeenSubmittedFromApplicationThread))
		{
			// Switch the render data pointers so that
			// the data that the application just submitted becomes the data that will now be rendered
			std::swap(s_dataBeingSubmittedByApplicationThread, s_dataBeingRenderedByRenderThread);
			// Once the pointers have been swapped the application loop can submit new data
			if (!s_whenDataForANewFrameCanBeSubmittedFromApplicationThread.Signal())
			{
				EAE6320_ASSERTF(false, "Couldn't signal that new graphics data can be submitted");
				Logging::OutputError("Failed to signal that new render data can be submitted");
				UserOutput::Print("The renderer failed to signal to the application that new graphics data can be submitted."
					" The application is probably in a bad state and should be exited");
				return;
			}
		}
		else
		{
			EAE6320_ASSERTF(false, "Waiting for the graphics data to be submitted failed");
			Logging::OutputError("Waiting for the application loop to submit data to be rendered failed");
			UserOutput::Print("The renderer failed to wait for the application to submit data to be rendered."
				" The application is probably in a bad state and should be exited");
			return;
		}
	}

	// TODO: Color should be valid
	if (s_dataBeingRenderedByRenderThread->background_color[0] > 0) {
		float background[3] = { 0 };
		std::copy(s_dataBeingRenderedByRenderThread->background_color,
			(s_dataBeingRenderedByRenderThread->background_color) + 3, background);
		sContext::g_context.ClearScreen(background, s_dataBeingRenderedByRenderThread->background_color[3]);
	}
	

	for (int i = 0; i < s_dataBeingRenderedByRenderThread->CurIndex; i++) {
		cEffect* cur_effect = s_dataBeingRenderedByRenderThread->effect[i];
		cMesh* cur_mesh = s_dataBeingRenderedByRenderThread->mesh[i];
		if (cur_effect) {
			cur_effect->Bind();
		}
		if (cur_mesh) {
			// Update draw call data
			{
				auto& drawCall_data = s_dataBeingRenderedByRenderThread->drawCall_data[i];
				s_drawCall_data.Update(&drawCall_data);
			}
			cur_mesh->Draw();
		}
	}
	

	EAE6320_ASSERT(s_dataBeingRenderedByRenderThread);

	// Update the frame constant buffer
	{
		// Copy the data from the system memory that the application owns to GPU memory
		auto& constantData_frame = s_dataBeingRenderedByRenderThread->constantData_frame;
		s_constantBuffer_frame.Update(&constantData_frame);
	}

	sContext::g_context.Display();

	// After all of the data that was submitted for this frame has been used
	// you must make sure that it is all cleaned up and cleared out
	// so that the struct can be re-used (i.e. so that data for a new frame can be submitted to it)
	{
		s_dataBeingRenderedByRenderThread->Reset();
	}
}

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::Initialize(const sInitializationParameters& i_initializationParameters)
{
	char str1[100];
	strcpy(str1, "The size of effect is:");
	strcat(str1, std::to_string(sizeof(cEffect)).c_str());
	Logging::OutputMessage(str1);

	char str2[100];
	strcpy(str2, "The size of mesh is:");
	strcat(str2, std::to_string(sizeof(cMesh)).c_str());
	Logging::OutputMessage(str2);

	char str3[100];
	strcpy(str3, "The size of structure is:");
	strcat(str3, std::to_string(sizeof(sDataRequiredToRenderAFrame)).c_str());
	Logging::OutputMessage(str3);

	auto result = Results::Success;

	// Initialize the platform-specific context
	if (!(result = sContext::g_context.Initialize(i_initializationParameters)))
	{
		EAE6320_ASSERTF(false, "Can't initialize Graphics without context");
		return result;
	}
	// Initialize the platform-independent graphics objects
	{
		if (result = s_constantBuffer_frame.Initialize())
		{
			// There is only a single frame constant buffer that is reused
			// and so it can be bound at initialization time and never unbound
			s_constantBuffer_frame.Bind(
				// In our class both vertex and fragment shaders use per-frame constant data
				static_cast<uint_fast8_t>(eShaderType::Vertex) | static_cast<uint_fast8_t>(eShaderType::Fragment));
		}
		else
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without frame constant buffer");
			return result;
		}
	}
	// Initialize the draw call
	{
		if (result = s_drawCall_data.Initialize())
		{
			s_drawCall_data.Bind(
				// In our class both vertex and fragment shaders use per-frame constant data
				static_cast<uint_fast8_t>(eShaderType::Vertex) | static_cast<uint_fast8_t>(eShaderType::Fragment));
		}
		else
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without draw call buffer");
			return result;
		}
	}
	// Initialize the events
	{
		if (!(result = s_whenAllDataHasBeenSubmittedFromApplicationThread.Initialize(Concurrency::EventType::ResetAutomaticallyAfterBeingSignaled)))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without event for when data has been submitted from the application thread");
			return result;
		}
		if (!(result = s_whenDataForANewFrameCanBeSubmittedFromApplicationThread.Initialize(Concurrency::EventType::ResetAutomaticallyAfterBeingSignaled,
			Concurrency::EventState::Signaled)))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without event for when data can be submitted from the application thread");
			return result;
		}
	}

	return result;
}

eae6320::cResult eae6320::Graphics::CleanUp()
{
	auto result = Results::Success;

	s_dataBeingSubmittedByApplicationThread->Reset();
	s_dataBeingRenderedByRenderThread->Reset();

	{
		const auto result_constantBuffer_frame = s_constantBuffer_frame.CleanUp();
		if (!result_constantBuffer_frame)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_constantBuffer_frame;
			}
		}
	}

	{
		const auto result_drawCall_data = s_drawCall_data.CleanUp();
		if (!result_drawCall_data)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_drawCall_data;
			}
		}
	}

	{
		const auto result_context = sContext::g_context.CleanUp();
		if (!result_context)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_context;
			}
		}
	}

	return result;
}


