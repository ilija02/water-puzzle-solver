# Water puzzle solver
## A console program written in C that generates and solves the water bottle puzzle using a game tree.
<p align="center"><img alt="PICTURE logo" src="/images/game.png" width="800"></p>

This program uses a game tree data structure to solve the game, which means that every possible state is stored in order to get to the solution. Number of moves parameter determines the height of the tree.
Given that information, it can use a large amount of RAM if the number of empty bottles is greater than 2. 
Recommended parameters are:
 - 5 bottles
 - 2 empty bottles
 - 10 moves
