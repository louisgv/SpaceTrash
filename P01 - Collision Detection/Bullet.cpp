#include "Bullet.h"

namespace Simplex
{
	Bullet::Bullet(String a_sFileName, String a_sUniqueID, vector3 a_v3Velocity) : MyEntity(a_sFileName, a_sUniqueID)
	{
		// sets velocity
		m_v3Velocity = a_v3Velocity;
		m_fSpeed = 3;//glm::length(m_v3Velocity);
	}
	Bullet::~Bullet()
	{
		Release();
	}

	// overriden IsColliding method for bullet
	bool Bullet::IsColliding(MyEntity* const other)
	{
		//if not in memory return
		if (this == nullptr || other == nullptr)
			return false;

		if (!IsInMemory() || !other->IsInMemory())
			return true;

		return m_pRigidBody->IsColliding(other->GetRigidBody());
	}
}


