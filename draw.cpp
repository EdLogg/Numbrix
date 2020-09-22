#include "stdafx.h"

int totalX[10] = { 304, 304, 304, 304, 304, 304, 224, 152, 80, 8 };
int totalY[10] = { 8, 80, 152, 224, 296, 376, 376, 376, 376, 376 };
int puzzleX[4] = { 8, 80, 152, 224 };
int puzzleY[4] = { 80, 152, 224, 296 };
RECT TextRect = { 0, 0, SCREEN_WIDTH, SCREEN_TOP };
RECT ValueRect;


void SetMsgTextRect(RECT &TextRect, int x, int y)
{
	TextRect.left = x;
	TextRect.right = x + TILE_WIDTH;
	TextRect.top = y + 10;
	TextRect.bottom = y + TILE_HEIGHT;
}


void DrawPuzzle(HWND hWnd, HDC hdc)
{
	char string[256];

	if (state == STATE_INVALID)
		return;

	HDC hdcMem = CreateCompatibleDC(hdc);
	HFONT hbmOld = (HFONT)SelectObject(hdc, hGameFont0);		// select new object and save the old
	
	// draw state information
	SetBkMode(hdc, TRANSPARENT);
	int oldColor = SetTextColor(hdc, RGB(0, 0, 0));
	switch (state)
	{
	case STATE_ENTER:
		DrawText(hdc, "Enter puzzle values.  Use the mouse or arrow keys to select entry.\nPress Return to start solving the puzzle.", -1, &TextRect, DT_CENTER | DT_VCENTER);
		break;
	case STATE_SOLVE:
		DrawText(hdc, "Press Alt+Space to solve the puzzle or press Space to make a single step.\nTo manually solve: select a tile, enter the value, then select elsewhere.", -1, &TextRect, DT_CENTER | DT_VCENTER);
		break;
	case STATE_DONE:
		sprintf(string, "Puzzle solved using %d steps!", s.nodes);
		DrawText(hdc, string, -1, &TextRect, DT_CENTER | DT_VCENTER);
		break;
	default:
		break;
	}

	// display puzzle
	for (int y = 0; y < s.puzzleHeight; y++)
	{
		for (int x = 0; x < s.puzzleWidth; x++)
		{
			if (state == STATE_ENTER)
			{
				if (s.puzzle[y][x].val < 0)
					continue;
				if (x == s.cursorX
				&&  y == s.cursorY)
					SelectObject(hdcMem, hBoardOn);
				else
					SelectObject(hdcMem, hBoard);
			}
			else  if (state == STATE_SOLVE)
			{
				if (s.puzzle[y][x].added					// just added via the step function
				||  (y == s.cursorY && x == s.cursorX))		// we are manually changing this tile
					SelectObject(hdcMem, hBoardOn);
				else 
					SelectObject(hdcMem, hBoard);
			}
			else  if (state == STATE_DONE)
			{
				SelectObject(hdcMem, hBoard);
			}
			int xx = xOffset + x * TILE_WIDTH;
			int yy = yOffset + y * TILE_HEIGHT;
			BitBlt(hdc, xx, yy, TILE_WIDTH, TILE_HEIGHT, hdcMem, 0, 0, SRCCOPY);
			if (s.puzzle[y][x].val > 0)
			{
				if (state == STATE_ENTER)
					SelectObject(hdc, hGameFont1);
				else if (state == STATE_SOLVE)
				{
					if ((y == s.cursorY && x == s.cursorX)
					|| s.SegmentEnd(s.puzzle[y][x].val))
						SelectObject(hdc, hGameFont1);
					else
						SelectObject(hdc, hGameFont2);
				}
				else
					SelectObject(hdc, hGameFont1);
				SetMsgTextRect(ValueRect, xx, yy);
				sprintf(string, "%d", s.puzzle[y][x].val);
				DrawText(hdc, string, -1, &ValueRect, DT_CENTER | DT_VCENTER);
			}
		}
	}
	SetTextColor(hdc, oldColor);
	SelectObject(hdcMem, hbmOld);										// restore the old object
	DeleteDC(hdcMem);													// delete 												
}
