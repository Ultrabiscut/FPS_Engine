#include "main.h"

Game *g_game = NULL;

SceneObject *object = NULL;

Game::Game() : State(STATE_GAME)
{
	g_game = this;

	m_scoreBoardFont = NULL;
}

void Game::Load()
{
	ShowCursor(false);

	m_crosshair = g_engine->GetMaterialManager()->Add("Crosshair.dds.txt", "./Assets/");

	m_scoreBoardFont = new Font("Arial", 14, FW_BOLD);

	m_scoreBoardDeaths[0] = 0;
	m_scoreBoardFrags[0] = 0;
	m_scoreBoardNames[0] = 0;

	m_playerManager = new PlayerManager;
}

void Game::Close()
{
	ShowCursor(true);

	g_engine->GetNetwork()->Terminate();

	g_engine->GetSceneManager()->DestroyScene();

	//Destroy the score board font.
	SAFE_DELETE(m_playerManager);
	SAFE_DELETE(m_scoreBoardFont);



	//Destroy the crosshair material.
	g_engine->GetMaterialManager()->Remove(&m_crosshair);
}

//-----------------------------------------------------------------------------
// Returns the view setup details for the given frame.
//-----------------------------------------------------------------------------
void Game::RequestViewer(ViewerSetup *viewer)
{
	viewer->viewer = m_playerManager->GetViewingPlayer();
	viewer->viewClearFlags = D3DCLEAR_ZBUFFER;
}

void Game::Update(float elapsed)
{
	if (g_engine->GetInput()->GetKeyPress(DIK_R) == true)
	{
		m_playerManager->GetLocalPlayer()->SetEnabled(true);
		m_playerManager->SpawnLocalPlayer();
	}

	m_playerManager->Update(elapsed);


	// Scoreboard displayed if tab key pressed
	if (g_engine->GetInput()->GetKeyPress(DIK_TAB, true) == true)
	{
		sprintf(m_scoreBoardNames, "PLAYER\n");
		sprintf(m_scoreBoardFrags, "FRAGS\n");
		sprintf(m_scoreBoardDeaths, "DEATHS\n");

		// Add each player's details to the score board.
		PlayerObject *player = m_playerManager->GetNextPlayer(true);
		while (player != NULL)
		{
			strcat(m_scoreBoardNames, player->GetName());
			strcat(m_scoreBoardNames, "\n");

			sprintf(m_scoreBoardFrags, "%s%d", m_scoreBoardFrags, player->GetFrags());
			strcat(m_scoreBoardFrags, "\n");

			sprintf(m_scoreBoardDeaths, "%s%d", m_scoreBoardDeaths, player->GetDeaths());
			strcat(m_scoreBoardDeaths, "\n");

			player = m_playerManager->GetNextPlayer();
		}
	}

	// Check if the user wants to exit back to the menu.
	if (g_engine->GetInput()->GetKeyPress(DIK_ESCAPE))
		g_engine->ChangeState(STATE_MENU);
}

void Game::Render()
{
	// Make sure scene is loaded
	if (g_engine->GetSceneManager()->IsLoaded() == false)
		return;

	if (g_engine->GetInput()->GetKeyPress(DIK_TAB, true) == true)
	{
		m_scoreBoardFont->Render(m_scoreBoardNames, 20, 100, 0xffff7700);
		m_scoreBoardFont->Render(m_scoreBoardFrags, 180, 100, 0xffff7700);
		m_scoreBoardFont->Render(m_scoreBoardDeaths, 260, 100, 0xffff7700);
	}

	//Draw the local player's crosshair in the centre of the screen.
	g_engine->GetSprite()->Begin(D3DXSPRITE_ALPHABLEND);
	g_engine->GetSprite()->Draw(m_crosshair->GetTexture(),
		NULL, NULL,
		&D3DXVECTOR3(g_engine->GetDisplayMode()->Width / 2.0f - 15.0f,
		g_engine->GetDisplayMode()->Height / 2.0 - 15.0f,
		0.0f),
		0xFFFFFFFF);

	g_engine->GetSprite()->End();
}

//-----------------------------------------------------------------------------
// Handles the game specific network messages. Called by the network object.
//-----------------------------------------------------------------------------
void Game::HandleNetworkMessage(ReceivedMessage *msg)
{
	// Process the received messaged based on its type.
	switch (msg->msgid)
	{
	case MSGID_CREATE_PLAYER:
	{
		// Add the new player to the player manager and the scene.
		PlayerObject *object = m_playerManager->AddPlayer(g_engine->GetNetwork()->GetPlayer(msg->dpnid));
		g_engine->GetSceneManager()->AddObject(object);

		// Check if the new player is the host player.
		if (object->GetID() == g_engine->GetNetwork()->GetHostID())
		{

			// Load the scene from the host player's selection.
			g_engine->GetSceneManager()->LoadScene(((PlayerData*)g_engine->GetNetwork()->GetPlayer(msg->dpnid)->data)->map, "./Assets/Scenes/");

			// Allow the network to receive game specific messages.
			g_engine->GetNetwork()->SetReceiveAllowed(true);

		}
		break;
	}

	case MSGID_DESTROY_PLAYER:
	{
		break;
	}

	case MSGID_TERMINATE_SESSION:
	{
		// Switch to the menu state.
		g_engine->ChangeState(STATE_MENU);

		break;
	}
	}
}