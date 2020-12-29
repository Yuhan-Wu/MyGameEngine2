#pragma once

#include "Pawn.h"

namespace eae6320
{
	class Enemy : public Pawn
	{
	public:
		Enemy() = default;
		Enemy(Graphics::cMesh* p_Mesh, Graphics::cEffect* p_Effect) : Pawn(p_Mesh, p_Effect) {  }
		void OnTrigger(eae6320::Physics::sCollider* p_Other) override;

		void OnHPZero() override;
	};
}
