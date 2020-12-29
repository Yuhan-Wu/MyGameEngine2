#ifndef EAE6320_PHYSICS_H
#define EAE6320_PHYSICS_H

// Includes
//=========

#include "sCollider.h"
#include <Engine/Physics/sRigidBodyState.h>
#include <Engine/Results/Results.h>

// Function Declarations
//===================

namespace eae6320
{
	namespace Physics
	{
		void CheckCollision();
		void RegisterCollider(sCollider* i_collider);
		void DeregisterCollider(sCollider* i_collider);
		void ChangeColliderIsStatic(sCollider* i_collider);
		cResult CleanUp();
	}
}

#endif	// EAE6320_PHYSICS_H