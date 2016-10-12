#pragma comment(lib, "..\\Debug\\Engine.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "alut.lib")
#pragma comment(lib, "OpenAL32.lib")

//-----------------------------------------------------------------------------
// System Includes
//-----------------------------------------------------------------------------
#include <windows.h>

//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#include "..\Engine\Engine.h"

//-----------------------------------------------------------------------------
// Test State Class
//-----------------------------------------------------------------------------
class TestState : public State
{
public:
	//-------------------------------------------------------------------------
	// Allows the test state to preform any pre-processing construction.
	//-------------------------------------------------------------------------
	virtual void Load()
	{
		m_font = new Font;

		m_sound = new Sound("./Assets/Sound.wav");
	}

	//-------------------------------------------------------------------------
	// Allows the test state to preform any post-processing destruction.
	//-------------------------------------------------------------------------
	virtual void Close()
	{
		SAFE_DELETE(m_font);

		SAFE_DELETE(m_sound);
	}

	//-------------------------------------------------------------------------
	// Returns the view setup details for the given frame.
	//----------------------------------------------- --------------------------
	virtual void RequestViewer(ViewerSetup *viewer)
	{
		viewer->viewClearFlags = D3DCLEAR_TARGET;
	}

	//-------------------------------------------------------------------------
	// Updates the state.
	//-------------------------------------------------------------------------
	virtual void Update(float elapsed)
	{
		if (g_engine->GetInput()->GetKeyPress(DIK_SPACE))
			m_sound->Play(false, 0);
	}

	//-------------------------------------------------------------------------
	// Renders the state.
	//-------------------------------------------------------------------------
	virtual void Render()
	{
		m_font->Render("Press the space bar to test the sound system.", 10, 10);
	}

private:
	Font *m_font; // A font used to render text.
	Sound *m_sound; // The test sound.
};

//-----------------------------------------------------------------------------
// Application specific state setup.
//-----------------------------------------------------------------------------
void StateSetup()
{
	g_engine->AddState(new TestState, true);
}

//-----------------------------------------------------------------------------
// Entry point for the application.
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmdLine, int cmdShow)
{
	// Create the engine setup structure.
	EngineSetup setup;
	setup.instance = instance;
	setup.name = "Sound Test";
	setup.StateSetup = StateSetup;

	// Create the engine (using the setup structure), then run it.
	new Engine(&setup);
	g_engine->Run();

	return true;
}