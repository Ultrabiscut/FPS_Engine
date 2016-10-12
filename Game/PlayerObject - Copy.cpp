#include "main.h"


PlayerObject::PlayerObject(PlayerInfo *player, Script *script, unsigned long type) :
AnimatedObject(script->GetStringData("mesh_name"), script->GetStringData("mesh_path"), type)
{
	// Set the player's DirectPlay ID.
	m_dpnid = player->dpnid;

	// Set the players's name.
	m_name = new char[strlen(player->name) + 1];
	strcpy(m_name, player->name);

	// Players start with full health;
	m_health = 100.0f;
	m_dying = false;

	// Indicate that the view transform is not being taken from this player.
	m_isViewing = false;

	// Clear the player's score.
	m_frags = 0;
	m_deaths = 0;

	// Player objects start off invisible and disabled.
	SetVisible(false);
	SetEnabled(false);

	// Clear the player's input.
	m_drive = 0.0f;
	m_strafe = 0.0f;
	m_fire = false;

	// Set the correct ellipse radius.
	SetEllipsoidRadius(*script->GetVectorData("ellipse_radius"));

	// Level the player's view tilt.
	m_viewTilt = 0.0f;

	// Set the default view smoothing and sensitivity.
	m_viewSmoothing = 0.5f;
	m_viewSensitivity = 0.65f;

	// Create the callback data used for tracking the player's foot steps.
	m_callbackData[0].foot = 0;
	m_callbackData[1].foot = 1;

	// Create the callback keys. The second key time is set per animation.
	D3DXKEY_CALLBACK keys[2];
	keys[0].Time = 0;
	keys[0].pCallbackData = &m_callbackData[0];
	keys[1].pCallbackData = &m_callbackData[1];

	LPD3DXKEYFRAMEDANIMATIONSET oldAS;
	LPD3DXCOMPRESSEDANIMATIONSET newAS;
	LPD3DXBUFFER buffer;
	//Go through the four movement animations and set the foot step keys.
	for (char a = 1; a < 5; a++)
	{
		//Get the old animation.
		GetAnimationController()->GetAnimationSet(a, (LPD3DXANIMATIONSET*)&oldAS);

		//Set the time for the second key
		keys[1].Time = float(oldAS->GetPeriod() / 2.0f * oldAS->GetSourceTicksPerSecond());

		//Compress the old animation set.
		oldAS->Compress(D3DXCOMPRESS_DEFAULT, 0.4f, NULL, &buffer);

		//Create the new animation using the old one and the foot step keys.
		D3DXCreateCompressedAnimationSet(oldAS->GetName(),
			oldAS->GetSourceTicksPerSecond(),
			oldAS->GetPlaybackType(),
			buffer,
			2,
			keys,
			&newAS);

		SAFE_RELEASE(buffer);

		//Unregister the old animation set.
		GetAnimationController()->UnregisterAnimationSet(oldAS);
		SAFE_RELEASE(oldAS);

		//Register the new animation set.
		//Note: The new animation is appended to the end of the list.
		GetAnimationController()->RegisterAnimationSet(newAS);
		SAFE_RELEASE(newAS);

	}

	//Play the idle animation.
	PlayAnimation(0, 0.0f);

	//Create tyhe step sound audio paths
	//m_leftStepAudioPath = new AudioPath3D;
	//m_rightStepAudioPath = new AudioPath3D;

	//Set the friction on this object.
	SetFriction(8.0f);

}


PlayerObject::~PlayerObject()
{
	//Destroy the string buffer containing the player's name.
	SAFE_DELETE(m_name);
}

void PlayerObject::Update(float elapsed, bool addVelocity)
{
	//Allows the base class to update
	AnimatedObject::Update(elapsed, addVelocity);

	//Override the object's foward vector to take the view tilt into account.
	//This will allow the foward vector to move up and down as well instead
	//of just remaining horizontal. This is not important for movement since
	//the player can not fly, but for things like shooting it is.
	m_forward.x = (float)sin(GetRotation().y);
	m_forward.y = (float)-tan(m_viewTilt);
	m_forward.z = (float)cos(GetRotation().y);

	D3DXVec3Normalize(&m_forward, &m_forward);

	//Set the player's view point. This is done every frame because as the
	//mesh is animated, the reference point in the mesh may move. This
	//will allow the view point to move with the mesh's animations.
	m_viewPoint = GetMesh()->GetReferencePoint("rp_view_point")->GetTranslation();

	//Ensure that the view movement is relative to the rotation
	D3DXVec3TransformCoord(&m_viewPoint, &m_viewPoint, GetRotationMatrix());

	//Only calculate the correct view matrix if it is being used.
	if (m_isViewing == true)
	{
		//Create the x axis rotation matrix.
		D3DXMATRIX rotationXMatrix;
		D3DXMatrixRotationX(&rotationXMatrix, m_viewTilt);

		//Create the combined rotation matrix (i.e. y axis rotation from the 
		//scene object plus the x axis rotation from the player object).
		D3DXMATRIX combinedRotation;
		D3DXMatrixMultiply(&combinedRotation, &rotationXMatrix, GetRotationMatrix());

		//Build a translation matrix that represents the final view point.
		D3DXMATRIX viewPointTranslationMatrix;
		D3DXVECTOR3 finalViewPointTranslation = GetTranslation() + m_viewPoint;
		D3DXMatrixTranslation(&viewPointTranslationMatrix, finalViewPointTranslation.x,
			finalViewPointTranslation.y,
			finalViewPointTranslation.z);

		//Override the object's view matrix using the combined rotation and
		//the position of the final view point translation.
		D3DXMatrixMultiply(&m_viewMatrix, &combinedRotation, &viewPointTranslationMatrix);
		D3DXMatrixInverse(&m_viewMatrix, NULL, &m_viewMatrix);
	}

	//Ignore the rest if the player is dying
	if (m_dying) return;

	//drive and strafe the player accordingly
	if (m_drive != 0.0f)
		Drive(m_drive * 8000.0f * elapsed);
	if (m_strafe != 0.0f)
		Strafe(m_strafe * 4000.0f * elapsed); //side to side is always slower than front/back

	//play audio
	// Update the step audio paths.
	//m_leftStepAudioPath->SetPosition(GetTranslation() + GetMesh()->GetReferencePoint("rp_left_foot")->GetTranslation());
	//m_leftStepAudioPath->SetVelocity(GetVelocity());
	//m_rightStepAudioPath->SetPosition(GetTranslation() + GetMesh()->GetReferencePoint("rp_right_foot")->GetTranslation());
	//m_rightStepAudioPath->SetVelocity(GetVelocity());
}

//----------------------------------------------------------------------------
//Render the player object
//----------------------------------------------------------------------------
void PlayerObject::Render(D3DXMATRIX *world)
{
	//Allow the base animated object to render.
	if (m_dpnid != g_engine->GetNetwork()->GetLocalID())
		AnimatedObject::Render(world);
	else if (m_dying == true)
		return;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void PlayerObject::CollisionOccurred(SceneObject *object, unsigned long collisionStamp)
{
	//Ignore collisions if the player is dying (or dead)
	if (m_dying == true)
		return;

	//Allow the base scene object to register the collision/
	SceneObject::CollisionOccurred(object, collisionStamp);
}

//----------------------------------------------------------------------------
//Rotates the player's view
//----------------------------------------------------------------------------
void PlayerObject::MouseLook(float elapsed, float x, float y, bool reset)
{
	static float lastX = 0.0f;
	static float lastY = 0.0f;

	//check if players view must be reset
	if (reset == true)
	{
		lastX = lastY = 0.0f;
		SetRotation(0.0f, 0.0f, 0.0f);
		m_viewTilt = 0.0f;
		return;
	}

	//Calculate the real x and y values by accounting for smoothing.
	lastX = lastX	* m_viewSmoothing + (x * elapsed) * (1.0f - m_viewSmoothing);
	lastY = lastY	* m_viewSmoothing + (y * elapsed) * (1.0f - m_viewSmoothing);

	//Adjust the values for sensitivity
	float sensitivity = 1.0f / elapsed * m_viewSensitivity / 100.0f;
	if (sensitivity > 4.0f)
		sensitivity = 4.0f;
	if (sensitivity < m_viewSensitivity)
		sensitivity = m_viewSensitivity;
	sensitivity = sensitivity / 4.0f;
	lastX *= sensitivity;
	lastY *= sensitivity;

	AddRotation(0.0f, lastY, 0.0f);

	//Ensure the view will not rotate too far up or down
	if ((m_viewTilt > 0.8f && lastX > 0.0f) || (m_viewTilt < -0.8f && lastX < 0.0f))
		lastX = 0.0f;

	m_viewTilt += lastX;
}

void PlayerObject::Kill()
{
	//what should happen
	//indicate that the player is dying
	m_dying = true;

	//Clear the player's movement.
	SetDrive(0.0f);
	SetStrafe(0.0f);
	SetFire(false);
	Stop();

	//Play the death animation
	PlayAnimation(ANIM_DEATH, 0.0f, true);

}

//-----------------------------------------------------------------------------
// Returns the player object's DirectPlay ID number.
//-----------------------------------------------------------------------------
DPNID PlayerObject::GetID()
{
	return m_dpnid;
}

//-----------------------------------------------------------------------------
// Returns the player object's name.
//-----------------------------------------------------------------------------
char *PlayerObject::GetName()
{
	return m_name;
}

//-----------------------------------------------------------------------------
// Sets the player object's health.
//-----------------------------------------------------------------------------
void PlayerObject::SetHealth(float health)
{
	m_health = health;
}

//-----------------------------------------------------------------------------
// Returns the player object's health.
//-----------------------------------------------------------------------------
float PlayerObject::GetHealth()
{
	return m_health;
}

//-----------------------------------------------------------------------------
// Sets the player object's dying flag.
//-----------------------------------------------------------------------------
void PlayerObject::SetDying(bool dying)
{
	m_dying = dying;
}

//-----------------------------------------------------------------------------
// Returns the player object's dying flag.
//-----------------------------------------------------------------------------
bool PlayerObject::GetDying()
{
	return m_dying;
}

//-----------------------------------------------------------------------------
// Sets the player object's viewing flag.
//-----------------------------------------------------------------------------
void PlayerObject::SetIsViewing(bool isViewing)
{
	m_isViewing = isViewing;
}

//-----------------------------------------------------------------------------
// Sets the player object's frag count.
//-----------------------------------------------------------------------------
void PlayerObject::SetFrags(unsigned long frags)
{
	m_frags = frags;
}

//-----------------------------------------------------------------------------
// Returns the player object's frag count.
//-----------------------------------------------------------------------------
unsigned long PlayerObject::GetFrags()
{
	return m_frags;
}

//-----------------------------------------------------------------------------
// Sets the player object's death count.
//-----------------------------------------------------------------------------
void PlayerObject::SetDeaths(unsigned long deaths)
{
	m_deaths = deaths;
}

//-----------------------------------------------------------------------------
// Returns the player object's death count.
//-----------------------------------------------------------------------------
unsigned long PlayerObject::GetDeaths()
{
	return m_deaths;
}

//-----------------------------------------------------------------------------
// Sets the player object's drive.
//-----------------------------------------------------------------------------
void PlayerObject::SetDrive(float drive)
{
	if (m_dying = true)
		return;
	if (drive == 0.0f)
	{
		if (m_drive != 0.0f && m_strafe == 0.0f)
			PlayAnimation(ANIM_IDLE, 0.2f);
	}
	else if (m_drive == 0.0f && m_strafe == 0.0f)
	{
		if (drive == 1.0f)
			PlayAnimation(ANIM_FORWARDS, 0.2f);
		else
			PlayAnimation(ANIM_BACKWARDS, 0.2f);
	}

	m_drive = drive;
}

//-----------------------------------------------------------------------------
// Returns the player object's current drive.
//-----------------------------------------------------------------------------
float PlayerObject::GetDrive()
{
	return m_drive;
}

//-----------------------------------------------------------------------------
// Sets the player object's strafe.
//-----------------------------------------------------------------------------
void PlayerObject::SetStrafe(float strafe)
{
	if (m_dying = true)
		return;
	if (strafe == 0.0f)
	{
		if (m_drive == 0.0f && m_strafe != 0.0f)
			PlayAnimation(ANIM_IDLE, 0.2f);
	}
	else if (m_drive == 0.0f && m_strafe == 0.0f)
	{
		if (strafe == 1.0f)
			PlayAnimation(ANIM_RIGHT, 0.2f);
		else
			PlayAnimation(ANIM_LEFT, 0.2f);
	}

	m_strafe = strafe;
}

//-----------------------------------------------------------------------------
// Returns the player object's current strafe.
//-----------------------------------------------------------------------------
float PlayerObject::GetStrafe()
{
	return m_strafe;
}

//-----------------------------------------------------------------------------
// Sets the player object's fire flag.
//-----------------------------------------------------------------------------
void PlayerObject::SetFire(bool fire)
{
	// Ignore if the player is dying (or dead)
	if (m_dying == true)
		return;

	m_fire = fire;
}

//-----------------------------------------------------------------------------
// Returns the player object's fire flag.
//-----------------------------------------------------------------------------
bool PlayerObject::GetFire()
{
	return m_fire;
}

//-----------------------------------------------------------------------------
// Sets the player object's view tilt (i.e. the rotation around the x axis).
//-----------------------------------------------------------------------------
void PlayerObject::SetViewTilt(float tilt)
{
	m_viewTilt = tilt;
}

//-----------------------------------------------------------------------------
// Returns the player object's view tilt (i.e. the rotation around the x axis).
//-----------------------------------------------------------------------------
float PlayerObject::GetViewTilt()
{
	return m_viewTilt;
}

//-----------------------------------------------------------------------------
// Returns the player object's eye point.
//-----------------------------------------------------------------------------
D3DXVECTOR3 PlayerObject::GetEyePoint()
{
	return GetTranslation() + m_viewPoint;
}

//-----------------------------------------------------------------------------
// Returns the player object's eye point.
//-----------------------------------------------------------------------------
HRESULT CALLBACK PlayerObject::HandleCallback(THIS_ UINT Track, LPVOID pCallbackData)
{
	//is player touching ground

	//which foot?
	//left... check the ground to see what material they are stepping on and play sound
	//right do the same

	//Get a pointer to the callback data.
	AnimationCallbackData *data = (AnimationCallbackData*)pCallbackData;

	//If the player is not touching the ground, then it can't make foot steps.
	if (IsTouchingGround() == false)
		return S_OK;

	//Check which foot caused the callback.
	if (data->foot == 1) //left foot
	{
		//Reset the step result.
		m_stepResult.material = NULL;

		//Perform a ray intersection for the left foot. If it is successful,
		//then play the material's step sound.
		if (g_engine->GetSceneManager()->RayIntersectScene(&m_stepResult,
			GetTranslation() + GetMesh()->GetReferencePoint("rp_left_foot")->GetTranslation(),
			D3DXVECTOR3(0.0f, -1.0f, 0.0f)) == true)
		{
			//if (((GameMaterial*)m_stepResult.material)->GetStepSound() != NULL)
			//{
			//	//m_leftStepAudioPath->Play(((GameMaterial*)m_stepResult.material)->GetStepSound()->GetSegment());
			//}
		}
	}
	else
	{
		//Reset the step result.
		m_stepResult.material = NULL;

		//Perform a ray intersection for the left foot. If it is successful,
		//then play the material's step sound.
		if (g_engine->GetSceneManager()->RayIntersectScene(&m_stepResult,
			GetTranslation() + GetMesh()->GetReferencePoint("rp_right_foot")->GetTranslation(),
			D3DXVECTOR3(0.0f, -1.0f, 0.0f)) == true)
		{
			//if (((GameMaterial*)m_stepResult.material)->GetStepSound() != NULL)
			//{
			//	//m_leftStepAudioPath->Play(((GameMaterial*)m_stepResult.material)->GetStepSound()->GetSegment());
			//}
		}
	}
	return S_OK;
}