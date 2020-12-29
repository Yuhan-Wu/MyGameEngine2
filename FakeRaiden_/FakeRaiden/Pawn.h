#pragma once

#include <Engine/Physics/GameObject.h>
#include <Engine/Graphics/cMesh.h>
#include <Engine/Graphics/cEffect.h>

namespace eae6320
{
	class Pawn : public GameObject {
	public:
		Pawn() = default;
		Pawn(Graphics::cMesh* p_Mesh, Graphics::cEffect* p_Effect) : GameObject(p_Mesh, p_Effect) {  }

		inline void SetDamageEachTime(int p_Damage) { m_Damage = p_Damage; }

		virtual void OnHPZero() {}

	protected:
		int m_HP = 100;
		int m_Damage = 50;
	};
}