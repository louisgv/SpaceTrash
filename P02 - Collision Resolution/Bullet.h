
#include "MyEntity.h"

namespace Simplex
{
	class Bullet : public MyEntity
	{
	public:
		Bullet(String a_sFileName, String a_sUniqueID, vector3 a_v3Velocity);
		vector3 m_v3Velocity;
		float m_fSpeed = 1.f;
		vector3 m_v3StartPos;
		vector3 m_v3EndPos;
		float m_fRange = 10.f;
		float fTimer = 0.0f;
		~Bullet();
		bool IsColliding(MyEntity* const other);
	};
}

