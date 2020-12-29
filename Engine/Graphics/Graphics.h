/*
	This file declares the external interface for the graphics system
*/

#ifndef EAE6320_GRAPHICS_H
#define EAE6320_GRAPHICS_H

// Includes
//=========

#include "Configuration.h"
#include "cMesh.h"
#include "cEffect.h"
#include "ConstantBufferFormats.h"

#include <cstdint>
#include <Engine/Results/Results.h>

#if defined( EAE6320_PLATFORM_WINDOWS )
	#include <Engine/Windows/Includes.h>
#endif

// Interface
//==========

namespace eae6320
{
	namespace Graphics
	{
		// Submission
		//-----------

		// These functions should be called from the application (on the application loop thread)

		// As the class progresses you will add your own functions for submitting data,
		// but the following is an example (that gets called automatically)
		// of how the application submits the total elapsed times
		// for the frame currently being submitted
		void SubmitElapsedTime( const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_simulationTime );

		// When the application is ready to submit data for a new frame
		// it should call this before submitting anything
		// (or, said another way, it is not safe to submit data for a new frame
		// until this function returns successfully)
		cResult WaitUntilDataForANewFrameCanBeSubmitted( const unsigned int i_timeToWait_inMilliseconds );
		// When the application has finished submitting data for a frame
		// it must call this function
		cResult SignalThatAllDataForAFrameHasBeenSubmitted();


		struct sColor {
			float R = 0;
			float G = 0;
			float B = 0;
			float A = 1.f;

			sColor(float p_R, float p_G, float p_B, float p_A = 1.f) {
				R = p_R;
				G = p_G;
				B = p_B;
				A = p_A;
			}

			bool operator==(sColor p_Other) {
				if (p_Other.R - R <= 0.01f && p_Other.G - G <= 0.01f && p_Other.B - B <= 0.01f) {
					return true;
				}
				return false;
			}
		};

		void SubmitBackgroundColor(sColor p_Background);
		void SubmitMeshEffectDrawCallPair(cEffect* p_Effect, cMesh* p_Mesh, ConstantBufferFormats::sDrawCall p_DrawCall);
		void SubmitCameraInfo(Math::cMatrix_transformation p_WorldToCamera, Math::cMatrix_transformation p_CameraToProjected);

		// Render
		//-------

		// This is called (automatically) from the main/render thread.
		// It will render a submitted frame as soon as it is ready
		// (i.e. as soon as SignalThatAllDataForAFrameHasBeenSubmitted() has been called)
		void RenderFrame();

		// Initialize / Clean Up
		//----------------------

		struct sInitializationParameters
		{
#if defined( EAE6320_PLATFORM_WINDOWS )
			HWND mainWindow = NULL;
	#if defined( EAE6320_PLATFORM_D3D )
			uint16_t resolutionWidth = 0, resolutionHeight = 0;
	#elif defined( EAE6320_PLATFORM_GL )
			HINSTANCE thisInstanceOfTheApplication = NULL;
	#endif
#endif
		};

		cResult Initialize( const sInitializationParameters& i_initializationParameters );
		cResult CleanUp();
	}
}

#endif	// EAE6320_GRAPHICS_H
