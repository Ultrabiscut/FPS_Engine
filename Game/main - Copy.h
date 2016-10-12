#pragma once
#pragma comment(lib, "..\\Debug\\Engine.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "openal32.lib")
#pragma comment(lib, "alut.lib")

#include <windows.h>

#include "..\Engine\Engine.h"

#include "GameMaterial.h"
#include "PlayerObject.h"
#include "PlayerManager.h"
#include "resource.h"
#include "Menu.h"
#include "Game.h"

//------------------------------------------------------------------------
//Player Data Structure
//------------------------------------------------------------------------
struct PlayerData
{
	char character[MAX_PATH];
	char map[MAX_PATH];
};