#pragma once

#include "Pawn.h"

namespace eae6320
{
	class cFakeRaiden;

	class Player : public Pawn
	{
	public:
		Player() = default;
		Player(Graphics::cMesh * p_Mesh, Graphics::cEffect * p_Effect) : Pawn(p_Mesh, p_Effect) {  }
		void OnCollision(eae6320::Physics::sCollider* p_Other) override;
		inline void SetContext(cFakeRaiden* p_Context) { m_Context = p_Context; }

		void OnHPZero() override;

	private:
		cFakeRaiden* m_Context = nullptr;
	};
}
