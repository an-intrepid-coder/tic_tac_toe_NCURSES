#include <ncurses.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

// #define's for the COLOR_PAIRs
#define X_COLOR 1
#define O_COLOR 2
#define BG_COLOR 3

// #defines used as a global constant
#define SIDE_LEN 3
#define NUM_SQUARES (SIDE_LEN*SIDE_LEN)

typedef struct board {
    char spaces[NUM_SQUARES];
    int available;
    int start_row;
    int start_col;
} Board;

struct player {
    char token;
    int (*turn)(Board *, char);
};

void paint_background(){
	// Paints an elaborate flashy background
    static const int colors[3] = { X_COLOR, O_COLOR, BG_COLOR };
    static const char tokens[3] = { 'X', 'O' , ' ' };
    int row, col;
	getmaxyx(stdscr, row, col);
	for(int y = 0; y <= row; y++){
		for(int x = 0; x <= col; x++){
            int n = rand() % 3;
            attron(COLOR_PAIR(colors[n]));
            mvaddch(y, x, tokens[n]);
		}
	}
	attron(COLOR_PAIR(BG_COLOR));
	refresh();
}

// prints msg centered on passed line number 
void center_print(int line, const char *msg) {
	int row, col;
	getmaxyx(stdscr, row, col);
	mvprintw(line, (col - strlen(msg) + 1)/2, msg);
}

bool main_menu(){
	// Takes user input and returns an int that quits or starts a game
	static const char error_string[] = " Invalid Input! Any key to try again... ";
	static const char str1[] =      " NCURSES TIC TAC TOE (v2) ";
	static const char padding[] =   "                          ";
	static const char str2[] =      "    (P)lay or (Q)uit?     ";
    curs_set(0);
	paint_background();
	int row, col;
	getmaxyx(stdscr, row, col);
	int y = row / 2 - 2;
    center_print(y++, padding);
    center_print(y++, str1);
    center_print(y++, padding);
    center_print(y++, str2);
    center_print(y++, padding);
	refresh();
    while (1) {
        int input = toupper(getch());
        switch(input) {
            case 'P':
                return true;
                break;
            case 'Q':
                return false;
                break;
            default:
                center_print(y, error_string);
        }
    }
}

char pick_side(){
	// Takes user input and returns the chosen side
	clear();
	paint_background();
	char str1[] =    " Press 'X' for X, 'O' for O, or 'R' for random! ";
	char str2[] =    "        Good choice! Any key to continue...     ";
	char padding[] = "                                                ";
	char err_str[] = "      Invalid input! Any key to continue...     ";
	int row, col;
    getmaxyx(stdscr, row, col);
	int y = row / 2 - 1;
    center_print(++y, padding);
    center_print(++y, str1);
    center_print(++y, padding);
    ++y;
	refresh();
	// Get user input for picking a side. 'R' is random.
    while (1) {
        int input = toupper(getch());
        switch (input) {
            case 'R':
                input = "XO"[rand() % 2];
                // deliberately fall through
            case 'X':
            case 'O':
                center_print(y, str2);
                refresh();
                getch();
                return (char) input;
                break;
            case 'A':
                return (char) input;
                break;
            default:
                center_print(y, err_str);
                refresh();
                getch();
                center_print(y, padding);
        }
    }
}

void victory_splash(int game_over_state){
	// Takes the game over state and creates a victory splash
	static const char padding[] = "                                   ";
    static const char *result[] = {
        "              X Wins!              ",
        "              O Wins!              ",
        "             A tie game!           ",
    };
	static const char *cont =     "         any key to continue...    ";

	paint_background();
	int row, col;
	getmaxyx(stdscr, row, col);
	int y = row / 2 - 1;
    center_print(++y, padding);
    center_print(++y, result[game_over_state-1]);
    center_print(++y, padding);
    center_print(++y, cont);
	refresh();
	getch();
}

int player_turn(Board *b, struct player *p) {
    return p->turn(b, p->token);
}

char board_gettoken(const Board *b, int row, int col) {
    return b->spaces[row + col * SIDE_LEN];
}

char board_settoken(Board *b, int row, int col, char token) {
    return b->spaces[row + col * SIDE_LEN] = token;
}

// transforms row and col into screen coordinates
void board_screen_coords(const Board *b, int *row, int *col) {
    *col = b->start_col+(*col * 2);
    *row = b->start_row+(*row * 2);
}

void board_moveto(const Board *b, int row, int col) {
    int board_x = col, board_y = row;
    board_screen_coords(b, &board_y, &board_x);
	move(board_y, board_x);
}

void board_showtoken(const Board *b, int row, int col) {
    char token = board_gettoken(b, col, row);
    switch(token) {
        case 'X':
            attron(COLOR_PAIR(X_COLOR));
            break;
        case 'O':
            attron(COLOR_PAIR(O_COLOR));
            break;
        case ' ':
            attron(COLOR_PAIR(BG_COLOR));
            break;
    }
    int board_x = col, board_y = row;
    board_screen_coords(b, &board_y, &board_x);
    mvaddch(board_y, board_x, token);
}

void board_paint(const Board *b) {
	// paint the board and the playable spaces
	paint_background();
    static const char linewidth = 4+2*SIDE_LEN;
	char break_lines[linewidth]; 
	char play_lines[linewidth]; 
	char padding[linewidth];
    int i = linewidth-1;
    break_lines[i] = play_lines[i] = padding[i] = '\0';
    for (i=0; i < linewidth-1; ++i) {
        break_lines[i] = play_lines[i] = padding[i] = ' ';
    }
    for (i=1; i < linewidth-2; ++i) {
        break_lines[i] = '-';
    }
    for (i=1; i < linewidth-1; i += 2) {
        play_lines[i] = '|';
    }
    int startline = b->start_row - 2;

    // print the outline and blank squares
	attron(COLOR_PAIR(BG_COLOR));
    center_print(startline++, padding);
    for (int i=0; i < SIDE_LEN; ++i) {
        center_print(startline++, break_lines);
        center_print(startline++, play_lines);
    }
    center_print(startline++, break_lines);
    center_print(startline++, padding);

	// insert Xs and Os:
    for (int i=0; i < SIDE_LEN; ++i) {
        for (int j=0; j < SIDE_LEN; ++j) {
            board_showtoken(b, i, j);
		}
	}
	refresh();
}

void board_reset(Board *b) {
    for (int i=0; i < NUM_SQUARES; ++i) {
       b->spaces[i] = ' '; 
    }
    b->available = NUM_SQUARES;
    int row, col;
	getmaxyx(stdscr, row, col);
	b->start_col = col / 2 - SIDE_LEN + 1;
    b->start_row = row / 2 - SIDE_LEN + 1;
}

// given that `token` just moved, return 
//  0 if no winner
//  1 if token just won
//  2 if tie
int board_evaluate(const Board *b, char token) {
    bool diag_winner = true;
    bool rev_diag_winner = true;
    for (int i=0; i < SIDE_LEN; ++i) {
        diag_winner &= board_gettoken(b, i, i) == token;
        rev_diag_winner &= board_gettoken(b, i, SIDE_LEN-1-i) == token;
    }
    if (diag_winner || rev_diag_winner) {
        return 1;
    }
    for (int i=0; i < SIDE_LEN; ++i) {
        bool row_winner = true;
        bool col_winner = true;
        for (int j=0; j < SIDE_LEN; ++j) {
            col_winner &= board_gettoken(b, i, j) == token;
            row_winner &= board_gettoken(b, j, i) == token;
        }
        if (row_winner || col_winner) {
            return 1;
        }
    }
    // must be a non-win or tie
    return b->available == 0 ? 2 : 0;
}

int human_turn(Board *b, char token) {
	// Function for the player turn
	char padding[] =  "                                                ";
	char str1[] =     "    Use arrow keys to move and 'P' to place!    ";
	char str3[] =     "                 Invalid input!                 ";
	char str4[] =     "             You can't move that way!           ";
	char str5[] =     "              Space already occupied!           ";

    attron(COLOR_PAIR(BG_COLOR));
	const int info_line = b->start_row + 2*SIDE_LEN + 1;
	center_print(info_line - 1, padding);
	center_print(info_line, str1);
	center_print(info_line + 1, padding);

	// collect user input
    int x = 0;
    int y = 0;
    board_moveto(b, y, x);
	curs_set(1);
	refresh();
	for(bool moving = true; moving; ){
		// For each movement key, if the move is valid, use pointer
		// arithmetic to mov pos_x and pos_y around.
		int input = toupper(getch());
        switch (input) {
            case KEY_UP:
                if (y == 0) {  // can't move up
                    center_print(info_line, str4);
                } else {
                    center_print(info_line, padding);
                    --y;
                }
                break;
            case KEY_DOWN:
                if (y == SIDE_LEN-1) { // can't move down
                    center_print(info_line, str4);
                } else {
                    center_print(info_line, padding);
                    ++y;
                }
                break;
            case KEY_LEFT:
                if (x == 0) { // can't move left
                    center_print(info_line, str4);
                } else {
                    center_print(info_line, padding);
                    --x;
                }
                break;
            case KEY_RIGHT:
                if (x == SIDE_LEN-1) { // can't move right
                    center_print(info_line, str4);
                } else {
                    center_print(info_line, padding);
                    ++x;
                }
                break;
            case 'P':
                if (board_gettoken(b, x, y) != ' ') {
                    center_print(info_line, str5);
                } else {
                    board_settoken(b, x, y, token);
                    --b->available;
                    moving = false;
                }
                break;
            default:
                center_print(info_line, str3);
        }
        board_moveto(b, y, x);
        refresh();
	}
    return board_evaluate(b, token);
}

int board_makemove(Board *b, int i, int j, char token) {
    board_settoken(b, i, j, token);
    --b->available;
    return board_evaluate(b, token);
}

int ai_turn(Board *b, char token) {
    const char antitoken = token == 'X' ? 'O' : 'X';
    // first look for a move that would win
    for (int i=0; i < SIDE_LEN; ++i) {
        for (int j=0; j < SIDE_LEN; ++j) {
            if (board_gettoken(b, i, j) == ' ') {
                board_settoken(b, i, j, token);
                int status = board_evaluate(b, token);
                if (status) {
                    --b->available;
                    return status;
                } else {
                    board_settoken(b, i, j, ' ');  // undo move
                }
            }
        }
    }
    // next look for a move that would block
    for (int i=0; i < SIDE_LEN; ++i) {
        for (int j=0; j < SIDE_LEN; ++j) {
            if (board_gettoken(b, i, j) == ' ') {
                board_settoken(b, i, j, antitoken);
                if (board_evaluate(b, antitoken) == 1) {
                    return board_makemove(b, i, j, token);
                } else {
                    board_settoken(b, i, j, ' ');  // undo move
                }
            }
        }
    }
    // look for center
    { 
        int i = SIDE_LEN/2;
        if (board_gettoken(b, i, i) == ' ') {
            return board_makemove(b, i, i, token);
        }
    }
    // look for corner 
    for (int i=0; i < SIDE_LEN; i += (SIDE_LEN-1)) {  
        for (int j=0; j < SIDE_LEN; j += (SIDE_LEN-1)) {  
            if (board_gettoken(b, i, j) == ' ') {
                return board_makemove(b, i, j, token);
            }
        }
    }
    // choose first available
    for (int i=0; i < SIDE_LEN; ++i) {
        for (int j=0; j < SIDE_LEN; ++j) {
            if (board_gettoken(b, i, j) == ' ') {
                return board_makemove(b, i, j, token);
            }
        }
    }
    return 0;
}

int main(){
    // Game over splash messages
    static const char game_over_str[] =  " Game Over! Any key to continue... ";
    static const char go_padding[] = "                                   ";

    // the players  (X is always first)
    struct player Player[2] = {
        { 'X', ai_turn },
        { 'O', ai_turn } 
    };
    Board board;

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
	for (bool running = main_menu(); running; running = main_menu()) {
        // Init all spaces to blank
        board_reset(&board);
        // Player picks their side.
        switch (pick_side()) {
            case 'X':
                Player[0].turn = human_turn;
                Player[1].turn = ai_turn;
                break;
            case 'O':
                Player[0].turn = ai_turn;
                Player[1].turn = human_turn;
                break;
            case 'A':
                Player[0].turn = ai_turn;
                Player[1].turn = ai_turn;
                break;
        }
        int current_player = 0;
        int game_over = 0;
        for (board_paint(&board);
                (game_over = player_turn(&board, &Player[current_player])) == 0; 
                current_player = 1 - current_player)
        {
            // Function that governs the turn cycle
            board_paint(&board);
        }
        board_paint(&board);
        // paint the board with a splash on game over
        // so the player can evaluate the board for a moment
        attron(COLOR_PAIR(BG_COLOR));
        const int info_line = board.start_row + 2*SIDE_LEN + 1;
        center_print(info_line - 1, go_padding);
        center_print(info_line, game_over_str);
        center_print(info_line - 1, go_padding);
        refresh();
        getch();
        // call victory_splash with int game_over as a parameter
        // 1 = X wins, 2 = O wins, 3 = Tie
        victory_splash(game_over == 2 ? 3 : 1+current_player);
	}

	// end curses
	endwin();
}
