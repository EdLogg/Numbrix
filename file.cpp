#include "stdafx.h"


//***********************************************************************
//	Open and save puzzle functions
//***********************************************************************
BOOL LoadTextFile(LPCTSTR pszFileName)
{
	FILE * file;

	file = fopen(pszFileName, "r");
	if (file == NULL)
		return false;
	int		version;
	int		c;
	if (fscanf(file, "%d,%d", &version, &state) != 2
	|| version != SAVE_FILE_VERSION)			// version has changed 
	{
		fclose(file);
		return false;
	}
	c = fscanf(file, "%d,%d,%d,%d,%d,%d",
		&s.puzzleWidth, &s.puzzleHeight, &s.puzzleMax, &s.cursorX, &s.cursorY, &s.nodes);
	if (c != 6)
	{
		fclose(file);
		return false;
	}
	int num = s.puzzleHeight * s.puzzleWidth;
	for (int n = 0; n <= num + 1; n++)
	{
		s.pos[n].x = s.pos[n].y = -1;
	}
	for (int y = 0; y < s.puzzleHeight; y++)
	{
		for (int x = 0; x < s.puzzleWidth; x++)
		{
			c = fscanf(file, "%d", &s.puzzle[y][x].val);
			s.puzzle[y][x].neighbors0 = 0;
			s.puzzle[y][x].neighbors = 0;
			s.puzzle[y][x].added = false;
			if (state == STATE_SOLVE			// do not set pos except when solving
			&&  s.puzzle[y][x].val > 0)
			{
				s.pos[s.puzzle[y][x].val].x = x;
				s.pos[s.puzzle[y][x].val].y = y;
			}
			if (c != 1)
			{
				fclose(file);
				return false;
			}
		}
	}
	fclose(file);
	return true;
}


BOOL SaveTextFile(LPCTSTR pszFileName)
{
	FILE * file;
	file = fopen(pszFileName, "w");
	if (file != NULL)
	{
		fprintf(file, "%d,%d\n", SAVE_FILE_VERSION, state);
		fprintf(file, "%d,%d,%d,%d,%d,%d\n",
			s.puzzleWidth, s.puzzleHeight, s.puzzleMax, s.cursorX, s.cursorY, s.nodes);
		for (int y = 0; y < s.puzzleHeight; y++)
		{
			for (int x = 0; x < s.puzzleWidth; x++)
			{
				fprintf(file, "%d\n", s.puzzle[y][x].val);
			}
		}
		fclose(file);
		return true;
	}
	return false;
}


bool DoFileLoad(HWND hWnd)
{
	OPENFILENAME ofn;
	char szFileName[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "txt";

	if (GetOpenFileName(&ofn))
	{
		HWND hEdit = GetDlgItem(hWnd, IDC_MAIN_EDIT);
		if (LoadTextFile(szFileName))
			return true;
		MessageBox(hWnd, "Unable to open load file!", "User input error", MB_ICONSTOP | MB_OK | MB_APPLMODAL);
		return false;								// we probably destroyed something
	}
	return true;									// nothing changed
}


bool DoFileSave(HWND hWnd)
{
	OPENFILENAME ofn;
	char szFileName[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = "txt";
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	if (GetSaveFileName(&ofn))
	{
		if (SaveTextFile(szFileName))
			return true;
		MessageBox(hWnd, "Unable to open save file!", "User input error", MB_ICONSTOP | MB_OK | MB_APPLMODAL);
	}
	return false;
}

