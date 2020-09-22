Numbrix is a puzzle where you are given an array of 9x9 boxes or tiles with 
some of the numbers from 1 to 81.  You are asked to solve the puzzle going 
horizontal or vertical to get the adjacent numbers.  The puzzle should always 
have only one solution.  This was popularized by Marilyn vos Savant and
would appear in Parade magazine.

So to enter a new puzzle select New in the File Menu.  Now you will be asked
to input any set of numbers you like.  Press Return to enter the solve phase.
You may select Solve from the Game Menu or select Step to get a single bit of 
information.  Of course you may enter the numbers yourself using the arrow 
keys or the mouse to select any tile.  

It is easy to solve the puzzle by using a recursive algorithm.  However, this 
is difficult for people to understand why.  So I am doing something different here.
I try to solve the puzzle in "steps" by finding obvious choices.  

When I create a possible path between A and B I create a list of open areas and 
count the number of entries in this area as well as a list of end segments that 
need to be used for each area.  I then cull the list of end segments, removing 
any ends that cannot reach the next end through this area.  I also make sure 0 
and 82 are used as possible end segments to be sure all end points are covered.
Now several situations will help give us more information:
1. Any path that leaves "dead" areas cannot be the correct path so disregard it.
I define a dead area as any open area with 0 or 1 end segments.  I also include 
in this list any "isloated" end segments that do not have an open area next to them.
2. The end segments will give us a count of the number of tiles that will be filled
and if this cannot fill the open area then we can reject this path.  
3. If there is only one way to get from one segment end A to the next end B
then fill this path.  
4. Some times all paths include portions that are same although other entries may 
vary, so fill in the common tiles. 
5. Because the above condition is necessary it is not sufficient.  It is possible
that all possible paths for all end segments leave some tiles with only one possible 
value.  In that case we use those values.  See "Puzzles/20_05_25 Rule 5.txt".
6. Just in case I missed something I added a brute force method that finds all
solutions to the puzzle and any common tiles are filled in.  I do not believe this
method is needed using the above steps.  See "Puzzles/brute force.txt".
7. I found other situations where we can make possible solution paths invalid.
If a solution leaves one tile with only one open neighbor and no end segments
then this solution is not valid unless this tile is 1 or 81. See the example
"Puzzles/20_05_22 22 known.txt".  If (7,2)=22 then tile (7,1) must be 1 
or 81 and we know this cannot be the case.  Also see "Puzzles/20_05_22 49-57 known.txt".
If (1,2) = 50 then we have a single place where 1 or 81 cannot occur.
8. There are cases when we need to extend rule 6. If there is an area with only 
one way in, then 1 or 81 must be in this area. 
9. However, the above algorithm fails if there are more than one solution to 
the puzzle. You can see this method stops to show you where there multiple 
solutions are possible.  There are examples in the Puzzles folder.

NOTE: I have not implemented rule 7 or 8 yet because I have not needed to.

By the way the debug version of the program creates files which provide the 
solutions for all the segments it tries.

I have provided a means to load and save puzzles.  Please look in the Puzzles
folder for samples.  Most of these samples I got from the Parade magazine in
the Mercury News or from the Parade website at:
https://parade.com/numbrix/

This game is very similar to Hidato so I reused a lot of my code for this project.

Ed Logg
