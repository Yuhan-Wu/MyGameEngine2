#ifndef EAE6320_PHYSICS_SCOLLIDER_H
#define EAE6320_PHYSICS_SCOLLIDER_H

// Includes
//=========

#include "sRigidBodyState.h"
#include "GameObject.h"

#include <Engine/Results/Results.h>
#include <Engine/Physics/eColliderType.h>
#include <Engine/Math/sVector.h>

// Struct Declaration
//===================

namespace eae6320
{
	namespace Physics
	{
		enum ObjectType {
			PlayerBullet,
			EnemyBullet,
			Enemy,
			Player,
			WorldStatic,
			Pawn
		};

		struct sCollider
		{
			static eae6320::cResult CreateMeshInstanceFromPath(const char* i_path, sCollider*& o_ptr);

			void AttachToGameObject(GameObject* i_ptr); // TODO

			sRigidBodyState* body;
			GameObject* owner; // TODO

			eae6320::Math::sVector colliderData0;
			eae6320::Math::sVector colliderData1;
			float colliderData2;

			float bounciness = 1.f;

			eColliderType shape;
			bool isStatic;
			bool isTrigger;

			ObjectType m_Type = ObjectType::WorldStatic;
		};
	}
}

#endif	// EAE6320_PHYSICS_SCOLLIDER_H
