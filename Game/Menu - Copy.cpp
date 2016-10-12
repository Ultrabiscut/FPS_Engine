#include "Main.h"

void UpdateSessionList(HWND window)
{
	//Enumerate the current sessions on the network.
	g_engine->GetNetwork()->EnumerateSessions();

	//Try to keep the same session selected, if it still exists.
	SessionInfo *selectedSession = NULL;
	int selected = (int)SendMessage(GetDlgItem(window, IDC_SESSIONS), LB_GETCURSEL, 0, 0);
	if (selected != LB_ERR)//List box error
	{
		selectedSession = (SessionInfo*)SendMessage(GetDlgItem(window, IDC_SESSIONS), LB_GETITEMDATA, selected, 0);
	}

	SendMessage(GetDlgItem(window, IDC_SESSIONS), WM_SETREDRAW, false, 0);

	SendMessage(GetDlgItem(window, IDC_SESSIONS), LB_RESETCONTENT, 0, 0);

	//Go through the list of sessions found on the local network.
	char name[MAX_PATH];
	SessionInfo *session = g_engine->GetNetwork()->GetNextSession(true);

	while (session != NULL)
	{
		//Convert this session's name into a character string.
		wcstombs(name, session->description.pwszSessionName, MAX_PATH);

		// Add Session to the listbox
		int index = (int)SendMessage(GetDlgItem(window, IDC_SESSIONS), LB_ADDSTRING, 0, (LPARAM)name);
		SendMessage(GetDlgItem(window, IDC_SESSIONS), LB_SETITEMDATA, index, (LPARAM)session);

		if (selectedSession == session)
		{
			SendMessage(GetDlgItem(window, IDC_SESSIONS), LB_SETCURSEL, index, 0);
		}

		session = g_engine->GetNetwork()->GetNextSession();

	}

	// If no selected seession, select the first
	if (selectedSession == NULL)
	{
		SendMessage(GetDlgItem(window, IDC_SESSIONS), LB_SETCURSEL, 0, 0);
	}

	SendMessage(GetDlgItem(window, IDC_SESSIONS), WM_SETREDRAW, 0, 0);
	InvalidateRect(GetDlgItem(window, IDC_SESSIONS), NULL, false);
}

int CALLBACK MenuDialogProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//Theses are used to keep the text in the edit boxes between state changes.
	static char name[MAX_PATH] = "Unknown Player";
	static char character[MAX_PATH] = "Marine.txt";
	static char map[MAX_PATH] = "Abandoned City.txt";

	switch (msg)
	{
	case WM_INITDIALOG:
	{
		//Show the player's name, selected character, and selected map.
		SetWindowText(GetDlgItem(window, IDC_NAME), name);
		SetWindowText(GetDlgItem(window, IDC_CHARACTER), character);
		SetWindowText(GetDlgItem(window, IDC_MAP), map);

		UpdateSessionList(window);

		return true;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wparam))
		{
		case IDC_HOST:
		{
			PlayerData data;
			GetWindowText(GetDlgItem(window, IDC_CHARACTER), character, MAX_PATH);
			GetWindowText(GetDlgItem(window, IDC_MAP), map, MAX_PATH);
			strcpy(data.character, character);
			strcpy(data.map, map);

			GetWindowText(GetDlgItem(window, IDC_NAME), name, MAX_PATH);

			//Create a session name using the player's name.
			char session[MAX_PATH];

			sprintf(session, "%s's Session", name);

			if (g_engine->GetNetwork()->Host(name, session, 8, &data, sizeof(data)))
			{
				g_engine->ChangeState(STATE_GAME);

				EndDialog(window, true); // closing the window without exiting program
			}

			return true;
		}
		case IDC_REFRESH:
		{
			UpdateSessionList(window);
			return true;
		}
		case IDC_EXIT:
		{
			PostQuitMessage(0);
			return true;
		}

		case IDC_SESSIONS:
		{
			if (HIWORD(wparam) != LBN_DBLCLK)
				return true;
		} // if so then fall through to join
		case IDC_JOIN:
		{
			PlayerData data;
			GetWindowText(GetDlgItem(window, IDC_CHARACTER), character, MAX_PATH);
			strcpy(data.character, character);

			GetWindowText(GetDlgItem(window, IDC_NAME), name, MAX_PATH);

			int session = (int)SendMessage(GetDlgItem(window, IDC_SESSIONS), LB_GETCURSEL, 0, 0);

			if (g_engine->GetNetwork()->Join(name, session, &data, sizeof(data)))
			{
				g_engine->ChangeState(STATE_GAME);
				EndDialog(window, true);
			}
			else
			{
				//If the join failed, it may be because the session
				//doesn't exist anymore, so refresh the session list.
				UpdateSessionList(window);
			}
			return true;
			//GetMessage goes and comes back
			//SendMessage sends message 
			//LB_GetCurrentSelection
		}
		}
	}
	}
	return false;
}
//1. Handler to window
//2. Message to itself
//3. WPARAM
//4. LPARAM

Menu::Menu() : State(STATE_MENU)
{

}

void Menu::Update(float elapsed)
{
	//Display the menu dialog. Processing will hang here until this is closed.
	DialogBox(NULL, MAKEINTRESOURCE(IDD_MENU), g_engine->GetWindow(), MenuDialogProc);
}

