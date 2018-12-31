// tic tac toe v2 using suggestions from Stack Exchange for better style
// Minus the struct stuff which I don't quite understand just yet.

// ncurses for, well, ncurses
#include <ncurses.h>
// time for the random seed
#include <time.h>
// string.h for strlen() 
#include <string.h>
// stdlib and stdio because why not
#include <stdlib.h>
#include <stdio.h>
// ctype.h for toupper()
#include <ctype.h>

// #define's for the COLOR_PAIRs
#define X_COLOR 1
#define O_COLOR 2
#define BG_COLOR 3

// #defines used as a global constant
#define num_spaces 9

// Function Declarations
void init_spaces(char *space_ptr);
void paint_board(char playable_spaces[num_spaces]);
void take_turn(char side, char *space_ptr, char playable_spaces[num_spaces]);
void victory_splash(int game_over_state);
void paint_background();
void player_turn(char *space_ptr, char playable_spaces[num_spaces], char side);
void ai_turn(char *space_ptr, char playable_spaces[num_spaces], char side);
void set_color_ai_side(char ai_side);
void set_color_side(char side);
int main_menu();
int evaluate_board(char playable_spaces[num_spaces]);
int spaces_left(char playable_spaces[num_spaces]);
int ai_fart(const int chance_to_fart);
int pick_random_space(char playable_spaces[num_spaces]);
int check_for_winning_move(char playable_spaces[num_spaces], char ai_side);
int check_for_block(char playable_spaces[num_spaces], char side);
int check_for_2_space_path(char playable_spaces[num_spaces], char ai_side);
char pick_side();

int main(){
	// To-Do: Try the time(NULL) method for srand initialization and see if it works the same
	time_t t;
	srand((unsigned) time(&t));
	char playable_spaces[num_spaces] = {'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X'};
	char *space_ptr = &playable_spaces[0];

	// Game over splash
	char game_over_str[] =  " Game Over! Any key to continue... ";
	char go_padding[] = "                                   ";
	int game_over_len = strlen(game_over_str);
	int row, col, x, y;

	//curses init
	initscr();
	cbreak();
	keypad(stdscr, 1);
	curs_set(0);
	start_color();
	init_pair(X_COLOR, COLOR_CYAN, COLOR_BLACK);
	init_pair(O_COLOR, COLOR_GREEN, COLOR_BLACK);
	init_pair(BG_COLOR, COLOR_YELLOW, COLOR_BLACK);
	noecho();

	// Main Menu outer loop
	int running = 1;
	while(running){
		curs_set(0);
		// Main menu function quits or continues
		running = main_menu();
		// In-Game inner loop
		if(running == 0){
			break;
		}
		int playing = 1;
		while(playing){
			// Init all spaces to blank
			init_spaces(space_ptr);
			// Player picks their side.
			char side = pick_side();
			// The inner, inner turn loop
			int turning = 1;
			while(turning){
				int game_over = 0;
				// Paint the board state as it is that turn
				paint_board(playable_spaces);
				// Function that governs the turn cycle
				take_turn(side, space_ptr, playable_spaces);
				// Evaluate the board for game over state
				game_over = evaluate_board(playable_spaces);
				if(game_over > 0){
					// paint the board with a splash on game over
					// so the player can evaluate the board for a moment
					paint_board(playable_spaces);
					getmaxyx(stdscr, row, col);
					y = row / 2 + 6;
					x = col / 2 - game_over_len / 2;
					attron(COLOR_PAIR(BG_COLOR));
					mvprintw(y++, x, go_padding);
					mvprintw(y++, x, game_over_str);
					mvprintw(y, x, go_padding);
					refresh();
					getch();
					// call victory_splash with int game_over as a parameter
					// 1 = X wins, 2 = O wins, 3 = Tie
					victory_splash(game_over);
					// Reset the turning and playing loops to effectively start over
					turning = 0;
					playing = 0;
				}
			}
		}
	}

	// end curses
	endwin();

	return 0;
}


void init_spaces(char *space_ptr){ 
	// init all the spaces to ' ';
	int i;
	for(i = 0; i < 9; i++){
		*space_ptr = ' ';
		space_ptr++;
	}
}

void paint_board(char playable_spaces[num_spaces]){
	// paint the board and the playable spaces
	clear();
	paint_background();
	char break_lines[] = " ------- ";
	char play_lines[] =  " | | | | ";
	char padding[] =     "         ";
	int row, col, x, y;
	getmaxyx(stdscr, row, col);
	y = row / 2 - 4;
	int len;
	len = strlen(padding);
	x = col / 2 - len / 2;
	int k;
	const int num_lines = 9;
	attron(COLOR_PAIR(BG_COLOR));
	for(k = 0; k < num_lines; k++){
		// Paint the board itself without the pieces
		if(k == 0 || k == num_lines - 1){
			mvprintw(y + k, x, padding);
		}else{
			if(k % 2 == 0){
				mvprintw(y + k, x, play_lines);
			}else{
				mvprintw(y + k, x, break_lines);
			}
		}
	}
	attroff(COLOR_PAIR(BG_COLOR));
	// insert Xs and Os:
	// First set the dynamic x and y coordinates based on terminal size
	int playable_x[num_spaces] = {x+2, x+4, x+6, x+2, x+4, x+6, x+2, x+4, x+6};
	int playable_y[num_spaces] = {y+2, y+2, y+2, y+4, y+4, y+4, y+6, y+6, y+6};
	for(k = 0; k < num_spaces; k++){
		// For each of the playable spaces, first set the color
		if(playable_spaces[k] == 'O'){
			attron(COLOR_PAIR(O_COLOR));		
		}else if(playable_spaces[k] == 'X'){
			attron(COLOR_PAIR(X_COLOR));
		}else{
			attron(COLOR_PAIR(BG_COLOR));
		}
		// then insert the char for that space into the proper spot on the terminal
		mvaddch(playable_y[k], playable_x[k], playable_spaces[k]);
	}
	// refresh the screen
	refresh();
}

void take_turn(char side, char *space_ptr, char playable_spaces[num_spaces]){
	// using "side" to determine the order, call the functions to play a whole turn
	if(side == 'X'){
		player_turn(space_ptr, playable_spaces, side);
		paint_board(playable_spaces);
		if(spaces_left(playable_spaces)){
			if(!(evaluate_board(playable_spaces))){
				ai_turn(space_ptr, playable_spaces, side);
				paint_board(playable_spaces);
			}
		}
	}else if(side == 'O'){
		ai_turn(space_ptr, playable_spaces, side);
		paint_board(playable_spaces);
		if(spaces_left(playable_spaces)){
			if(!(evaluate_board(playable_spaces))){
				player_turn(space_ptr, playable_spaces, side);
				paint_board(playable_spaces);
			}
		}
	}
}

int main_menu(){
	clear();
	// Takes user input and returns an int that quits or starts a game
	int row, col, x, y;
	char error_string[] = " Invalid Input! Any key to try again... ";
	int error_str_len = strlen(error_string);
	char str1[] =      " NCURSES TIC TAC TOE (v2) ";
	char padding[] =   "                          ";
	char str2[] =      "    (P)lay or (Q)uit?     ";
	int len = strlen(str1);
	paint_background();
	getmaxyx(stdscr, row, col);
	y = row / 2 - 2;
	x = col / 2 - len / 2;
	mvprintw(y++, x, padding);
	mvprintw(y++, x, str1);
	mvprintw(y++, x, padding);
	mvprintw(y++, x, str2);
	mvprintw(y++, x, padding);
	int input;
	refresh();
	// get user input and return it
	input = toupper(getch());
	if(input == 'P'){
		return 1;
	}else if(input == 'Q'){
		return 0;
	}else{
		// call the function again if the input is bad
		x = col / 2 - error_str_len / 2;
		mvprintw(++y, x, error_string);
		getch();
		main_menu();
	}
}

int evaluate_board(char playable_spaces[num_spaces]){
	// Evaluates the state of the playable spaces and either does nothing
	// or ends the game.
	// Check all the possible winning combinations:
	if(playable_spaces[0] == 'X' && playable_spaces[1] == 'X' && playable_spaces[2] == 'X'){
		return 1;
	}else if(playable_spaces[3] == 'X' && playable_spaces[4] == 'X' && playable_spaces[5] == 'X'){
		return 1;
	}else if(playable_spaces[6] == 'X' && playable_spaces[7] == 'X' && playable_spaces[8] == 'X'){
		return 1;
	}else if(playable_spaces[0] == 'X' && playable_spaces[3] == 'X' && playable_spaces[6] == 'X'){
		return 1;
	}else if(playable_spaces[1] == 'X' && playable_spaces[4] == 'X' && playable_spaces[7] == 'X'){
		return 1;
	}else if(playable_spaces[2] == 'X' && playable_spaces[5] == 'X' && playable_spaces[8] == 'X'){
		return 1;
	}else if(playable_spaces[0] == 'X' && playable_spaces[4] == 'X' && playable_spaces[8] == 'X'){
		return 1;
	}else if(playable_spaces[2] == 'X' && playable_spaces[4] == 'X' && playable_spaces[6] == 'X'){
		return 1;
	}else if(playable_spaces[0] == 'O' && playable_spaces[1] == 'O' && playable_spaces[2] == 'O'){
		return 2;
	}else if(playable_spaces[3] == 'O' && playable_spaces[4] == 'O' && playable_spaces[5] == 'O'){
		return 2;
	}else if(playable_spaces[6] == 'O' && playable_spaces[7] == 'O' && playable_spaces[8] == 'O'){
		return 2;
	}else if(playable_spaces[0] == 'O' && playable_spaces[3] == 'O' && playable_spaces[6] == 'O'){
		return 2;
	}else if(playable_spaces[1] == 'O' && playable_spaces[4] == 'O' && playable_spaces[7] == 'O'){
		return 2;
	}else if(playable_spaces[2] == 'O' && playable_spaces[5] == 'O' && playable_spaces[8] == 'O'){
		return 2;
	}else if(playable_spaces[0] == 'O' && playable_spaces[4] == 'O' && playable_spaces[8] == 'O'){
		return 2;
	}else if(playable_spaces[2] == 'O' && playable_spaces[4] == 'O' && playable_spaces[6] == 'O'){
		return 2;
	}else{
		// Check all spaces for a tie
		int hits = 0;
		int i;
		for(i = 0; i < num_spaces; i++){
			if(playable_spaces[i] != ' '){
				hits++;
			}
		}
		if(hits >= num_spaces){
			return 3;
		}else{
			return 0;
		}
	}
}

char pick_side(){
	// Takes user input and returns the chosen side
	clear();
	paint_background();
	int row, col, x, y;
	char str1[] =    " Press 'X' for X, 'O' for O, or 'R' for random! ";
	char str2[] =    "        Good choice! Any key to continue...     ";
	char padding[] = "                                                ";
	char err_str[] = "      Invalid input! Any key to continue...     ";
	int len = strlen(str1);
	getmaxyx(stdscr, row, col);
	y = row / 2 - 2;
	x = col / 2 - len / 2;
	mvprintw(y++, x, padding);
	mvprintw(y++, x, str1);
	mvprintw(y++, x, padding);
	int input;
	int pick;
	refresh();
	// Get user input for picking a side. 'R' is random.
	input = toupper(getch());
	if(input == 'X' || input == 'O'){
		mvprintw(y, x, str2);
		refresh();
		getch();
		return (char) input;
	}else if(input == 'R'){
		pick = rand() % 2;
		if(pick == 0){
			input = 'X';
		}else if(pick == 1){
			input = 'O';
		}
		mvprintw(y, x, str2);
		refresh();
		getch();
		return (char) input;
	}else{
		// Call the function again on bad input
		mvprintw(y, x, err_str);
		refresh();
		getch();
		pick_side();
	}
}

void victory_splash(int game_over_state){
	// Takes the game over state and creates a victory splash
	char padding[] = "                                   ";
	char *str1 =     "              X Wins!              ";
	char *str2 =     "              O Wins!              ";
	char str3[] =    "         any key to continue...    ";
	char *str4 =     "             A tie game!           ";
	int len = strlen(padding);
	char *vic_pointer = NULL;
	// To avoid code duplication, use a pointer to pick the right string
	if(game_over_state == 1){
		vic_pointer = str1;
	}else if(game_over_state == 2){
		vic_pointer = str2;
	}else if(game_over_state == 3){
		vic_pointer = str4;
	}
	clear();
	paint_background();
	int row, col, x, y;
	getmaxyx(stdscr, row, col);
	y = row / 2 - 2;
	x = col / 2 - len / 2;
	mvprintw(y++, x, padding);
	mvprintw(y++, x, vic_pointer);
	mvprintw(y++, x, padding);
	mvprintw(y, x, str3);
	refresh();
	getch();
}

void paint_background(){
	// Paints an elaborate flashy background
	int row, col, x, y;
	int pick;
	getmaxyx(stdscr, row, col);
	for(y = 0; y <= row; y++){
		for(x = 0; x <= col; x++){
			pick = rand() % 3;
			if(pick == 0){
				attron(COLOR_PAIR(X_COLOR));
				mvprintw(y, x, "X");
				attroff(COLOR_PAIR(X_COLOR));
			}else if(pick == 1){
				attron(COLOR_PAIR(O_COLOR));
				mvprintw(y, x, "O");
				attroff(COLOR_PAIR(O_COLOR));
			}else if(pick == 2){
				attron(COLOR_PAIR(BG_COLOR));
				mvprintw(y, x, " ");
				attroff(COLOR_PAIR(BG_COLOR));
			}
		}
	}
	refresh();
}

void player_turn(char *space_ptr, char playable_spaces[num_spaces], char side){
	// Function for the player turn
	char padding[] =  "                                                ";
	char str1[] =     "    Use arrow keys to move and 'P' to place!    ";
	char str2[] =     "                   Good move!                   ";
	char str3[] =     "                 Invalid input!                 ";
	char str4[] =     "             You can't move that way!           ";
	char str5[] =     "              Space already occupied!           ";
	int len = strlen(padding);
	int row, col, x, y;
	getmaxyx(stdscr, row, col);
	const int board_line_len = 9;
	const int board_lines = 9;
	y = row / 2 - board_line_len / 2;
	x = col / 2 - board_line_len / 2;
	// Use the same method of dynamically measuring where the spaces are at using
	// terminal size as in the paint_board() function.
	int playable_x[num_spaces] = {x+2, x+4, x+6, x+2, x+4, x+6, x+2, x+4, x+6};
	int playable_y[num_spaces] = {y+2, y+2, y+2, y+4, y+4, y+4, y+6, y+6, y+6};
	// The variables and mvprintw functions for the "info line"
	const int info_line_y = (row / 2 - board_lines / 2) + 10;
	const int info_line_x = col / 2 - len / 2;
	mvprintw(info_line_y - 1, info_line_x, padding);
	mvprintw(info_line_y, info_line_x, str1);
	mvprintw(info_line_y + 1, info_line_x, padding);
	// Using a loop and pointers to collect user input
	int moving = 1;
	int input;
	int *pos_x = &playable_x[0];
	int *pos_y = &playable_y[0];
	move(*pos_y, *pos_x);
	curs_set(1);
	refresh();
	while(moving){
		// For each movement key, if the move is valid, use pointer
		// arithmetic to mov pos_x and pos_y around.
		input = toupper(getch());
		if(input == KEY_UP){
			if(*pos_y != playable_y[0]){
				pos_y -= 3;
				move(*pos_y, *pos_x);
				refresh();
			}else{
				mvprintw(info_line_y, info_line_x, str4);
				move(*pos_y, *pos_x);
				refresh();
			}
		}else if(input == KEY_DOWN){
			if(*pos_y != playable_y[6]){
				pos_y += 3;
				move(*pos_y, *pos_x);
				refresh();
			}else{
				mvprintw(info_line_y, info_line_x, str4);
				move(*pos_y, *pos_x);
				refresh();
			}
		}else if(input == KEY_LEFT){
			if(*pos_x != playable_x[0]){
				pos_x -= 1;
				move(*pos_y, *pos_x);
				refresh();
			}else{
				mvprintw(info_line_y, info_line_x, str4);
				move(*pos_y, *pos_x);
				refresh();
			}
		}else if(input == KEY_RIGHT){
			if(*pos_x != playable_x[2]){
				pos_x += 1;
				move(*pos_y, *pos_x);
				refresh();
			}else{
				mvprintw(info_line_y, info_line_x, str4);
				move(*pos_y, *pos_x);
				refresh();
			}
		}else if(input == 'P'){
			// I wanted to use KEY_ENTER instead of 'P' but it would not work
			// for some reason. When the user presses 'P' it checks where the
			// cursor is and sets the space_ptr to the appropriate index in the
			// playable_spaces array.
			if(*pos_y == playable_y[0] && *pos_x == playable_x[0]){
				space_ptr = &playable_spaces[0];				
			}else if(*pos_y == playable_y[1] && *pos_x == playable_x[1]){
				space_ptr = &playable_spaces[1];
			}else if(*pos_y == playable_y[2] && *pos_x == playable_x[2]){
				space_ptr = &playable_spaces[2];
			}else if(*pos_y == playable_y[3] && *pos_x == playable_x[3]){
				space_ptr = &playable_spaces[3];
			}else if(*pos_y == playable_y[4] && *pos_x == playable_x[4]){
				space_ptr = &playable_spaces[4];
			}else if(*pos_y == playable_y[5] && *pos_x == playable_x[5]){
				space_ptr = &playable_spaces[5];
			}else if(*pos_y == playable_y[6] && *pos_x == playable_x[6]){
				space_ptr = &playable_spaces[6];
			}else if(*pos_y == playable_y[7] && *pos_x == playable_x[7]){
				space_ptr = &playable_spaces[7];
			}else if(*pos_y == playable_y[8] && *pos_x == playable_x[8]){
				space_ptr = &playable_spaces[8];
			}
			// Then checks to see if that space is empty.
			// If so it sets the color properly and then places the piece.
			if(*space_ptr == ' '){
				if(side == 'X'){
					attron(COLOR_PAIR(X_COLOR));
					mvaddch(*pos_y, *pos_x, 'X');
					attron(COLOR_PAIR(BG_COLOR));
					*space_ptr = 'X';
				}else if(side == 'O'){
					attron(COLOR_PAIR(O_COLOR));
					mvaddch(*pos_y, *pos_x, 'O');
					attron(COLOR_PAIR(BG_COLOR));
					*space_ptr = 'O';
				}
				refresh();
				moving = 0;
			}else{
				mvprintw(info_line_y, info_line_x, str5);
				move(*pos_y, *pos_x);
				refresh();
			}
		}else{
			mvprintw(info_line_y, info_line_x, str3);
			move(*pos_y, *pos_x);
			refresh();
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////
// Begin AI Logic ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

void ai_turn(char *space_ptr, char playable_spaces[num_spaces], char side){
	// wrapper for the AI turn
	/*
		Note: Since it is easy to accidentally create an unbeatable AI for tic tac toe
		      I am building into the AI the chance for it to not make the optimal move.
					This intentional fuzziness will be built into the functions that check for
					avaialable spaces. When they find an optimal move they may just decide
					to return 0 anyway.
		
		P-Code:
		if center square not taken, take center square 70% of the time;
		else:
			if opponent about to win, block them 90% of the time;
			elif self about to win take winning spot 90% of the time;
			else pick a random open spot;
	*/
	// The chances for the AI to blow a move
	const int chance_to_fart_big_move = 10;
	const int chance_to_fart_center = 30;
	// Picking the character for the AI to use in its calculations
	char ai_side;
	if(side == 'X'){
		ai_side = 'O';
	}else if(side == 'O'){
		ai_side = 'X';
	}
	// Check the board state with a few functions.
	// These all return 0 if FALSE and the number of a valid
	// index to move into if TRUE
	int can_block_opponent = check_for_block(playable_spaces, side);
	int can_winning_move = check_for_winning_move(playable_spaces, ai_side);
	// Flow through the decision making logic applying the functions and checking for a fart
	int thinking = 1;
	int picked_space;
	while(thinking){
		if(playable_spaces[4] == ' '){
			if(!(ai_fart(chance_to_fart_center))){
				picked_space = 4;
				thinking = 0;
				break;
			}
		}
		if(can_winning_move){
			if(!(ai_fart(chance_to_fart_big_move))){
				picked_space = can_winning_move;
				thinking = 0;
			}else{
				picked_space = pick_random_space(playable_spaces);
				thinking = 0;
			}
		}else if(can_block_opponent){
			if(!(ai_fart(chance_to_fart_big_move))){
				picked_space = can_block_opponent;
				thinking = 0;
			}else{
				picked_space = pick_random_space(playable_spaces);
				thinking = 0;
			}
		}else{
			picked_space = pick_random_space(playable_spaces);
			thinking = 0;
		}
	}
	space_ptr = &playable_spaces[picked_space];
	if(ai_side == 'X'){
		attron(COLOR_PAIR(X_COLOR));
	}else if(ai_side == 'O'){
		attron(COLOR_PAIR(O_COLOR));
	}
	*space_ptr = ai_side;
	attron(COLOR_PAIR(BG_COLOR));
}


int ai_fart(const int chance_to_fart){
	// Takes the fart chance and returns 1 if the AI blows the move, 0 otherwise
	int roll;
	roll = rand() % 100 + 1;
	if(roll < chance_to_fart){
		return 1;
	}else{
		return 0;
	}
}

int pick_random_space(char playable_spaces[num_spaces]){
	// Returns a random open space on the board
	int roll;
	int rolling = 1;
	int pick;
	while(rolling){
		roll = rand() % num_spaces;
		if(playable_spaces[roll] == ' '){
			pick = roll;
			rolling = 0;
		}else{
			continue;
		}
	}
	return pick;
}

int check_for_winning_move(char playable_spaces[num_spaces], char ai_side){
	// Checks to see if the AI can win the game with a final move and returns the
	// index of the valid move if TRUE, returns 0 if FALSE
	int space;
	int pick;
	int picked = 0;
	for(space = 0; space < num_spaces; space++){
		// For each space: Check to see if it is a potential winning space and if so
		// switch "picked" to 1 and set "pick" to the winning index
		switch(space){
			case(0):
				if(playable_spaces[space] == ' '){
					if(playable_spaces[1] == ai_side && playable_spaces[2] == ai_side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[3] == ai_side && playable_spaces[6] == ai_side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[4] == ai_side && playable_spaces[8] == ai_side){
						pick = space;
						picked = 1;
					}
				}
				break;
			case(1):
				if(playable_spaces[space] == ' '){
					if(playable_spaces[0] == ai_side && playable_spaces[2] == ai_side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[4] == ai_side && playable_spaces[7] == ai_side){
						pick = space;
						picked = 1;
					}
				}
				break;
			case(2):
				if(playable_spaces[space] == ' '){
					if(playable_spaces[1] == ai_side && playable_spaces[0] == ai_side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[4] == ai_side && playable_spaces[6] == ai_side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[5] == ai_side && playable_spaces[8] == ai_side){
						pick = space;
						picked = 1;
					}
				}
				break;
			case(3):
				if(playable_spaces[space] == ' '){
					if(playable_spaces[4] == ai_side && playable_spaces[5] == ai_side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[0] == ai_side && playable_spaces[6] == ai_side){
						pick = space;
						picked = 1;
					}
				}
				break;
			case(4):
				if(playable_spaces[space] == ' '){
					if(playable_spaces[1] == ai_side && playable_spaces[7] == ai_side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[3] == ai_side && playable_spaces[5] == ai_side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[0] == ai_side && playable_spaces[8] == ai_side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[6] == ai_side && playable_spaces[2] == ai_side){
						pick = space;
						picked = 1;
					}
				}
				break;
			case(5):
				if(playable_spaces[space] == ' '){
					if(playable_spaces[8] == ai_side && playable_spaces[2] == ai_side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[3] == ai_side && playable_spaces[4] == ai_side){
						pick = space;
						picked = 1;
					}
				}
				break;
			case(6):
				if(playable_spaces[space] == ' '){
					if(playable_spaces[4] == ai_side && playable_spaces[2] == ai_side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[7] == ai_side && playable_spaces[8] == ai_side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[3] == ai_side && playable_spaces[0] == ai_side){
						pick = space;
						picked = 1;
					}
				}
				break;
			case(7):
				if(playable_spaces[space] == ' '){
					if(playable_spaces[6] == ai_side && playable_spaces[8] == ai_side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[4] == ai_side && playable_spaces[1] == ai_side){
						pick = space;
						picked = 1;
					}
				}
				break;
			case(8):
				if(playable_spaces[space] == ' '){
					if(playable_spaces[5] == ai_side && playable_spaces[2] == ai_side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[4] == ai_side && playable_spaces[0] == ai_side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[7] == ai_side && playable_spaces[6] == ai_side){
						pick = space;
						picked = 1;
					}
				}
				break;
		}
	}
	// return winning index if any
	if(picked){
		return pick;
	}else{
		return 0;
	}
}

int check_for_block(char playable_spaces[num_spaces], char side){
	// Checks to see if the AI can block the player from winning the game with a final move
	// and returns the index of the valid move if TRUE, returns 0 if FALSE
	// Note: I am sure there is a way to combine this this function with the
	// 	check_for_winning_move() function in order to avoid code duplication, probably using
	//  one more parameter as a switch of some kind. I'd be open to examples of how to do that.
	int space;
	int pick;
	int picked = 0;
	for(space = 0; space < num_spaces; space++){
		// For each space: Check to see if it is a potential winning space and if so
		// switch "picked" to 1 and set "pick" to the winning index
		switch(space){
			case(0):
				if(playable_spaces[space] == ' '){
					if(playable_spaces[1] == side && playable_spaces[2] == side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[3] == side && playable_spaces[6] == side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[4] == side && playable_spaces[8] == side){
						pick = space;
						picked = 1;
					}
				}
				break;
			case(1):
				if(playable_spaces[space] == ' '){
					if(playable_spaces[0] == side && playable_spaces[2] == side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[4] == side && playable_spaces[7] == side){
						pick = space;
						picked = 1;
					}
				}
				break;
			case(2):
				if(playable_spaces[space] == ' '){
					if(playable_spaces[1] == side && playable_spaces[0] == side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[4] == side && playable_spaces[6] == side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[5] == side && playable_spaces[8] == side){
						pick = space;
						picked = 1;
					}
				}
				break;
			case(3):
				if(playable_spaces[space] == ' '){
					if(playable_spaces[4] == side && playable_spaces[5] == side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[0] == side && playable_spaces[6] == side){
						pick = space;
						picked = 1;
					}
				}
				break;
			case(4):
				if(playable_spaces[space] == ' '){
					if(playable_spaces[1] == side && playable_spaces[7] == side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[3] == side && playable_spaces[5] == side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[0] == side && playable_spaces[8] == side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[6] == side && playable_spaces[2] == side){
						pick = space;
						picked = 1;
					}
				}
				break;
			case(5):
				if(playable_spaces[space] == ' '){
					if(playable_spaces[8] == side && playable_spaces[2] == side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[3] == side && playable_spaces[4] == side){
						pick = space;
						picked = 1;
					}
				}
				break;
			case(6):
				if(playable_spaces[space] == ' '){
					if(playable_spaces[4] == side && playable_spaces[2] == side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[7] == side && playable_spaces[8] == side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[3] == side && playable_spaces[0] == side){
						pick = space;
						picked = 1;
					}
				}
				break;
			case(7):
				if(playable_spaces[space] == ' '){
					if(playable_spaces[6] == side && playable_spaces[8] == side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[4] == side && playable_spaces[1] == side){
						pick = space;
						picked = 1;
					}
				}
				break;
			case(8):
				if(playable_spaces[space] == ' '){
					if(playable_spaces[5] == side && playable_spaces[2] == side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[4] == side && playable_spaces[0] == side){
						pick = space;
						picked = 1;
					}else if(playable_spaces[7] == side && playable_spaces[6] == side){
						pick = space;
						picked = 1;
					}
				}
				break;
		}
	}
	// return winning index if any
	if(picked){
		return pick;
	}else{
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// End AI Logic ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

int spaces_left(char playable_spaces[num_spaces]){
	// Returns 0 if no spaces left
	int hits = 0;
	int k;
	for(k = 0; k < num_spaces; k++){
		if(playable_spaces[k] == ' '){
		hits++;
		}
	}
	return hits;
}

