// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include "resource.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>								// qsort and others
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <crtdbg.h>								// used for _CrtDumpMemoryLeaks()
#include <commdlg.h>							// required for load and save files
#include <stdio.h>								// required for load and save files

#include "resource.h"

#define SHOW_SEARCH			false				// show search for solutions

#define SAVE_FILE_VERSION	1					// version of load/save files
#define MAX_WIDTH			12					// max puzzle dimensions to fit the screen
#define MAX_HEIGHT			12
#define TILE_WIDTH			72					// size of piece of the puzzle
#define TILE_HEIGHT			72
#define SCREEN_TOP			48
#define SCREEN_WIDTH		(MAX_WIDTH * TILE_WIDTH)
#define SCREEN_HEIGHT		(MAX_HEIGHT * TILE_HEIGHT + SCREEN_TOP)

#define MAX_DXDY			4

#define	MAX_AREAS			32					// max number of areas to count
#define MAX_SEGMENTENDS		32					// max number in an area

extern int SortEnds(const void * e1, const void * e2);


enum STATE
{
	STATE_INVALID = 0,
	STATE_ENTER,
	STATE_SOLVE,
	STATE_DONE
};

struct Node
{
	int		val;
	int		neighbors0;								// mask of neighbors with unused entries
	int		neighbors;								// mask of valid neighbors
	bool	added;									// if this was just added with step algorithm
	// used to see if a partial solution is valid
	int		area;									// area number (-1 if not tested or if value is present)
	int		segmentEnd;								// -1=not checked, 0=not segment end, 1= segment end
	int		used;									// 0 if unused, -1 if used multiple times, else value used once
};

struct Position
{
	int		x, y;
};

struct Area
{
	int		numOpen;								// number of open tiles
	int		numEnds;								// number of segment ends below
	int		ends[MAX_SEGMENTENDS];					// values of segment ends
};


//------------------------------------------------------------------------------
class hSolver
{
public:
	//	max = last numbr in the puzzle
	//	dx,dy are the offsets for the 8 neighbors
	//	puzzle[][] is the array of nodes (.val=-1 is off the board, 0 if unknown)
	//	given[][] = true for those numbers that were given
	Node	puzzle[MAX_HEIGHT][MAX_WIDTH];			// puzzle with -1 not valid, 0 for unknown, else the real value
	Position pos[MAX_HEIGHT * MAX_WIDTH + 2];		// position of number else x = -1
	int		puzzleWidth;							// actual width and height of puzzle 
	int		puzzleHeight;
	int		puzzleMax;								// number of entries in the puzzle
	int		cursorX, cursorY;						// where we are inputing values in the puzzle if any
	int		nodes;									// nodes searched
	int		solutionCount;							// count of solutions found
	int		solutionCommon[MAX_HEIGHT][MAX_WIDTH];	// common values accross all solutions

	hSolver()
	{
		dx[0] = 0; dx[1] = -1; dx[2] = 1; dx[3] = 0; 
		dy[0] = -1; dy[1] = 0; dy[2] = 0; dy[3] = 1; 
		ClearPuzzle();								// be sure to set pos[0] and pos[82]
	}

	void ClearPuzzle()
	{
		int n = 0;
		pos[n].x = pos[n].y = -1;					// this is needed to find solutions from 0 to N
		n++;
		for (int y = 0; y < puzzleHeight; y++)
		{
			for (int x = 0; x < puzzleWidth; x++)
			{
				puzzle[y][x].val = 0;
				puzzle[y][x].neighbors0 = 0;
				puzzle[y][x].neighbors = 0;
				puzzle[y][x].added = false;
				pos[n].x = pos[n].y = -1;
				n++;
			}
		}
		pos[n].x = pos[n].y = -1;					// this is needed to find solutions from N to 82
	}

	bool PuzzleSolved()
	{
		int x, y;
		for (int n = 1; n <= puzzleMax; n++)
		{
			if (pos[n].x < 0)
				return false;
			if (n > 1)
			{
				x = pos[n - 1].x - pos[n].x;
				y = pos[n - 1].y - pos[n].y;
				if (x < -1 || x > 1 || y < -1 || y > 1)
					return false;
				if (x != 0 && y != 0)					// no diagonals
					return false;
			}
			if (n < puzzleMax)
			{
				x = pos[n + 1].x - pos[n].x;
				y = pos[n + 1].y - pos[n].y;
				if (x < -1 || x > 1 || y < -1 || y > 1)
					return false;
				if (x != 0 && y != 0)					// no diagonlals
					return false;
			}
		}
		return true;
	}

	void ClearAdded()
	{
		for (int y = 0; y < puzzleHeight; y++)
		{
			for (int x = 0; x < puzzleWidth; x++)
			{
				puzzle[y][x].added = false;
			}
		}
	}

	void FlipNumbers()
	{
		for (int n = 0; n <= puzzleMax + 1; n++)
		{
			pos[n].x = pos[n].y = -1;
		}
		for (int y = 0; y < puzzleHeight; y++)
		{
			for (int x = 0; x < puzzleWidth; x++)
			{
				if (puzzle[y][x].val > 0)
				{
					puzzle[y][x].val = puzzleMax + 1 - puzzle[y][x].val;
					pos[puzzle[y][x].val].x = x;
					pos[puzzle[y][x].val].y = y;
				}
			}
		}
	}
	
	// set cursor to first entry
	void InitCursor()
	{
		for (int y = 0; y < puzzleHeight; y++)
		{
			for (int x = 0; x < puzzleWidth; x++)
			{
				if (puzzle[y][x].val == 0)
				{
					cursorX = x;
					cursorY = y;
					return;
				}
			}
		}
	}

	int CountPositions()
	{
		int count = 0;
		for (int y = 0; y < puzzleHeight; y++)
		{
			for (int x = 0; x < puzzleWidth; x++)
			{
				if (puzzle[y][x].val != 0)
					count++;
			}
		}
		return count;
	}

	void MoveUp()
	{
		int y = cursorY;
		do
		{
			if (--cursorY < 0)
				cursorY = puzzleHeight - 1;
			if (y == cursorY)						// no empty tile
			{
				if (--cursorY < 0)
					cursorY = puzzleHeight - 1;
				break;
			}
		} while (puzzle[cursorY][cursorX].val < 0);
	}

	void MoveDown()
	{
		int y = cursorY;
		do
		{
			if (++cursorY >= puzzleHeight)
				cursorY = 0;
			if (y == cursorY)						// no empty tile
			{
				if (++cursorY >= puzzleHeight)
					cursorY = 0;
				break;
			}
		} while (puzzle[cursorY][cursorX].val < 0);
	}

	void MoveLeft()
	{
		int x = cursorX;
		do
		{
			if (--cursorX < 0)
				cursorX = puzzleWidth - 1;
			if (x == cursorX)						// no empty tile
			{
				if (--cursorX < 0)
					cursorX = puzzleWidth - 1;
				break;
			}
		} while (puzzle[cursorY][cursorX].val < 0);
	}

	void MoveRight()
	{
		int x = cursorX;
		do
		{
			if (++cursorX >= puzzleWidth)
				cursorX = 0;
			if (x == cursorX)						// no empty tile
			{
				if (++cursorX >= puzzleWidth)
					cursorX = 0;
				break;
			}
		} while (puzzle[cursorY][cursorX].val < 0);
	}

	void SetMax()
	{
		puzzleMax = puzzleWidth * puzzleHeight;
	}

	bool SetPositions()
	{
		bool ret = true;
		for (int y = 0; y < puzzleHeight; y++)
		{
			for (int x = 0; x < puzzleWidth; x++)
			{
				if (puzzle[y][x].val > 0)
				{
					if (pos[puzzle[y][x].val].x >= 0)
					{
						puzzle[y][x].val = 0;		// clear dulpicate entry
						ret = false;
						continue;
					}
					pos[puzzle[y][x].val].x = x;
					pos[puzzle[y][x].val].y = y;
				}
			}
		}
		return ret;
	}

	void ClearPositions()
	{
		for (int n = 0; n <= puzzleMax + 1; n++)
		{
			pos[n].x = pos[n].y = -1;
		}
	}

	bool ClearCurrentPos()
	{
		puzzle[cursorY][cursorX].val = 0;
		for (int i = 1; i < puzzleMax; i++)
		{
			if (pos[i].x == cursorX && pos[i].y == cursorY)
			{
				pos[i].x = pos[i].y = -1;
				return true;
			}
		}
		return false;
	}

	bool SegmentEnd(int val)
	{
		if (val != 1
		&& pos[val - 1].x < 0)
			return true;
		if (val != puzzleMax
		&& pos[val + 1].x < 0)
			return true;
		return false;
	}

	// start with the first number
	void FindStart(int & x, int & y)
	{
		for (y = 0; y < puzzleHeight; y++)
			for (x = 0; x < puzzleWidth; x++)
				if (puzzle[y][x].val != 0)
				{
					return;
				}
		x = y = -1;
	}

	bool SolvePuzzle(bool countSolutions)
	{
		int x, y;
		FindStart(x, y);
		if (x < 0)										// must be a blank puzzle
			return false;
#if SHOW_SEARCH
		file = fopen("solutionSearch.txt", "w");
#endif
		solutionCount = 0;
		Search(x, y, puzzle[y][x].val + 1, puzzle[y][x].val, 1, countSolutions);
#if SHOW_SEARCH
		fclose(file);
#endif
		return solutionCount > 0;
	}

	bool FindObviousStep()								// if any tile has just one play make it 
	{
		for (int n = 1; n <= puzzleMax; n++)
		{
			if (pos[n].x < 0)
				continue;
			Node * node = &puzzle[pos[n].y][pos[n].x];
			int x, y;
			if (n > 1
			&& pos[n - 1].x < 0)
			{
				if (GetNeighbors(node, pos[n].x, pos[n].y) == 1)
				{
					if (node->neighbors0 == 1)
					{
						x = pos[n].x;
						y = pos[n].y - 1;
					}
					else if (node->neighbors0 == 2)
					{
						x = pos[n].x - 1;
						y = pos[n].y;
					}
					else if (node->neighbors0 == 4)
					{
						x = pos[n].x + 1;
						y = pos[n].y;
					}
					else 
					{
						x = pos[n].x;
						y = pos[n].y + 1;
					}
					pos[n - 1].x = x;
					pos[n - 1].y = y;
					puzzle[y][x].val = n - 1;
					puzzle[y][x].added = true;
					return true;
				}
			}
			if (n < puzzleMax
			&& pos[n + 1].x < 0)
			{
				if (GetNeighbors(node, pos[n].x, pos[n].y) == 1)
				{
					if (node->neighbors0 == 1)
					{
						x = pos[n].x;
						y = pos[n].y - 1;
					}
					else if (node->neighbors0 == 2)
					{
						x = pos[n].x - 1;
						y = pos[n].y;
					}
					else if (node->neighbors0 == 4)
					{
						x = pos[n].x + 1;
						y = pos[n].y;
					}
					else 
					{
						x = pos[n].x;
						y = pos[n].y + 1;
					}
					pos[n + 1].x = x;
					pos[n + 1].y = y;
					puzzle[y][x].val = n + 1;
					puzzle[y][x].added = true;
					return true;
				}
			}
		}
		return false;
	}

	// look for the smallest differences between digits
	bool FindNextStep(HWND hWnd)
	{
		int diffs[MAX_HEIGHT * MAX_WIDTH + 1];
		int largest = 1, smallest = puzzleMax;
		int unused = 0;
		diffs[0] = 0;
		for (int y = 0; y < puzzleHeight; y++)
		{
			for (int x = 0; x < puzzleWidth; x++)
			{
				puzzle[y][x].used = 0;
			}
		}
		for (int n1 = 1; n1 < puzzleMax; )
		{
			while (pos[n1].x < 0)						// skip 1 if it is not present
			{
				diffs[n1] = 0;
				unused++;
				n1++;
			}
			for (int n2 = n1 + 1; n2 <= puzzleMax; n2++)
			{
				if (pos[n2].x >= 0)						// this is known
				{
					int diff = n2 - n1;					// how many are in between
					diffs[n1] = diff;					// this is the difference between the numbers
					if (diff > 1)						// look at these
					{
						if (smallest > diff)
							smallest = diff;
						if (largest < diff)
							largest = diff;
					}
					n1 = n2;
					break;
				}
				diffs[n2] = 0;							// ignore holes
				if (n2 == puzzleMax)					// no end point 
				{
					int diff = n2 - n1 + 1;
					diffs[n1] = diff;
					if (smallest > diff)
						smallest = diff;
					if (largest < diff)
						largest = diff;
					n1++;
				}
			}
		}
		for (int diff = smallest; diff <= largest; diff++)
		{
			int n = 1;
			while (diffs[n] == 0)						// this assumes there is at least one number
				n++;
			for (; n < puzzleMax; n += diffs[n])
			{
				if (diffs[n] == diff)
				{
					// OK now look for obvious solutions
					if (FindSolutions(n, n + diff))		// if we found one stop
						return true;
				}
			}
		}
		// we already got the N to 82 above
		// but it is possible we only have open tiles from 0 to N 
		int start = 1;
		while (pos[start].x < 0)
			start++;
		if (pos[1].x < 0
		&& FindSolutions(start, 0))
			return true;
		// Rule 5: see if there is only one path that touched some location
		int updated = 0;
		for (int y = 0; y < puzzleHeight; y++)
		{
			for (int x = 0; x < puzzleWidth; x++)
			{
				if (puzzle[y][x].used > 0)			// only used for one value
				{
					int val = puzzle[y][x].used;
					puzzle[y][x].val = val;
					puzzle[y][x].added = true;
					pos[val].x = x;
					pos[val].y = y;
					updated++;
				}
			}
		}
		if (updated > 0)							// Rule 5 used here
			return true;
		// try looking at all solutions and see if there is anything in common
		SolvePuzzle(true);
		if (solutionCount == 0)
		{
			MessageBox(hWnd, "There are no solutions!", "Error", MB_OK | MB_ICONEXCLAMATION);
			return false;
		}
		int count = 0;
		for (int y = 0; y < puzzleHeight; y++)
		{
			for (int x = 0; x < puzzleWidth; x++)
			{
				int val = solutionCommon[y][x];
				if (val == 0
					|| val == puzzle[y][x].val)
					continue;
				count++;
				pos[val].x = x;
				pos[val].y = y;
				puzzle[y][x].val = val;
				puzzle[y][x].added = true;
			}
		}
		if (count > 0)
		{
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			MessageBox(hWnd, "I had to brute force all solutions!", "Very Interesting Situation", MB_OK | MB_ICONEXCLAMATION);
			return true;
		}
		// OK nothing will work
		MessageBox(hWnd, "There are several solutions to this puzzle!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

private:
	int		dx[MAX_DXDY], dy[MAX_DXDY];
	Position choices[MAX_HEIGHT * MAX_WIDTH];		// used for finding obvious choices
	Position solution[MAX_HEIGHT * MAX_WIDTH];		// current solution we found	
	int		numChoices;								// number of valid entries in choices[]
	Area	areas[MAX_AREAS];						// contiguous area info
	int		numAreas;								// number of areas we found
	FILE *	file;									// debug info

	// This is the recursive solver
	// (x,y) is the current position
	// next is the next number we are trying to find
	// start is the next number we started at
	// dir is the direction (+1 or -1) we are going
	// we stop when we hit 0 with dir=-1 
	// if countSolutions is true we try to find all solutions
	// and save any common values
	// This returns the solution count (0 or 1 if countSolutions is false)
	bool Search(int x, int y, int next, int start, int dir, bool countSolutions)
	{
RETRY:	if (next > puzzleMax)
		{
			dir = -1;
			next = start - 1;
			x = pos[start].x;
			y = pos[start].y;
		}
		if (next == 0 && dir == -1)
		{
			if (countSolutions)
			{
				for (int y = 0; y < puzzleHeight; y++)
				{
					for (int x = 0; x < puzzleWidth; x++)
					{
						if (solutionCount == 0)
							solutionCommon[y][x] = puzzle[y][x].val;
						else if (solutionCommon[y][x] != puzzle[y][x].val)
							solutionCommon[y][x] = 0;
					}
				}
			}
			solutionCount++;
			return true;
		}
		if (pos[next].x >= 0)							// we already know where this number is
		{
			int x1 = pos[next].x;
			int y1 = pos[next].y;
			if (x == x1
			&& (y == y1 - 1 || y == y1 + 1))
			{
				x = x1;
				y = y1;
				next += dir;
				goto RETRY;
			}
			if (y == y1
			&& (x == x1 - 1 || x == x1 + 1))
			{
				x = x1;
				y = y1;
				next += dir;
				goto RETRY;
			}
#if SHOW_SEARCH
			for (int y = 0; y < 9; y++)
			{
				for (int x = 0; x < 9; x++)
				{
					fprintf(file, "%2d,", puzzle[y][x].val);
				}
				fprintf(file, "\n");
			}
			fprintf(file, "\n\n");
#endif
			return false;								// the numbers are not adjacent
		}
		nodes++;
		Node * n = &puzzle[y][x];
		if (x == 1 && y == 1)
			x = 1;
		GetNeighbors(n, x, y);
		int neighbors = n->neighbors0;					// this may get clobbered so save it
		// recursively try different places for the next number w
		for (int d = 0; d < MAX_DXDY; d++)
		{
			if ((neighbors & (1 << d)) != 0)
			{
				int a = x + dx[d];
				int b = y + dy[d];
				puzzle[b][a].val = next;
				if (next == 68)
					next = 68;
				if (Search(a, b, next + dir, start, dir, countSolutions)
				&& countSolutions == false)				// stop at first solution
					return true;
				puzzle[b][a].val = 0;
			}
		}
		return false;
	}

	int CountNeighbors(int mask)
	{
		int count = 0;
		while (mask != 0)
		{
			if ((mask & 1) != 0)
				count++;
			mask >>= 1;
		}
		return count;
	}

	// create a connected area and get the segment ends in this area
	bool CreateArea(int x, int y, int area)
	{
		Node * n = &puzzle[y][x];
		GetNeighbors(n, x, y);
		int neighbors = n->neighbors | n->neighbors0;
		bool single = CountNeighbors(n->neighbors0) == 1; // if this is a single cell by itself
		int countSegmentEnds = 0;						// count of segment ends for this cell
		puzzle[y][x].area = area;						// mark this as checked
		areas[area].numOpen++;
		if (areas[area].numEnds == 0)					// add 0 and 82 just in case
		{
			int ends = 0;
			if (pos[1].x < 0)
				areas[area].ends[ends++] = 0;
			if (pos[81].x < 0)
				areas[area].ends[ends++] = 82;
			areas[area].numEnds = ends;
		}
		for (int d = 0; d < MAX_DXDY; d++)
		{
			if ((neighbors & (1 << d)) != 0)
			{
				int a = x + dx[d];
				int b = y + dy[d];
				if (puzzle[b][a].val == 0)
				{
					if (puzzle[b][a].area >= 0)			// we already processed this entry
						continue;
					if (CreateArea(a, b, area) == false)
						return false;
				}
				else if (puzzle[b][a].val > 0)
				{
					if (puzzle[b][a].segmentEnd < 0)	// not calculated yet
					{
						if (SegmentEnd(puzzle[b][a].val))
						{
							puzzle[b][a].segmentEnd = 1 + numAreas;
							countSegmentEnds++;
							if (areas[area].numEnds < MAX_SEGMENTENDS)
							{
								areas[area].ends[areas[area].numEnds] = puzzle[b][a].val;
								areas[area].numEnds++;
							}
						}
						else
							puzzle[b][a].segmentEnd = 0;
					}
					else if (puzzle[b][a].segmentEnd > 0)
					{
						countSegmentEnds++;
						if (puzzle[b][a].segmentEnd != 1 + numAreas)	// it may have been used in a previous area
						{
							puzzle[b][a].segmentEnd = 1 + numAreas;
							if (areas[area].numEnds < MAX_SEGMENTENDS)
							{
								areas[area].ends[areas[area].numEnds] = puzzle[b][a].val;
								areas[area].numEnds++;
							}
						}
					}
				}
			}
		}
		if (single && countSegmentEnds == 0)				// this is a dead end that will fail
		{
			if (pos[1].x >= 0 && pos[81].x >= 0)			// 1 or 81 canot be here
				return false;
			// Rule 7 add code to test to see if we can get from 1 or 81 to this location
		}
		if (areas[area].numOpen == 1
		&& areas[area].numEnds == 0)						// deadends always fail
			return false;
		return true;
	}


	void RemoveEnd(int index)
	{
		int val = areas[numAreas].ends[index];
		if (val > 0 && val < 82)
			puzzle[pos[val].y][pos[val].x].segmentEnd = -1;	// this belongs to another areas
		for (int n = index + 1; n < areas[numAreas].numEnds; n++)
		{
			areas[numAreas].ends[n - 1] = areas[numAreas].ends[n];
		}
		areas[numAreas].numEnds--;
	}


	// check for areas around this location
	// we assume the neighbors have already been fetched for the solution
	bool CheckAreas(int x, int y)
	{
		if (x < 0 || y < 0)							// not in the puzzle so probably checking 0 to N or N to 82
			return true;
		Node * node = &puzzle[y][x];
		GetNeighbors(node, x, y);					// check neighbors now
		if (node->neighbors0 == 0					// segment end with no place to play
		&& SegmentEnd(node->val))
		{
#ifdef _DEBUG
			fprintf(file, "  Rule 1 Failed - isolated end segment!\n");
#endif
			return false;
		}
		int neighbors = node->neighbors | node->neighbors0;
		for (int d = 0; d < 8; d++)
		{
			if ((neighbors & (1 << d)) != 0)
			{
				int a = x + dx[d];
				int b = y + dy[d];
				if (puzzle[b][a].val == 0)				// open area
				{
					if (puzzle[b][a].area >= 0)			// we already checked this area
						continue;
					// some segment ends may be adjacent to more than one area
					// so we reset the flag segmentEnd to retest it in CreateArea()
					if (numAreas > 0)					// undo segment ends from previous list
					{
						for (int i = 0; i < areas[numAreas - 1].numEnds; i++)
						{
							int val = areas[numAreas - 1].ends[i];
							if (val > 0 && val < 82)
								puzzle[pos[val].y][pos[val].x].segmentEnd = -1;
						}
					}
					areas[numAreas].numOpen = 0;
					areas[numAreas].numEnds = 0;
					if (CreateArea(a, b, numAreas) == false)	// create an area
					{
#ifdef _DEBUG
						fprintf(file, "  Rule 1 Failed - Single isolated cell!\n");
#endif
						return false;
					}
					qsort(areas[numAreas].ends, areas[numAreas].numEnds, sizeof(areas[numAreas].ends[0]), SortEnds);
					// remove unused ends by looking for two patterns a->b->...->c or a->b
					bool cont = false;
					int used = 0;
					for (int n = 0; n < areas[numAreas].numEnds; )
					{
						if (n + 1 == areas[numAreas].numEnds)		// last entry is not possible
						{
							if (cont == false)						
								RemoveEnd(n);						// so remove last entry
							break;
						}
						int end0 = areas[numAreas].ends[n];
						int end1 = areas[numAreas].ends[n + 1];
						int nn;
						if (cont == false)							// a->b case
						{
							for (nn = end0 + 1; nn < end1; nn++)
							{
								if (pos[nn].x >= 0)					// a->b is not possible so a can be removed
									break;
							}
							if (nn == end1)							// a->b is valid
							{
								used += end1 - end0 - 1;
								cont = true;						// see if a->b->c is possible
								n++;
							}
							else
							{
								RemoveEnd(n);						// remove a
								continue;
							}
						}
						else										// see if a->b->c is possible
						{
							for (nn = end0 + 1; nn < end1; nn++)
							{
								if (pos[nn].x >= 0)					// a->b->c is not possible so look for b->c
								{
									cont = false;					// leave b alone
									break;
								}
							}
							if (nn == end1)							// a->b is valid
								used += end1 - end0 - 1;
							n++;
						}
					}
#ifdef _DEBUG
					fprintf(file, "  areas[%d] open=%d ends=", numAreas, areas[numAreas].numOpen);
					for (int i = 0; i < areas[numAreas].numEnds; i++)
						fprintf(file, "%2d ", areas[numAreas].ends[i]);
					fprintf(file, "\n");
#endif
					if (areas[numAreas].numEnds < 2)
					{
#ifdef _DEBUG
						fprintf(file, "  Rule 1 Failed - area with < 2 ends!\n");
#endif
						return false;
					}
					// now check to see if the area is invalid
					// NOTE it is possible to have open > used
					// when end segments are used in more than one area
					if (areas[numAreas].numOpen > used)
					{
#ifdef _DEBUG
						fprintf(file, "  Rule 2 Failed - Cannot fill area!\n");
#endif
						return false;
					}
					numAreas++;
				}
				else if (SegmentEnd(puzzle[b][a].val))			// this neighbor is a segment end
				{
					Node * node = &puzzle[b][a];
					GetNeighbors(node, a, b);					// check neighbors now
					if (node->neighbors0 == 0)					// segment end with no place to play
					{
#ifdef _DEBUG
						fprintf(file, "  Rule 1 Failed - Isolated segment end!\n");
#endif
						return false;
					}
				}
			}
		}
		return true;
	}

	// Find all areas and count the sement ends in each area
	bool ValidSolution(int n1, int n2)
	{
		int x, y;
		// clear segment end flags to create list of segment ends
		for (int y = 0; y < puzzleHeight; y++)
		{
			for (int x = 0; x < puzzleWidth; x++)
			{
				puzzle[y][x].area = -1;
				puzzle[y][x].segmentEnd = -1;
			}
		}
		for (int n = 0; n < n2 - n1 - 1; n++)
		{
			x = solution[n].x;
			y = solution[n].y;
			puzzle[y][x].segmentEnd = 0;
		}
		// make sure all areas ares valid
		numAreas = 0;
		if (CheckAreas(pos[n1].x, pos[n1].y) == false		// check for areas near the end segments
		|| CheckAreas(pos[n2].x, pos[n2].y) == false)
			return false;
		int dir = (n1 < n2 ? 1 : -1);
		int end = (n1 < n2 ? n2 - 1 - n1 : n1 - 1);
		for (int n = 0; n < end; n++)
		{
			x = solution[n].x;
			y = solution[n].y;
			if (CheckAreas(x, y) == false)					// this will not work either
				return false;
		}
		for (int n = 0; n < end; n++)
		{
			x = solution[n].x;
			y = solution[n].y;
			int val = (dir > 0 ? n1 + 1 + n : n1 - 1 - n);
			if (puzzle[y][x].used == 0)
				puzzle[y][x].used = val;
			else if (puzzle[y][x].used != val)
				puzzle[y][x].used = -1;
		}
		return true;
	}


	bool FindSolution(int x, int y, int n1, int n2, int next)
	{
		Node * n = &puzzle[y][x];
		int dir = (n1 < n2 ? 1 : -1);
		GetNeighbors(n, x, y);
		int neighbors = n->neighbors | n->neighbors0;
		// recursively try different places for the next number 
		for (int d = 0; d < MAX_DXDY; d++)
		{
			if ((neighbors & (1 << d)) != 0)
			{
				int a = x + dx[d];
				int b = y + dy[d];
				if (puzzle[b][a].val == 0)
				{
					if (next == n2)								// try again
						continue;
					if (dir > 0)
					{
						solution[next - 1 - n1].x = a;			// save solution so far
						solution[next - 1 - n1].y = b;
					}
					else
					{
						solution[n1 - 1 - next].x = a;			// save solution so far
						solution[n1 - 1 - next].y = b;
					}
					puzzle[b][a].val = next;
					pos[next].x = a;
					pos[next].y = b;
					bool answer = FindSolution(a, b, n1, n2, next + dir);
					puzzle[b][a].val = 0;
					pos[next].x = -1;
					pos[next].y = -1;
					// we do not stop because we want to see all possible paths
					// so that rule 5 can find the only possible values
				}
				else if (next == 0 
				||  next == puzzleMax + 1
				||  puzzle[b][a].val == next)
				{
					int end = (n1 < n2 ? n2 - 1 - n1 : n1 - 1);
#ifdef _DEBUG
					for (int n = 0; n < end; n++)
					{
						int val = (dir > 0 ? n1 + 1 + n : n1 - 1 - n);
						fprintf(file, "(%d,%d)=%d  ", solution[n].x, solution[n].y, val);
					}
					fprintf(file, "\n");
#endif
					if (ValidSolution(n1, n2) == false)
						return false;
#ifdef _DEBUG
					fprintf(file, "******** Valid Solution ********\n");
#endif
					if (numChoices < 0)							// add solution
					{
						numChoices = n2 - n1;					// number of valid entries
						if (numChoices < 0)
							numChoices = -numChoices;
						numChoices--;
						for (int n = 0; n < numChoices; n++)
							choices[n] = solution[n];
						return true;
					}
					else										// removes choices if necessary
					{
						for (int n = 0; n < end; n++)
						{
							if (choices[n].x < 0)				// deleted from our list
								continue;
							if (choices[n].x != solution[n].x	// different solution
							|| choices[n].y != solution[n].y)
							{
								choices[n].x = -1;
								numChoices--;
							}
						}
					}
					return true;
				}
			}
		}
		return false;
	}

	bool FindSolutions(int n1, int n2)
	{
#ifdef _DEBUG
		char string[32];
		sprintf(string, "solutions %d-%d.txt", n1, n2);
		file = fopen(string, "w");
#endif
		numChoices = -1;
		int dir = (n1 < n2 ? 1 : -1);
		int end = (n1 < n2 ? n2 - 1 - n1 : n1 - 1);
		FindSolution(pos[n1].x, pos[n1].y, n1, n2, n1 + dir);
		if (numChoices <= 0)
		{
#ifdef _DEBUG
			fclose(file);
#endif
			return false;
		}
		for (int n = 0; n < end; n++)
		{
			if (choices[n].x < 0)						// this is invalid
				continue;
			int val = (dir > 0 ? n1 + 1 + n : n1 - 1 - n);
#ifdef _DEBUG
			fprintf(file, "puzzle[%d][%d]=%d\n", choices[n].y, choices[n].x, val);
#endif
			puzzle[choices[n].y][choices[n].x].val = val;
			puzzle[choices[n].y][choices[n].x].added = true;
			pos[val].x = choices[n].x;
			pos[val].y = choices[n].y;
		}
		nodes++;
#ifdef _DEBUG
		fclose(file);
#endif
		return true;
	}

	int GetNeighbors(Node * n, int x, int y)
	{
		int count = 0;
		n->neighbors = 0;
		n->neighbors0 = 0;
		if (y > 0)
		{
			if (puzzle[y - 1][x].val == 0)
			{
				n->neighbors0 |= 1;
				count++;
			}
			else if (puzzle[y - 1][x].val > 0)
				n->neighbors |= 1;
		}
		if (x > 0)
		{
			if (puzzle[y][x - 1].val == 0)
			{
				n->neighbors0 |= 2;
				count++;
			}
			else if (puzzle[y][x - 1].val > 0)
				n->neighbors |= 2;
		}
		if (x < puzzleWidth - 1)
		{
			if (puzzle[y][x + 1].val == 0)
			{
				n->neighbors0 |= 4;
				count++;
			}
			else if (puzzle[y][x + 1].val > 0)
				n->neighbors |= 4;
		}
		if (y < puzzleHeight - 1)
		{
			if (puzzle[y + 1][x].val == 0)
			{
				n->neighbors0 |= 8;
				count++;
			}
			else if (puzzle[y + 1][x].val > 0)
				n->neighbors |= 8;
		}
		return count;
	}
};


extern hSolver	s;
extern STATE	state;
extern	int		xOffset, yOffset;

extern HBITMAP		hBoard;
extern HBITMAP		hBoardOn;
extern HFONT		hGameFont0;
extern HFONT		hGameFont1;
extern HFONT		hGameFont2;
void DrawPuzzle(HWND hWnd, HDC hdc);

bool DoFileLoad(HWND hwnd);
bool DoFileSave(HWND hwnd);
