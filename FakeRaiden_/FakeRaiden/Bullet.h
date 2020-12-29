#pragma once

#include <Engine/Physics/GameObject.h>
#include <Engine/Graphics/cMesh.h>
#include <Engine/Graphics/cEffect.h>

namespace eae6320
{
	class Bullet : public GameObject {
	public:
		Bullet() = default;
		Bullet(Graphics::cMesh * p_Mesh, Graphics::cEffect * p_Effect) : GameObject(p_Mesh, p_Effect) {  }
		void OnTrigger(eae6320::Physics::sCollider* p_Other) override;
		void DeActivate();
	};
}