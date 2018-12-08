#include "Rock.h"

namespace Simplex
{
	Rock::Rock(String a_sFileName, String a_sUniqueID, vector3 a_v3Velocity) : MyEntity(a_sFileName, a_sUniqueID)
	{
		// sets velocity
		m_v3Velocity = a_v3Velocity;
		fTimer = 0.0f;
	}
	Rock::~Rock()
	{
		Release();
	}

	// overriden IsColliding method for bullet
	bool Rock::IsColliding(MyEntity* const other)
	{
		//if not in memory return
		if (this == nullptr || other == nullptr)
			return false;

		if (!IsInMemory() || !other->IsInMemory())
			return true;

		return m_pRigidBody->IsColliding(other->GetRigidBody());
	}
}


