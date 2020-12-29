#include "Bullet.h"

#include <Engine/Physics/Physics.h>

#include <typeinfo>

namespace eae6320
{
	void Bullet::OnTrigger(eae6320::Physics::sCollider* p_Other) {
		if(typeid(*(p_Other->owner)) != typeid(Bullet))
			if(p_Other->m_Type == Physics::ObjectType::WorldStatic || 
			(p_Other->m_Type == Physics::ObjectType::Enemy && m_Collider->m_Type == Physics::ObjectType::PlayerBullet) || 
			(p_Other->m_Type == Physics::ObjectType::Player && m_Collider->m_Type == Physics::ObjectType::EnemyBullet))
				DeActivate();
	}

	void Bullet::DeActivate() {
		GetRigidbody()->SetVelocity(Math::sVector(0, 0, 0));
		Physics::DeregisterCollider(GetCollider());
		ToggleIsActive();
	}
}