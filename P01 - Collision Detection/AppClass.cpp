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
	uint uInstances = 100;
	m_uTimeLeft = 2000;
	// m_fSphereRadius = 10.f;
#else
	uint uInstances = 1800;
#endif
	int nSquare = static_cast<int>(std::sqrt(uInstances));
	m_uObjects = nSquare * nSquare;
	uint uIndex = -1;
	fTimer = 0;
	for (int i = 0; i < nSquare; i++)
	{
		for (int j = 0; j < nSquare; j++)
		{
			uIndex++;
			m_pEntityMngr->AddEntity("..\\_Binary\\Data\\MFBX\\Rock.fbx"); //load rock
			vector3 v3Position = vector3(glm::sphericalRand(m_fSphereRadius));

			//random rotation for asteroids 
			matrix4 m4Rotation = glm::rotate(IDENTITY_M4, glm::radians((float)rand()), glm::vec3(rand(),rand(),rand()));
			matrix4 m4Position = m4Rotation * glm::translate(v3Position) * glm::scale(vector3(.5f));
			m_pEntityMngr->SetModelMatrix(m4Position);
		}
	}
	m_uOctantLevels = 2;
	m_pRoot = new MyOctant(m_uOctantLevels, 5);

	/// player set up
	m_pPlayer = new MyEntity("..\\_Binary\\Data\\MFBX\\SpaceShip.fbx", "Player");
	///
	
	m_pEntityMngr->Update();
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	BulletShoot();

	//Is the ArcBall active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();

	//Update Entity Manager

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
	// handles player model positioning and basic collision
	// detection and resetting of player when colliding
	#pragma region PlayerAssignedCamPosition
	// sets base player model on camera
	// commented element used for creating object as crosshair for testing purposes
	vector3 pos = m_pCameraMngr->GetPosition() + m_pCameraMngr->GetForward() * 2 - m_pCameraMngr->GetUpward() * .5f; 

	vector3 v3Direction = m_pCameraMngr->GetForward();

	// + m_pCameraMngr->GetForward() * 2;

	//rotation attempt - have to fix/clean up
	float fAngle = dot(AXIS_X, m_pCameraMngr->GetForward());
	quaternion qRotation = glm::angleAxis(acos(fAngle), cross(AXIS_X, m_pCameraMngr->GetForward()));

	matrix4 m4pos = glm::translate(pos) * ToMatrix4(qRotation) * glm::scale(vector3(.2f));

	m_pPlayer->SetModelMatrix(m4pos);
	m_pPlayer->AddToRenderList();

	// resets position of camera to 0 vector if collided with debris, simulating death
	for (uint x = 0; x < m_pEntityMngr->GetEntityCount(); x++)
	{
		if (m_pPlayer->GetRigidBody()->IsColliding(m_pEntityMngr->GetEntity(x)->GetRigidBody()))
		{
			m_pCameraMngr->SetPosition(vector3(0.f));
		}
	}
	#pragma endregion

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