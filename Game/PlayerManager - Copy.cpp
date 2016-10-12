
#include "main.h"
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// PlayerManager.h implementation.
// Refer to the PlayerManager.h interface for more details.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Player manager class constructor.
//-----------------------------------------------------------------------------
PlayerManager::PlayerManager()
{
	// Create the list of player objects.
	m_players = new LinkedList< PlayerObject >;
	m_viewingPlayer = NULL;

	// Clear all the local player's movement details.
	m_localMovement = false;
	m_localDrive = 0.0f;
	m_localStrafe = 0.0f;
	m_localFire = false;

	// Indicate that the local player needs to be spawned.
	m_spawnLocalPlayer = true;
	m_requestedSpawnPoint = false;
}

//-----------------------------------------------------------------------------
// Player manager class destructor.
//-----------------------------------------------------------------------------
PlayerManager::~PlayerManager()
{
	// Destroy the list of player objects.
	m_players->ClearPointers();
	SAFE_DELETE(m_players);
}

//-----------------------------------------------------------------------------
// Updates all of the players.
//-----------------------------------------------------------------------------
void PlayerManager::Update(float elapsed)
{
	//Ensure the scene is loaded in memory or exit. //Scene is managed by certain class (SceneManager)
	if (g_engine->GetSceneManager()->IsLoaded() == false)
		return;

	//Get a pointer (PlayerObject*) to the local player. //Remember where the local player is stored. He's the first player
	//if there's no local player then exit function
	PlayerObject *localPlayer = m_players->GetFirst();
	if (localPlayer == NULL)
		return;

	//Check if local player is dead and not spawning
	if (localPlayer->GetHealth() <= 0.0f && m_spawnLocalPlayer == false)
	{
		//if the left mouse button is pressed, (0), set spawnLocalPlayer to true
		if (g_engine->GetInput()->GetButtonPress(0) == true)
			m_spawnLocalPlayer = true;

		return;
	}

	//If the local player needs to be spawned and a spawn point hasn't already
	//been requested, then request a spawn point from the host.
	if (m_spawnLocalPlayer == true && m_requestedSpawnPoint == false)
	{
		//send a request for a spawn point from the host
		NetworkMessage rspm;
		rspm.msgid = MSGID_SPAWN_POINT_REQUEST;
		rspm.dpnid = g_engine->GetNetwork()->GetLocalID();
		g_engine->GetNetwork()->Send(&rspm, sizeof(NetworkMessage), g_engine->GetNetwork()->GetHostID());

		//indicate spawn point has been requested
		m_requestedSpawnPoint = true;
		return;
	}
	//else if the player needs to be spawned, but a spawn point has already
	//been requested. So just return and wait for the request response.
	else if (m_spawnLocalPlayer = true)
	{
		return;
	}

	//Calculate a delayed elapsed time, used for smoothing out view movement.
	static float delayedElapsed = 0.0f;
	delayedElapsed = delayedElapsed * 0.99 + elapsed * 0.01f;

	//Skip any further input if the local player is dying
	if (localPlayer->GetDying() == true)
		return;

	//It is impossible to send network messages for every mouse movement,
	//therefore the player's looking direction will be updated directly
	localPlayer->MouseLook(delayedElapsed, (float)g_engine->GetInput()->GetDeltaY(),
		(float)g_engine->GetInput()->GetDeltaX());

	//Send a periodic player look update to sync the other players.
	static unsigned long lookUpdate = timeGetTime();
	if (lookUpdate + 100 < timeGetTime() && localPlayer->GetEnabled() == true)
	{
		PlayerLookUpdateMsg plum;
		plum.msgid = MSGID_PLAYER_LOOK_UPDATE;
		plum.dpnid = localPlayer->GetID();
		plum.viewTilt = localPlayer->GetViewTilt();
		plum.rotationY = localPlayer->GetRotation().y;
		g_engine->GetNetwork()->Send(&plum, sizeof(PlayerLookUpdateMsg), DPNID_ALL_PLAYERS_GROUP, DPNSEND_NOLOOPBACK);

		lookUpdate = timeGetTime();
	}

	//Used for storing the local player's desired movement.
	float desiredDrive = 0.0f;
	float desiredStrafe = 0.0f;
	float desiredFire = false;

	//Check if the local player is trying to drive fowards or backwards.
	if (g_engine->GetInput()->GetKeyPress(DIK_W, true))
		desiredDrive = 1.0f;
	else if (g_engine->GetInput()->GetKeyPress(DIK_S, true))
		desiredDrive = -1.0f;

	//Check if the local player is trying to strafe left or right.
	if (g_engine->GetInput()->GetKeyPress(DIK_D, true))
		desiredStrafe = 1.0;
	else if (g_engine->GetInput()->GetKeyPress(DIK_A, true))
		desiredStrafe = -1.0;

	//Check if the local player is trying to fire their weapon
	desiredFire = g_engine->GetInput()->GetButtonPress(0, true);

	//Check if we need to drive the local player.
	if (m_localDrive != desiredDrive)
	{
		m_localDrive = desiredDrive;
		m_localMovement = true;
	}

	//Check if we need to strage the local player
	if (m_localStrafe != desiredStrafe)
	{
		m_localStrafe = desiredStrafe;
		m_localMovement = true;
	}

	//Check if we need to fire the local player's weapon.
	if (m_localFire != desiredFire)
	{
		m_localFire = desiredFire;
		m_localMovement = true;
	}

	//If the local player's movement changed or the move update timer expires,
	//update the local player across the network. Since the network message is
	//sent using DPNSEND_NOLOOPBACK, the local player will need to update too.
	static unsigned long moveUpdate = timeGetTime();
	if ((m_localMovement == true || moveUpdate + 200 < timeGetTime()) && localPlayer->GetEnabled() == true)
	{
		PlayerMoveUpdateMsg pmum;
		pmum.msgid = MSGID_PLAYER_MOVE_UPDATE;
		pmum.translation = localPlayer->GetTranslation();
		pmum.drive = m_localDrive;
		pmum.strafe = m_localStrafe;
		pmum.fire = m_localFire;
		g_engine->GetNetwork()->Send(&pmum, sizeof(PlayerMoveUpdateMsg), DPNID_ALL_PLAYERS_GROUP, DPNSEND_NOLOOPBACK);
		m_localMovement = false;

		localPlayer->SetDrive(m_localDrive);
		localPlayer->SetStrafe(m_localStrafe);
		localPlayer->SetFire(m_localFire);
		moveUpdate = timeGetTime();
	}

}

//-----------------------------------------------------------------------------
// Spawns the local player using the given spawn point.
//-----------------------------------------------------------------------------
void PlayerManager::SpawnLocalPlayer(long spawnPoint)
{
	//Ensure the spawn point request was successful.
	if (spawnPoint == -1)
	{
		//End the spawn point request so a new one can be attempted.
		m_requestedSpawnPoint = false;

		//Indicate that the local player needs to be spawned.
		m_spawnLocalPlayer = true;

		return;
	}

	//Send a message to spawn the local player at the given spawn point.
	//getSpawnPointByID() from scenemanager. spawnPoint is the spawn point id

	SpawnPlayerMsg spm;
	spm.msgid = MSGID_SPAWN_PLAYER;
	spm.dpnid = g_engine->GetNetwork()->GetLocalID();
	spm.translation = g_engine->GetSceneManager()->GetSpawnPointByID(spawnPoint)->GetTranslation();
	g_engine->GetNetwork()->Send(&spm, sizeof(SpawnPlayerMsg), DPNID_ALL_PLAYERS_GROUP);



}

//-----------------------------------------------------------------------------
// Spawns the given player at the given translation.
//-----------------------------------------------------------------------------
void PlayerManager::SpawnPlayer(DPNID dpnid, D3DXVECTOR3 translation)
{
	//Find the player to spawn.
	m_players->Iterate(true);
	while (m_players->Iterate())
		if (m_players->GetCurrent()->GetID() == dpnid)
			break;
	if (m_players->GetCurrent() == NULL)
		return;

	m_players->GetCurrent()->SetEnabled(true);
	m_players->GetCurrent()->SetVisible(true);

	m_players->GetCurrent()->SetDying(false);
	m_players->GetCurrent()->SetHealth(100.0f);

	//Play the idle animation twice to ensure that both animation tracks
	//contain the idle animation data. This will prevent players from
	//transitioning from the death animation when respawning.
	m_players->GetCurrent()->PlayAnimation(0, 0.0f);
	m_players->GetCurrent()->PlayAnimation(0, 0.0f);

	//Set the player's translation and rotation.
	m_players->GetCurrent()->SetTranslation(translation);
	m_players->GetCurrent()->MouseLook(0.0f, 0.0f, 0.0f, true);

	//Check if the local player was spawned.
	if (m_players->GetCurrent()->GetID() == g_engine->GetNetwork()->GetLocalID())
	{
		//Set local player as the viewing player.
		m_players->GetCurrent()->SetIsViewing(true);
		m_viewingPlayer = m_players->GetCurrent();

		//The request for a local player spawn point was successful.
		m_requestedSpawnPoint = false;

		//Indicate that the local player has been spawned.
		m_spawnLocalPlayer = false;
	}
	else
	{
		m_players->GetCurrent()->SetIsViewing(false);
	}

}

//-----------------------------------------------------------------------------
// Adds a new player.
//-----------------------------------------------------------------------------
PlayerObject *PlayerManager::AddPlayer(PlayerInfo *player)
{
	//Create the script for the player's character.
	Script *script = new Script(((PlayerData*)player->data)->character, "./Assets/Characters/");

	PlayerObject *object = m_players->Add(new PlayerObject(player, script));

	SAFE_DELETE(script);
	return object;
}

//-----------------------------------------------------------------------------
// Removes the specified player.
//-----------------------------------------------------------------------------
void PlayerManager::RemovePlayer(DPNID dpnid)
{
	PlayerObject *player = GetPlayer(dpnid);
	if (player != NULL)
		m_players->Remove(&player);
}

//-----------------------------------------------------------------------------
// Returns the local player.
//-----------------------------------------------------------------------------
PlayerObject *PlayerManager::GetLocalPlayer()
{
	return m_players->GetFirst();
}

//-----------------------------------------------------------------------------
// Returns the specified player.
//-----------------------------------------------------------------------------
PlayerObject *PlayerManager::GetPlayer(DPNID dpnid)
{
	m_players->Iterate(true);
	while (m_players->Iterate())
		if (m_players->GetCurrent()->GetID() == dpnid)
			return m_players->GetCurrent();

	return NULL;
}

//-----------------------------------------------------------------------------
// Returns the next iterated player from the player list.
//-----------------------------------------------------------------------------
PlayerObject *PlayerManager::GetNextPlayer(bool restart)
{
	m_players->Iterate(restart);
	if (restart == true)
		m_players->Iterate();

	return m_players->GetCurrent();
}

//-----------------------------------------------------------------------------
// Returns the currently viewing player.
//-----------------------------------------------------------------------------
PlayerObject *PlayerManager::GetViewingPlayer()
{
	return m_viewingPlayer;
}