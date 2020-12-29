#include "GameObject.h"

#include "Physics.h"
#include "sCollider.h"

#include <Engine/Graphics/Graphics.h>
#include <Engine/Graphics/ConstantBufferFormats.h>

namespace eae6320 {
	GameObject::~GameObject() {
		if (m_Collider != nullptr)
			Physics::DeregisterCollider(m_Collider);
	}

	void GameObject::SetSpeed(Math::sVector p_Velocity) {
		m_RigidBodyState.velocity = p_Velocity;
	}

	void GameObject::AddForce(Math::sVector p_Force) {
		// Mass = 1 by default
		m_RigidBodyState.acceleration = p_Force / 1;
	}

	void GameObject::Update(float p_DeltaTime) {
		// Update position
		m_RigidBodyState.Update(p_DeltaTime);
	}

	void GameObject::SubmitData(float p_DeltaTimeToExtrapolate) {
		if (m_IsTriggered) {
			m_IsTriggered = false;
		}
		else {
			m_LastCollider = nullptr;
		}
		if (m_IsActive) {
			if (m_Mesh && m_Effect) {
				Graphics::ConstantBufferFormats::sDrawCall drawcall;
				drawcall.g_transform_localToWorld = m_RigidBodyState.PredictFutureTransform(p_DeltaTimeToExtrapolate);
				Graphics::SubmitMeshEffectDrawCallPair(m_Effect, m_Mesh, drawcall);
			}
		}
	}

	eae6320::Physics::sRigidBodyState* GameObject::GetRigidbody()
	{
		return &m_RigidBodyState;
	}

	void GameObject::AttachCollider(eae6320::Physics::sCollider* i_collider)
	{
		m_Collider = i_collider;
		m_Collider->AttachToGameObject(this);
	}
}