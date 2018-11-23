#include "AppClass.h"
using namespace Simplex;
void Application::InitVariables(void)
{
	//Set the position and target of the camera
	m_pCameraMngr->SetPositionTargetAndUpward(
		vector3(0.0f, 0.0f, 0.0f), //Position
		vector3(0.0f, 0.0f, -1.0f),	//Target
		AXIS_Y);					//Up

	m_pLightMngr->SetPosition(vector3(0.0f, 3.0f, 13.0f), 1); //set the position of first light (0 is reserved for ambient light)

#ifdef DEBUG
	uint uInstances = 9;
#else
	uint uInstances = 1849;
#endif
	int nSquare = static_cast<int>(std::sqrt(uInstances));
	m_uObjects = nSquare * nSquare;
	uint uIndex = -1;
	for (int i = 0; i < nSquare; i++)
	{
		for (int j = 0; j < nSquare; j++)
		{
			uIndex++;
			m_pEntityMngr->AddEntity("Minecraft\\cube.obj");
			vector3 v3Position = vector3(glm::sphericalRand(m_fSphereRadius));
			matrix4 m4Position = glm::translate(v3Position) * glm::scale(vector3(.5f));
			m_pEntityMngr->SetModelMatrix(m4Position);
		}
	}
	m_uOctantLevels = 1;
	m_pRoot = new MyOctant(m_uOctantLevels, 5);

	/// player set up
	m_pPlayer = new MyEntity("Planets\\00_Sun.obj", "player");
	///
	
	m_pEntityMngr->Update();
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the ArcBall active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();

	//Update Entity Manager
	m_pEntityMngr->Update();

	//check endgame conditions
	if (EndGameCheck()) {
		EndGame();
	}

	//Add objects to render list
	m_pEntityMngr->AddEntityToRenderList(-1, true);
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	

	///
	// handles player model positioning and basic collision
	// detection and resetting of player when colliding
	#pragma region PlayerAssignedCamPosition
	// sets base player model on camera
	// commented element used for creating object as crosshair for testing purposes
	vector3 pos = m_pCameraMngr->GetPosition();  // + m_pCameraMngr->GetForward() * 2;
	matrix4 m4pos = glm::translate(pos) * glm::scale(vector3(.5f));
	m_pPlayer->SetModelMatrix(m4pos);
	m_pPlayer->AddToRenderList();

	// resets position of camera to 0 vector if collided with debris, simulating death
	for (int x = 0; x < m_pEntityMngr->GetEntityCount(); x++)
	{
		if (m_pPlayer->GetRigidBody()->IsColliding(m_pEntityMngr->GetEntity(x)->GetRigidBody()))
		{
			m_pCameraMngr->SetPosition(vector3(0.f));
		}
	}
	#pragma endregion


	// uses timer system for percentage/lerp movement
	static float fTimer = 0;    //store the new timer
	static uint uClock = m_pSystem->GenClock(); //generate a new clock for that timer
	fTimer += m_pSystem->GetDeltaTime(uClock); //get the delta time for that timer

	// checks if there is a current bullet
	if (m_pBullet != nullptr)
	{
		// maps the values to be between 0 & 1
		float fPercentage = MapValue(fTimer, 0.0f, m_pBullet->m_fSpeed, 0.0f, 1.0f);

		// sets current position based on lerp
		vector3 v3CurrentPos = glm::lerp(m_pBullet->m_v3StartPos, m_pBullet->m_v3EndPos, fPercentage);
		matrix4 m4BulletModel = glm::translate(v3CurrentPos) * glm::scale(vector3(.4f));
		m_pBullet->SetModelMatrix(m4BulletModel);
		m_pBullet->AddToRenderList();

		// checks if bullet reaches its range
		// if so, deletes bullet and resets timer
		if (fPercentage > .99f)
		{
			SafeDelete(m_pBullet);
			m_pBullet = nullptr;
			fTimer = 0.f;
		}

		// checks for collisions and deletes bullet and entity it collides with
		for (int x = 0; x < m_pEntityMngr->GetEntityCount(); x++)
			if (m_pBullet->IsColliding(m_pEntityMngr->GetEntity(x)))
			{
				SafeDelete(m_pBullet);
				m_pBullet = nullptr;
				fTimer = 0.f;
				m_pEntityMngr->RemoveEntity(x);
			}

	}

	///

	if (m_bVisual)
	{
		//display octree
		if (m_uOctantID == -1)
			m_pRoot->Display();
		else
			m_pRoot->Display(m_uOctantID);
	}

	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();

	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();

	//draw gui,
	DrawGUI();

	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	//release GUI
	ShutdownGUI();
}