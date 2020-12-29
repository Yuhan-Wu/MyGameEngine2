#include "Enemy.h"

#include "Bullet.h"

#include <Engine/Physics/Physics.h>

#include <typeinfo>

namespace eae6320
{
	void Enemy::OnHPZero() {
		SetIsActive(false);
		m_RigidBodyState.SetVelocity(Math::sVector(0, 0, 0));
		Physics::DeregisterCollider(m_Collider);
		m_HP = 100;
	}

	void Enemy::OnTrigger(eae6320::Physics::sCollider* p_Other) {
		if (typeid(*(p_Other->owner)) == typeid(Bullet) && p_Other->m_Type == Physics::ObjectType::PlayerBullet) {
			m_IsTriggered = true;
			if (p_Other != m_LastCollider) {
				m_HP -= m_Damage;
				m_LastCollider = p_Other;
				if (m_HP <= 0) {
					OnHPZero();
				}
			}
		}
	}
}