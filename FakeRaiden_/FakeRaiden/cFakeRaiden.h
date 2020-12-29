/*
	This class is your specific game
*/

#ifndef EAE6320_CFakeRaiden_H
#define EAE6320_CFakeRaiden_H

// Includes
//=========
#include "Camera.h"

#include "Bullet.h"
#include "Player.h"
#include "Enemy.h"

#include <Engine/Application/iApplication.h>
#include <Engine/Results/Results.h>
#include <Engine/Graphics/Graphics.h>
#include <Engine/Physics/sRigidBodyState.h>
#include <Engine/Audio/Audio.h>
#include <Engine/Audio/cMusic.h>

#if defined( EAE6320_PLATFORM_WINDOWS )
	#include "Resource Files/Resource.h"
#endif

#define ENEMY_BULLET 2
#define ENEMY_COOL_DOWN 10000
#define BOSS_BULLET 6
#define BOSS_COOL_DOWN 8000

#define MAX_ROCKET_BULLET 5
#define ROCKET_COOL_DOWN 5000

#define BLINKING 1000
// Class Declaration
//==================

namespace eae6320
{
	class cFakeRaiden final : public Application::iApplication
	{

	public:
		bool GameEnd = false;
		Graphics::sColor original_background = Graphics::sColor(0.1f, 0.1f, 0.1f);
		bool isBlinking = false;
		int BlinkingTimer = 0;

		// Inherited Implementation
		//=========================
	private:

		// Configuration
		//--------------

#if defined( EAE6320_PLATFORM_WINDOWS )
		// The main window's name will be displayed as its caption (the text that is displayed in the title bar).
		// You can make it anything that you want, but please keep the platform name and debug configuration at the end
		// so that it's easy to tell at a glance what kind of build is running.
		const char* GetMainWindowName() const final
		{
			return "Yuhan's EAE6320 Fake Raiden"
				" -- "
#if defined( EAE6320_PLATFORM_D3D )
				"Direct3D"
#elif defined( EAE6320_PLATFORM_GL )
				"OpenGL"
#endif
#ifdef _DEBUG
				" -- Debug"
#endif
			;
		}
		// Window classes are almost always identified by name;
		// there is a unique "ATOM" associated with them,
		// but in practice Windows expects to use the class name as an identifier.
		// If you don't change the name below
		// your program could conceivably have problems if it were run at the same time on the same computer
		// as one of your classmate's.
		// You don't need to worry about this for our class,
		// but if you ever ship a real project using this code as a base you should set this to something unique
		// (a generated GUID would be fine since this string is never seen)
		const char* GetMainWindowClassName() const final { return "Yuhan's EAE6320 My Main Window Class"; }
		// The following three icons are provided:
		//	* IDI_EAEGAMEPAD
		//	* IDI_EAEALIEN
		//	* IDI_VSDEFAULT_LARGE / IDI_VSDEFAULT_SMALL
		// If you want to try creating your own a convenient website that will help is: http://icoconvert.com/
		const WORD* GetLargeIconId() const final { static constexpr WORD iconId_large = IDI_PENGUIN; return &iconId_large; }
		const WORD* GetSmallIconId() const final { static constexpr WORD iconId_small = IDI_PENGUIN; return &iconId_small; }
#endif

		// Run
		//----
		bool Paused = false;
		bool MusicPaused = false;
		float LastTime = 0.0f;

		// Round info
		int big_round = 1;
		int sub_round = 0;

		// Assets
		eae6320::Graphics::cEffect* effects[5] = { nullptr };
		eae6320::Graphics::cMesh* meshes[6] = { nullptr };

		// Game Objects
		Enemy plane[1];
		Bullet plane_bullet[6];
		Math::sVector plane_pos = Math::sVector(2, 3, 0);
		int active_plane = 0;
		int plane_timer = 3000;

		Enemy ufo[2];
		Bullet ufo_bullet[4];
		Math::sVector ufo_pos[2] = { Math::sVector(1, 1, 0), Math::sVector(-1, 1, 0) };
		int active_ufo = 0;
		int ufo_timer = 6000;

		Enemy car[2];
		Bullet car_bullet[4];
		int active_car = 0;
		Math::sVector car_pos[4] = { Math::sVector(-3, -3, 0), Math::sVector(-3, -2.5f,0),
									Math::sVector(-3, -2, 0), Math::sVector(-3, -1.5f, 0) };
		int car_timer = 9000;

		Player rocket[1];
		Bullet rocket_bullet[MAX_ROCKET_BULLET];
		int rocket_timer = 0;
		bool isShot = false;

		GameObject wall[4];

		// Camera
		Camera cameras[1];
		Camera active_camera;

		// Game end
		GameObject flag;

		// Audio
		eae6320::Audio::cMusic* music[2] = { nullptr };

		// Initialize round
		void InitializeSubRound();

		void UpdateBasedOnInput() final;
		void UpdateSimulationBasedOnInput() final;
		void UpdateSimulationBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate) final;

		void SubmitDataToBeRendered(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_sinceLastSimulationUpdate) final;

		// Initialize / Clean Up
		//----------------------

		cResult Initialize() final;
		cResult CleanUp() final;

	};
}

// Result Definitions
//===================

namespace eae6320
{
	namespace Results
	{
		namespace Application
		{
			// You can add specific results for your game here:
			//	* The System should always be Application
			//	* The __LINE__ macro is used to make sure that every result has a unique ID.
			//		That means, however, that all results _must_ be defined in this single file
			//		or else you could have two different ones with equal IDs.
			//	* Note that you can define multiple Success codes.
			//		This can be used if the caller may want to know more about how a function succeeded.
			constexpr cResult ExampleResult( IsFailure, eSystem::Application, __LINE__, Severity::Default );
		}
	}
}

#endif	// EAE6320_CFakeRaiden_H
