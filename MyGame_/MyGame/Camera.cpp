#include "Camera.h"

#include <Engine/Math/cMatrix_transformation.h>
#include <Engine/Math/Functions.h>
#include <Engine/Graphics/Graphics.h>

namespace eae6320 {
	Camera::Camera() {
		m_RigidBodyState.position = Math::sVector(0, 0, 10.f);
	}

	void Camera::SetSpeed(Math::sVector p_Velocity) {
		m_RigidBodyState.velocity = p_Velocity;
	}

	void Camera::SetRotation(float p_Rotation) {
		m_RigidBodyState.angularSpeed = p_Rotation;
	}

	void Camera::Update(float p_DeltaTime) {
		// Update position
		m_RigidBodyState.Update(p_DeltaTime);
	}

	void Camera::SubmitData(float p_DeltaTimeToExtrapolate) {
		Math::cMatrix_transformation localToWorld = m_RigidBodyState.PredictFutureTransform(p_DeltaTimeToExtrapolate);
		Math::cMatrix_transformation worldToCamera = Math::cMatrix_transformation::CreateWorldToCameraTransform(localToWorld);
		Math::cMatrix_transformation cameraToProjected = Math::cMatrix_transformation::CreateCameraToProjectedTransform_perspective(
			Math::ConvertDegreesToRadians(45.0f), 1.0f, 0.1f, 100.f
		);
		Graphics::SubmitCameraInfo(worldToCamera, cameraToProjected);
	}
}