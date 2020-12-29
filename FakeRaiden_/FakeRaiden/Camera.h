#pragma once

#include <Engine/Physics/sRigidBodyState.h>

namespace eae6320 {
	class Camera
	{
	public:
		Camera();

		// Set velocity
		void SetSpeed(Math::sVector p_Velocity);
		void SetRotation(float p_Rotation);

		void Update(float p_DeltaTime);

		// Submit active data
		void SubmitData(float p_DeltaTimeToExtrapolate);

	private:
		Physics::sRigidBodyState	m_RigidBodyState;
	};
}