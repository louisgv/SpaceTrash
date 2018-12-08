#include "MyOctant.h"

namespace Simplex
{
	uint MyOctant::m_uOctantCount = 0;
	uint MyOctant::m_uMaxLevel = 0;
	uint MyOctant::m_uIdealEntityCount = 0;

	void MyOctant::ConstructTree(uint a_nMaxLevel)
	{
		// Only root can construct the tree
		if (m_uLevel != 0)
		{
			return;
		}

		m_uMaxLevel = a_nMaxLevel;

		m_uOctantCount = 1;

		m_EntityList.clear();

		KillBranches();
		m_lChild.clear();

		if (ContainsMoreThan(m_uIdealEntityCount))
		{
			Subdivide();
		}

		// Assign dimension from root
		AssignIDtoEntity();

		ConstructList();
	}

	void MyOctant::Subdivide(void)
	{
		// Return if max level reached or is already subbed
		if (m_uLevel >= m_uMaxLevel || !IsLeaf())
		{
			return;
		}


#pragma region Construct Child Octants
		m_uChildren = 8;

		float fSize = m_fSize / 2.f;
		float fHalfSize = fSize / 2.f;

		// Bottom Left Back Octant
		vector3 v3Center = m_v3Center - vector3(fHalfSize);
		m_pChild[0] = new MyOctant(v3Center, fSize);

		// Bottom Right Back Octant
		v3Center.x += fSize;
		m_pChild[1] = new MyOctant(v3Center, fSize);

		// Bottom Right Front Octant
		v3Center.z += fSize;
		m_pChild[2] = new MyOctant(v3Center, fSize);

		// Bottom Left Front Octant
		v3Center.x -= fSize;
		m_pChild[3] = new MyOctant(v3Center, fSize);

		// Top Left Front Octant
		v3Center.y += fSize;
		m_pChild[4] = new MyOctant(v3Center, fSize);

		// Top Left Back Octant
		v3Center.z -= fSize;
		m_pChild[5] = new MyOctant(v3Center, fSize);

		// Top Right Back Octant
		v3Center.x += fSize;
		m_pChild[6] = new MyOctant(v3Center, fSize);

		// Top Right Front Octant
		v3Center.z += fSize;
		m_pChild[7] = new MyOctant(v3Center, fSize);
#pragma endregion

		uint uChildLevel = m_uLevel + 1;

		// Set root and parent
		for (uint nIndex = 0; nIndex < 8; nIndex++)
		{
			MyOctant* pChild = m_pChild[nIndex];
			pChild->m_pRoot = m_pRoot;
			pChild->m_pParent = this;
			pChild->m_uLevel = uChildLevel;
			if (pChild->ContainsMoreThan(m_uIdealEntityCount))
			{
				pChild->Subdivide();
			}
		}
	}

	void MyOctant::Init(void)
	{
		m_uChildren = 0;

		m_fSize = 0.0f;

		m_uID = m_uOctantCount;
		m_uLevel = 0;

		m_v3Center = vector3();
		m_v3Min = vector3();
		m_v3Max = vector3();

		m_pMeshMngr = MeshManager::GetInstance();
		m_pEntityMngr = MyEntityManager::GetInstance();

		m_pRoot = nullptr;
		m_pParent = nullptr;
		for (uint n = 0; n < 8; n++)
		{
			m_pChild[n] = nullptr;
		}
	}

	MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
	{
		// Initialize
		Init();

		m_uOctantCount = 0;
		m_uMaxLevel = a_nMaxLevel;
		m_uIdealEntityCount = a_nIdealEntityCount;
		m_uID = m_uOctantCount;

		m_uOctantCount++;

		m_pRoot = this;
		m_lChild.clear();

		float fMax = FLT_MIN;

		// Find the max coordinate
		for (uint i = 1; i < m_pEntityMngr->GetEntityCount(); i++)
		{
			MyEntity* pEntity = m_pEntityMngr->GetEntity(i);
			MyRigidBody* pRigidBody = pEntity->GetRigidBody();
			vector3 pRigidBodyMaxGlobal = pRigidBody->GetMaxGlobal();

			for (uint i = 0; i < 3; i++)
			{
				if (fMax < pRigidBodyMaxGlobal[i])
				{
					fMax = pRigidBodyMaxGlobal[i];
				}
			}
		}

		m_fSize = fMax * 2.0f;
		m_v3Size = vector3(fMax * 2.f);

		m_v3Min = m_v3Center - m_v3Size / 2.0f;
		m_v3Max = m_v3Center + m_v3Size / 2.0f;

		ConstructTree(m_uMaxLevel);
	}

	MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
	{
		Init();
		m_v3Center = a_v3Center;
		m_fSize = a_fSize;
		m_v3Size = vector3(m_fSize);

		m_v3Min = m_v3Center - m_v3Size / 2.0f;
		m_v3Max = m_v3Center + m_v3Size / 2.0f;

		m_uOctantCount++;
	}

	MyOctant::MyOctant(MyOctant const & other)
	{
		// Swap the values with 'other' values
		m_uChildren = other.m_uChildren;
		m_v3Center = other.m_v3Center;
		m_v3Min = other.m_v3Min;
		m_v3Max = other.m_v3Max;
		m_v3Size = other.m_v3Size;

		m_fSize = other.m_fSize;
		m_uID = other.m_uID;
		m_uLevel = other.m_uLevel;
		m_pParent = other.m_pParent;

		m_pRoot = other.m_pRoot;
		m_lChild = other.m_lChild;

		m_pMeshMngr = MeshManager::GetInstance();
		m_pEntityMngr = MyEntityManager::GetInstance();

		for (uint i = 0; i < 8; i++)
		{
			m_pChild[i] = other.m_pChild[i];
		}
	}

	MyOctant & MyOctant::operator=(MyOctant const & other)
	{
		if (this != &other)
		{
			Release();
			Init();
			Swap(MyOctant(other));
		}
		return *this;
	}

	MyOctant::~MyOctant(void)
	{
		Release();
	}

	void MyOctant::Swap(MyOctant & other)
	{
		std::swap(m_uChildren, other.m_uChildren);

		std::swap(m_uID, other.m_uID);
		std::swap(m_pRoot, other.m_pRoot);
		std::swap(m_lChild, other.m_lChild);

		std::swap(m_v3Center, other.m_v3Center);
		std::swap(m_v3Min, other.m_v3Min);
		std::swap(m_v3Max, other.m_v3Max);
		std::swap(m_v3Size, other.m_v3Size);

		m_fSize = other.m_fSize;

		m_pMeshMngr = MeshManager::GetInstance();
		m_pEntityMngr = MyEntityManager::GetInstance();

		std::swap(m_uLevel, other.m_uLevel);
		std::swap(m_pParent, other.m_pParent);

		for (uint i = 0; i < 8; i++)
		{
			std::swap(m_pChild[i], other.m_pChild[i]);
		}
	}

	float MyOctant::GetSize(void)
	{
		return m_fSize;
	}

	vector3 MyOctant::GetCenterGlobal(void)
	{
		return m_v3Center;
	}

	vector3 MyOctant::GetMinGlobal(void)
	{
		return m_v3Min;
	}

	vector3 MyOctant::GetMaxGlobal(void)
	{
		return m_v3Max;
	}

	bool MyOctant::IsColliding(uint a_uRBIndex)
	{
		uint uEntities = m_pEntityMngr->GetEntityCount();

		// Check index out of bound
		if (uEntities < a_uRBIndex)
		{
			return false;
		}

		// Simple collision check
		MyEntity* pEntity = m_pEntityMngr->GetEntity(a_uRBIndex);
		MyRigidBody* pRigidBody = pEntity->GetRigidBody();

		vector3 v3MinG = pRigidBody->GetMinGlobal();
		vector3 v3MaxG = pRigidBody->GetMaxGlobal();

		for (uint i = 0; i < 3; i++)
		{
			if (m_v3Max[i] < v3MinG[i] || m_v3Min[i] > v3MaxG[i]) {
				return false;
			}
		}

		return true;
	}

	void MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
	{
		if (m_uID == a_nIndex)
		{
			m_pMeshMngr->AddWireCubeToRenderList(
				glm::translate(m_v3Center) * glm::scale(m_v3Size),
				a_v3Color,
				RENDER_WIRE
			);

			return;
		}

		for (uint nIndex = 0; nIndex < m_uChildren; nIndex++)
		{
			m_pChild[nIndex]->Display(a_nIndex);
		}
	}

	void MyOctant::Display(vector3 a_v3Color)
	{
		m_pMeshMngr->AddWireCubeToRenderList(
			glm::translate(m_v3Center) * glm::scale(m_v3Size),
			a_v3Color,
			RENDER_WIRE
		);

		for (uint i = 0; i < m_uChildren; i++)
		{
			m_pChild[i]->Display(a_v3Color);
		}
	}

	void MyOctant::DisplayLeafs(vector3 a_v3Color)
	{
		m_pMeshMngr->AddWireCubeToRenderList(
			glm::translate(m_v3Center) * glm::scale(m_v3Size),
			a_v3Color,
			RENDER_WIRE
		);

		for (auto &child : m_lChild)
		{
			child->DisplayLeafs(a_v3Color);
		}
	}

	void MyOctant::ClearEntityList(void)
	{
		for (auto &child : m_lChild)
		{
			child->ClearEntityList();
		}

		m_EntityList.clear();
		m_uChildren = 0;
	}

	MyOctant * MyOctant::GetChild(uint a_nChild)
	{
		if (a_nChild >= 8 || a_nChild < 0)
		{
			return nullptr;
		}
		return m_pChild[a_nChild];
	}

	MyOctant * MyOctant::GetParent(void)
	{
		return m_pParent;
	}

	bool MyOctant::IsLeaf(void)
	{
		return m_uChildren == 0;
	}

	bool MyOctant::ContainsMoreThan(uint a_nEntities)
	{
		uint uCount = 0;
		uint uObjectCount = m_pEntityMngr->GetEntityCount();
	
		// Searching for all colliding objects within this octant
		for (uint i = 0; i < uObjectCount; i++)
		{
			if (IsColliding(i))
			{
				uCount++;
			}
			if (uCount > a_nEntities)
			{
				return true;
			}
		}
		return uCount >= a_nEntities;
	}

	void MyOctant::KillBranches(void)
	{
		if (IsLeaf()) {
			return;
		}

		for (uint i = 0; i < m_uChildren; i++)
		{
			m_pChild[i]->KillBranches();
			delete m_pChild[i];
			m_pChild[i] = nullptr;
		}

		m_uChildren = 0;
	}

	void MyOctant::AssignIDtoEntity(void)
	{
		if (IsLeaf())
		{
			uint uEntityCount = m_pEntityMngr->GetEntityCount();

			// Assign dimension to only entity inside the box
			for (uint i = 0; i < uEntityCount; i++)
			{
				if (IsColliding(i))
				{
					m_EntityList.push_back(i);
					m_pEntityMngr->AddDimension(i, m_uID);
				}
			}
			return;
		}

		for (uint i = 0; i < m_uChildren; i++)
		{
			m_pChild[i]->AssignIDtoEntity();
		}

	}

	uint MyOctant::GetOctantCount(void)
	{
		return m_uOctantCount;
	}

	void MyOctant::Release(void)
	{
		if (m_uLevel == 0)
		{
			KillBranches();
		}
		m_EntityList.clear();
		m_lChild.clear();
		Init();
	}

	void MyOctant::ConstructList(void)
	{
		for (uint i = 0; i < m_uChildren; i++)
		{
			m_pChild[i]->ConstructList();
		}

		if (m_EntityList.size() > 0)
		{
			m_pRoot->m_lChild.push_back(this);
		}
	}
}
