#include "Player.h"

#include "Bullet.h"
#include "Enemy.h"
#include "cFakeRaiden.h"

#include <Engine/Physics/Physics.h>

namespace eae6320
{
	void Player::OnHPZero() {
		m_Context->GameEnd = true;
	}

	void Player::OnCollision(eae6320::Physics::sCollider* p_Other) {
		if ((typeid(*(p_Other->owner)) == typeid(Bullet) && p_Other->m_Type == Physics::ObjectType::EnemyBullet) || typeid(*(p_Other->owner)) == typeid(Enemy)) {
			m_HP -= m_Damage;
			m_Context->isBlinking = true;
			if (m_HP <= 0) {
				OnHPZero();
			}
		}
	}
}