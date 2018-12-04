#include "AppClass.h"
using namespace Simplex;
void Application::InitVariables(void)
{
	//Set the position and target of the camera
	m_pCameraMngr->SetPositionTargetAndUpward(
		vector3(0.0f, 0.0f, 0.0f), //Position
		vector3(0.0f, 0.0f, -3.0f),	//Target
		AXIS_Y);					//Up

	m_pLightMngr->SetPosition(vector3(0.0f, 3.0f, 13.0f), 1); //set the position of first light (0 is reserved for ambient light)

#ifdef DEBUG
	uint uInstances = 300;
	m_uTimeLeft = uInstances * 100;
	m_planetCo = 5;
#else
	uint uInstances = 2000;
	m_uTimeLeft = 11000;
	m_planetCo = 100;
#endif

	//sound stuff
	m_soundBuffer.loadFromFile("..\\_Binary\\Data\\Audio\\Lazer02.wav");
	m_soundBufferTwo.loadFromFile("..\\_Binary\\Data\\Audio\\LowCrash.wav");
	m_sound.setBuffer(m_soundBuffer);
	m_soundTwo.setBuffer(m_soundBufferTwo);
	m_sound.setVolume(30.0f);
	m_soundTwo.setVolume(75.0f);

	m_uLives = 3;
	int nSquare = static_cast<int>(std::sqrt(uInstances));
	m_uObjects = nSquare * nSquare;
	uint uIndex = -1;
	//fTimer = 0;
	for (int i = 0; i < nSquare; i++)
	{
		for (int j = 0; j < nSquare; j++)
		{
			uIndex++;
			m_pEntityMngr->AddEntity("..\\_Binary\\Data\\MFBX\\Rock.fbx"); //load rock
			vector3 v3Position = vector3(glm::sphericalRand(m_fSphereRadius));

			//random rotation for asteroids 
			matrix4 m4Rotation = glm::rotate(IDENTITY_M4, glm::radians((float)rand()), glm::vec3(rand(),rand(),rand()));
			matrix4 m4Position = m4Rotation * glm::translate(v3Position) * glm::scale(vector3(.45f));
			m_pEntityMngr->SetModelMatrix(m4Position);
		}
	}

	/// player set up
	//m_pPlayer = new MyEntity("..\\_Binary\\Data\\MFBX\\SpaceShip.fbx", "Player");
	m_pPlayer = new MyEntity("..\\_Binary\\Data\\MOBJ\\Planets\\03A_Moon.obj", "Player");
	// m_pEntityMngr->AddEntity(m_pPlayer);
	///
	m_pPlanet = new MyEntity("..\\_Binary\\Data\\MOBJ\\Planets\\03_Earth.obj", "Planet");
	// m_pEntityMngr->AddEntity(m_pPlanet);

	m_uOctantLevels = 2;
	m_pRoot = new MyOctant(m_uOctantLevels, 5);

	m_pEntityMngr->Update();
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//update bullet stuff
	BulletShoot();

	//Is the ArcBall active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();

	//update object count
	m_uObjects = m_pEntityMngr->GetEntityCount();

	//Update Entity Manager
	//if there are no entities, dont update - prevent freeze
	if (m_pEntityMngr->GetEntityCount() != 0) {
		m_pEntityMngr->Update();
	}

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
#pragma region BoundsCheck
	if (m_pCameraMngr->GetPosition().x > m_pRoot->GetMaxGlobal().x + 1)
		m_pCameraMngr->SetPosition(vector3(m_pRoot->GetMinGlobal().x, m_pCameraMngr->GetPosition().y, m_pCameraMngr->GetPosition().z));
	if(m_pCameraMngr->GetPosition().x < m_pRoot->GetMinGlobal().x - 1)
		m_pCameraMngr->SetPosition(vector3(m_pRoot->GetMaxGlobal().x, m_pCameraMngr->GetPosition().y, m_pCameraMngr->GetPosition().z));

	if (m_pCameraMngr->GetPosition().y > m_pRoot->GetMaxGlobal().y + 1)
	{
		m_pCameraMngr->SetPosition(vector3(m_pCameraMngr->GetPosition().x, m_pRoot->GetMinGlobal().y, m_pCameraMngr->GetPosition().z));
		m_pCameraMngr->SetTarget(vector3(m_pCameraMngr->GetPosition().x - .3f, m_pCameraMngr->GetPosition().y + 2.7f, m_pCameraMngr->GetPosition().z));
	}
	if (m_pCameraMngr->GetPosition().y < m_pRoot->GetMinGlobal().y - 1)
	{
		m_pCameraMngr->SetPosition(vector3(m_pCameraMngr->GetPosition().x, m_pRoot->GetMaxGlobal().y, m_pCameraMngr->GetPosition().z));
		m_pCameraMngr->SetTarget(vector3(m_pCameraMngr->GetPosition().x - .3f, m_pCameraMngr->GetPosition().y - 2.7f, m_pCameraMngr->GetPosition().z));
	}
		
	if (m_pCameraMngr->GetPosition().z > m_pRoot->GetMaxGlobal().z + 1)
		m_pCameraMngr->SetPosition(vector3(m_pCameraMngr->GetPosition().x, m_pCameraMngr->GetPosition().y,  m_pRoot->GetMinGlobal().z));
	if (m_pCameraMngr->GetPosition().z < m_pRoot->GetMinGlobal().z - 1)
		m_pCameraMngr->SetPosition(vector3(m_pCameraMngr->GetPosition().x, m_pCameraMngr->GetPosition().y, m_pRoot->GetMaxGlobal().z));

#pragma endregion 

	// handles player model positioning and basic collision
	// detection and resetting of player when colliding
#pragma region PlayerAssignedCamPosition
	// sets base player model on camera
	// commented element used for creating object as crosshair for testing purposes
	vector3 pos = m_pCameraMngr->GetPosition() - m_pCameraMngr->GetForward()* .5f; 

	vector3 v3Direction = m_pCameraMngr->GetForward();

	matrix4 m4pos = glm::translate(pos) * glm::scale(vector3(.2f));

	m_pPlayer->SetModelMatrix(m4pos);
	m_pPlayer->AddToRenderList();

	// resets position of camera to 0 vector if collided with debris, simulating death
	for (uint x = 0; x < m_pEntityMngr->GetEntityCount(); x++)
	{
		if (m_pPlayer->GetRigidBody()->IsColliding(m_pEntityMngr->GetEntity(x)->GetRigidBody()))
		{
			//m_pCameraMngr->SetPosition(vector3(0.f));
			m_pCameraMngr->ResetCamera();
			m_soundTwo.play();
			m_uLives--;
		}
	}
#pragma endregion

	//planet position
	vector3 posP = vector3(m_pRoot->GetMaxGlobal().x / 2, m_pRoot->GetMaxGlobal().y / 2, m_uTimeLeft/m_planetCo + 80);
	matrix4 m4posP = glm::translate(posP) * glm::scale(vector3(35.0f));
	m_pPlanet->SetModelMatrix(m4posP);
	m_pPlanet->AddToRenderList();

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
	SafeDelete(m_pPlayer);
	SafeDelete(m_pPlanet);
	SafeDelete(m_pRoot);
	m_pBullet.clear();
	//release GUI
	ShutdownGUI();
}