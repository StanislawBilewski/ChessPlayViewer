# ChessPlayViewer
A project for 'Computer Graphics and Visualisation' classes

-----------------

The main goal of the project was to create a 3D viewer for chess plays using OpenGL technology.

The play in question should be written down within the 'partia.txt' file (files 'partia.txt', 'partia1.txt' and 'partia2.txt' present in the repository are example plays). The format used in this file should follow Algebraic Notation with the white's move and black's move are separated by whitespace and a new line signifies the next turn (there also is a possibility of adding comments - if a line starts with '//' it will be ignored - as well as signifying a player's resignation with an exclamation mark).

Within the application user may use arrow keys in order to rotate camera around the chessboard, use the spacebar to toggle pause (note that the pause is on by the default at the beggining of game) and 'r' key to reset current game and play it again from the start.

The lights behind each player's starting position indicate which player is making the current move (green light) as well as the game's outcome (green light for the winner, red light for the player that lost - in case of a draw or an unfinished game both of the lights would be blue).

-----------------

Project developed in collaboration with [Witold Andraszyk](https://github.com/WAndraszyk "Witold's repository")

-----------------

Note, that the project utilizes libraries such as glew, glfw, glm and lodepng which have not been uploaded onto this repository
