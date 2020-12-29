#pragma once

#include <Engine/Graphics/cMesh.h>
#include <Engine/Graphics/cEffect.h>
#include <Engine/Physics/sRigidBodyState.h>

namespace eae6320
{
	namespace Physics
	{
		struct sCollider;
	}
}

namespace eae6320 {
	class GameObject
	{
	public:
		GameObject() = default;
		GameObject(Graphics::cMesh* p_Mesh, Graphics::cEffect* p_Effect) :
			m_Mesh{ p_Mesh }, m_Effect{ p_Effect }{}
		~GameObject();

		void SetIsActive(bool p_IsActive) { m_IsActive = p_IsActive; }
		void ToggleIsActive() { m_IsActive = !m_IsActive; }
		inline bool GetIsActive() { return m_IsActive; }

		void SetEffect(Graphics::cEffect* p_Effect) { m_Effect = p_Effect; }
		void SetMesh(Graphics::cMesh* p_Mesh) { m_Mesh = p_Mesh; }

		// Set velocity
		void SetSpeed(Math::sVector p_Velocity);
		void AddForce(Math::sVector p_Force);

		void Update(float p_DeltaTime);

		// Submit active data
		void SubmitData(float p_DeltaTimeToExtrapolate);

		Physics::sRigidBodyState* GetRigidbody();
		void AttachCollider(eae6320::Physics::sCollider* i_collider);
		Physics::sCollider* GetCollider() { return m_Collider; }

		virtual void OnCollision(eae6320::Physics::sCollider* p_Other) {}
		virtual void OnTrigger(eae6320::Physics::sCollider* p_Other) {}

	protected:
		Graphics::cMesh*			m_Mesh = nullptr;
		Graphics::cEffect*			m_Effect = nullptr;
		Physics::sRigidBodyState	m_RigidBodyState;
		Physics::sCollider*			m_Collider = nullptr;
		bool m_IsActive = true;
		bool m_IsTriggered = false;
		Physics::sCollider* m_LastCollider = nullptr;
	};
}