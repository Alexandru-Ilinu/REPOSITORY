#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <time.h>
#include <ctype.h>

#define NO_KEY_PRESSED (-1)

#define NR_TILES 7 // numarul de tipuri diferite de piese
#define NR_ROTATIONS 4

#define BOARD_WIDTH 8
#define BOARD_HEIGHT 10

#define WALL_CHAR '*'
#define TILE_CHAR '#'
#define EMPTY_CHAR ' '

#define SCORE_POSITION_ON_SCREEN_ROW 2
#define SCORE_POSITION_ON_SCREEN_COL 12

#define NR_SQUARES_IN_TILE 4

#define LEFT 0
#define RIGHT 1
#define BOTTOM 2

#define NR_SQUARES_PER_TILE 4

#define SCORE_INC 1

#define INIT_FALL_INTERVAL 0.1

/* reprezinta o pozitie in spatiul lume (al tablei)
 */ 
typedef struct
{
	int x; 
	int y; 
} pos; 

/* reprezinta o pozitie in spatiul ecran
 */
typedef struct
{
	unsigned int r; 
	unsigned int c; 
} scr_pos; 

/* reprezinta o dimensiune a unui obiect 
 * (width si height - latime si inaltime)
 */
typedef struct
{
	unsigned int w; 
	unsigned int h; 
} size; 

/* defineste un patrat din componenta unei piese; 
 * reprezinta coordonatele in spatiul local (al piesei)
 * pentru un patrat din componenta piesei; 
 */
typedef struct
{
	int x; 
	int y; 
} point; 

/* structura reprezinta definitia unei piese; 
 * o piesa este alcatuita din 4 patratele, 
 * reprezentate prin coordonatele in spatiul obiect
 */
typedef struct 
{
	point sq[NR_SQUARES_PER_TILE]; 
} tile; 


typedef struct
{
	int tile_id; 
	int rotation; 
} tile_info; 

/* structura pastreaza informatiile legate de board - spatiul de lucru; 
 */
typedef struct 
{
	size board_size; 
	bool data[BOARD_HEIGHT][BOARD_WIDTH]; 
} board; 

typedef struct
{
	size screen_size; // dimensiunile terminalului
	scr_pos board_pos; 
	scr_pos score_pos; 
} screen; 

typedef struct
{
	tile_info current_tile; 
	// tile_info next_tile; 
	pos tile_position; 
	unsigned int score; 
	board *bd; 
	screen *scr; 
	tile *tiles; 
} game_state; 


/* functia initializeaza vectorul de piese
 */
void init_tiles(tile *tiles)
{
	tiles[0].sq[0].x = 0; 
	tiles[0].sq[0].y = 0; 
	tiles[0].sq[1].x = 0; 
	tiles[0].sq[1].y = 1; 
	tiles[0].sq[2].x = 1; 
	tiles[0].sq[2].y = 1; 
	tiles[0].sq[3].x = 1; 
	tiles[0].sq[3].y = 0; 
	tiles[1].sq[0].x = 0; 
	tiles[1].sq[0].y = 0; 
	tiles[1].sq[1].x = -1; 
	tiles[1].sq[1].y = 1; 
	tiles[1].sq[2].x = -1; 
	tiles[1].sq[2].y = 0; 
	tiles[1].sq[3].x = 1; 
	tiles[1].sq[3].y = 0; 
	tiles[2].sq[0].x = 0; 
	tiles[2].sq[0].y = 0; 
	tiles[2].sq[1].x = -1; 
	tiles[2].sq[1].y = 0; 
	tiles[2].sq[2].x = 1; 
	tiles[2].sq[2].y = 1; 
	tiles[2].sq[3].x = 1; 
	tiles[2].sq[3].y = 0; 
	tiles[3].sq[0].x = 0; 
	tiles[3].sq[0].y = 0; 
	tiles[3].sq[1].x = -1; 
	tiles[3].sq[1].y = 0; 
	tiles[3].sq[2].x = 0; 
	tiles[3].sq[2].y = 1; 
	tiles[3].sq[3].x = 1; 
	tiles[3].sq[3].y = 1; 
	tiles[4].sq[0].x = 0; 
	tiles[4].sq[0].y = 0; 
	tiles[4].sq[1].x = -1; 
	tiles[4].sq[1].y = 1; 
	tiles[4].sq[2].x = 0; 
	tiles[4].sq[2].y = 1; 
	tiles[4].sq[3].x = 1; 
	tiles[4].sq[3].y = 0; 
	tiles[5].sq[0].x = 0; 
	tiles[5].sq[0].y = 0; 
	tiles[5].sq[1].x = -1; 
	tiles[5].sq[1].y = 0; 
	tiles[5].sq[2].x = 0; 
	tiles[5].sq[2].y = 1; 
	tiles[5].sq[3].x = 1; 
	tiles[5].sq[3].y = 0; 
	tiles[6].sq[0].x = 0; 
	tiles[6].sq[0].y = 0; 
	tiles[6].sq[1].x = 0; 
	tiles[6].sq[1].y = 1; 
	tiles[6].sq[2].x = 0; 
	tiles[6].sq[2].y = 2; 
	tiles[6].sq[3].x = 0; 
	tiles[6].sq[3].y = -1; 
}

void draw_on_screen(scr_pos pos, char c)
{
	move(pos.r, pos.c); 
	addch(c); 
}

scr_pos world_to_screen(pos p, scr_pos board_pos, size board_size)
{
	scr_pos screen_position; 
	screen_position.r = board_pos.r + board_size.h - p.y; 
	screen_position.c = board_pos.c + p.x + 1; 
	return screen_position;
}

void draw_on_board(pos p, char c, game_state *state)
{
	scr_pos screen_position = world_to_screen(p, state->scr->board_pos, state->bd->board_size); 
	draw_on_screen(screen_position, c); 
}

/* afiseaza conturul spatiului de joc; 
 */ 
void display_board(board *bd, screen *scr)
{
	int i; 
	scr_pos pos; 
	
	// afiseaza linia de sus
	pos.r = scr->board_pos.r; 
	for (i = 0; i < bd->board_size.w + 2; i++)
	{
		pos.c = scr->board_pos.c + i; 
		draw_on_screen(pos, WALL_CHAR); 
	}

	// afiseaza linia de jos
	pos.r = scr->board_pos.r + bd->board_size.h + 1; 
	for (i = 0; i < bd->board_size.w + 2; i++)
	{
		pos.c = scr->board_pos.c + i; 
		draw_on_screen(pos, WALL_CHAR); 
	}

	// afiseaza linia din stanga
	pos.c = scr->board_pos.c; 
	for (i = 0; i < bd->board_size.h + 2; i++)
	{
		pos.r = scr->board_pos.r + i; 
		draw_on_screen(pos, WALL_CHAR); 
	}

	// afiseaza linia din dreapta
	pos.c = scr->board_pos.c + bd->board_size.w + 1; 
	for (i = 0; i < bd->board_size.h + 2; i++)
	{
		pos.r = scr->board_pos.r + i; 
		draw_on_screen(pos, WALL_CHAR); 
	}
}

/* functia initializeaza tabla de lucru
 * cu valoarea false, deoarece initial aceasta este goala
 */
void init_board_table(board *bd)
{
	//setez dimensiunile board-ului (spatiului de lucru)
	bd->board_size.w = BOARD_WIDTH; 
	bd->board_size.h = BOARD_HEIGHT; 

	int i, j; 
	for (i = 0; i < bd->board_size.h; i++)
	{
		for (j = 0; j < bd->board_size.w; j++)
		{
			bd->data[i][j] = false; 
		}
	}
}


void init_screen(screen *scr, int nrows, int ncols)
{
	scr->screen_size.w = ncols; 
	scr->screen_size.h = nrows; 
	scr->board_pos.r = 0; 
	scr->board_pos.c = 0; 
	scr->score_pos.r = SCORE_POSITION_ON_SCREEN_ROW; 
	scr->score_pos.c = SCORE_POSITION_ON_SCREEN_COL; 
}


tile_info get_random_tile()
{
	tile_info ti; 
	ti.tile_id = rand() % NR_TILES; 
	ti.rotation = rand() % NR_ROTATIONS; 
	return ti; 
}

pos initial_tile_position(board *bd)
{
	pos init_tile_pos; 
	init_tile_pos.x = bd->board_size.w / 2; 
	init_tile_pos.y = bd->board_size.h - 1; 
	return init_tile_pos; 
}

void init_game_state(game_state *state, board *bd, screen *scr, tile *tiles)
{
	state->score = 0; 
	state->current_tile = get_random_tile(); 
	state->tile_position = initial_tile_position(bd); 
	state->bd = bd; 
	state->scr = scr; 
	state->tiles = tiles; 
}

//executa actiuni de initializare a variabilelor specifice jocului
void init(tile *tiles, board *bd, screen *scr, game_state *state, int nrows, int ncols)
{
	init_tiles(tiles); 
	init_board_table(bd); 
	init_screen(scr, nrows, ncols); 
	init_game_state(state, bd, scr, tiles); 
}

int get_key_pressed()
{
	int ch = getch(); 
	if (ch != ERR)
	{
		return ch; 
	}
	return NO_KEY_PRESSED; 
}

point rotate(point p, int rotation)
{
	point p2; 

	switch(rotation)
	{
		case 0:
			p2.x = p.x; 
			p2.y = p.y; 
			break; 
		case 1:
			p2.y = p.x; 
			p2.x = -p.y; 
			break; 
		case 2:
			p2.x = -p.x; 
			p2.y = -p.y; 
			break; 
		case 3:
			p2.y = -p.x; 
			p2.x = p.y; 
			break; 
	}
	return p2; 
}

pos translate(point p, pos t)
{
	pos p2; 
	p2.x = p.x + t.x; 
	p2.y = p.y + t.y; 
	return p2; 
}

pos local_to_world(point p, game_state *state)
{
	return translate(p, state->tile_position); 
}

pos get_world_pos_for_square(int i, game_state *state)
{
	pos p; 
	int tile_id = state->current_tile.tile_id; 
	int rotation = state->current_tile.rotation; 

	point pt = state->tiles[tile_id].sq[i]; 
	pt = rotate(pt, rotation); 
	p = local_to_world(pt, state); 

	return p; 
}

void render_current_tile(game_state *state)
{
	pos p2; 
	int i; 

	for (i = 0; i < NR_SQUARES_PER_TILE; i++)
	{
		// randeaza patratul i din componenta piesei
		p2 = get_world_pos_for_square(i, state); 
		draw_on_board(p2, TILE_CHAR, state); 
	}
}

pos get_rotation_allowed_pos(game_state *state)
{
	// testez pentru peretele din stanga
	pos p1 = state->tile_position; 

	int i; 
	for (i = 0; i < NR_SQUARES_PER_TILE; i++)
	{
		// TODO:
	}

	return p1; 

}

void rotate_current_tile(game_state *state)
{
	int r = state->current_tile.rotation; 
	int new_rotation = (r + 1) % NR_ROTATIONS; 

	// verific daca se poate roti (dupa rotire nu atinge peretii)
	state->tile_position = get_rotation_allowed_pos(state); 

	state->current_tile.rotation = new_rotation; 
}

bool has_square(board *bd, pos p)
{
	int r = p.y; 
	int c = p.x; 
	return bd->data[r][c]; 
}

void set_square(board *bd, pos p)
{
	int r, c; 
	r = p.y; 
	c = p.x; 
	bd->data[r][c] = true; 
}

void clear_square(board *bd, pos p)
{
	int r, c; 
	r = p.y; 
	c = p.x; 
	bd->data[r][c] = false; 
}

void render_board(game_state *state)
{
	int i, j; 
	pos p; 
	for (i = 0; i < state->bd->board_size.h; i++)
	{
		p.y = i; 
		for (j = 0; j < state->bd->board_size.w; j++)
		{
			p.x = j; 
			if (has_square(state->bd, p))
			{
				draw_on_board(p, TILE_CHAR, state); 
			}
			else
			{
				draw_on_board(p, EMPTY_CHAR, state); 
			}
		}
	}
}

pos move_pos(pos p, int side)
{
	pos p2; 
	p2.x = p.x; 
	p2.y = p.y; 
	switch(side)
	{
		case LEFT:
			p2.x--; 
			break; 
		case RIGHT:
			p2.x++; 
			break; 
		case BOTTOM:
			p2.y--; 
			break; 
	}
	return p2; 
}

bool verify_free(int side, game_state *state)
{
	int i; 
	point pt; 
	int tile_id; 
	int rotation; 
	pos p_crtile;  
	pos p_moved; 
	pos p2; 

	tile_id = state->current_tile.tile_id; 
	rotation = state->current_tile.rotation; 
	p_crtile = state->tile_position; 
	p_moved = move_pos(p_crtile, side); 
	for (i = 0; i < NR_SQUARES_PER_TILE; i++)
	{
		pt = state->tiles[tile_id].sq[i]; 
		pt = rotate(pt, rotation); 
		p2 = translate(pt, p_moved); 
		if (has_square(state->bd, p2))
		{
			return false; 
		}
	}
	return true; 
}

bool is_near_border(pos p, int side, game_state *state)
{
	if (side == LEFT)
	{
		if (p.x <= 0)
		{
			return true; 
		}
	}
	else if (side == RIGHT)
	{
		if (p.x >= state->bd->board_size.w - 1)
		{
			return true; 
		}
	}
	else if (side == BOTTOM)
	{
		if (p.y <= 0)
		{
			return true; 
		}
	}
	return false; 
}

bool verify_border(int side, game_state *state)
{
	int i; 
	pos p; 

	for (i = 0; i < NR_SQUARES_PER_TILE; i++)
	{
		p = get_world_pos_for_square(i, state); 
		
		if (is_near_border(p, side, state))
		{
			return false; 
		}
	}
	return true; 
}

bool can_move(int side, game_state *state)
{
	return verify_border(side, state) && verify_free(side, state); 
}

void move_current_tile(int side, game_state *state)
{
	if (!can_move(side, state))
	{
		return; 
	}
	state->tile_position = move_pos(state->tile_position, side); 
}

void set_new_tile(game_state *state)
{
	state->current_tile = get_random_tile(); 
	state->tile_position = initial_tile_position(state->bd); 
}

void fix_tile(game_state *state)
{
	int i;
	pos p; 

	for (i = 0; i < NR_SQUARES_PER_TILE; i++)
	{
		p = get_world_pos_for_square(i, state); 
		set_square(state->bd, p); 
	}
}

void get_completed_lines(game_state *state, bool *completed_lines)
{
	int nr_lines = state->bd->board_size.h; 
	int nr_cols = state->bd->board_size.w; 
	int i, j; 
	pos p; 

	for (i = 0; i < nr_lines; i++)
	{
		p.y = i; 
		completed_lines[i] = true; 

		for (j = 0; j < nr_cols; j++)
		{
			p.x = j; 
			if (!has_square(state->bd, p))
			{
				completed_lines[i] = false; 
				break; 
			}
		}
	}
}

void delete_line(int i, board *bd)
{
	int j; 
	int nr_cols = bd->board_size.w; 
	pos p; 
	p.y = i; 

	for (j = 0; j < nr_cols; j++)
	{
		p.x = j; 
		clear_square(bd, p); 
	}
}

void move_down_all(int i, board *bd)
{
	int j, k; 
	int nr_lines = bd->board_size.h; 
	int nr_cols = bd->board_size.w; 

	for (j = i; j < nr_lines - 1; j++)
	{
		for (k = 0; k < nr_cols; k++)
		{
			bd->data[j][k] = bd->data[j + 1][k]; 
		}
	}
}

void increment_score(game_state *state)
{
	state->score += SCORE_INC; 
}

void manage_completed_lines(game_state *state)
{
	int i; 

	int nr_lines = state->bd->board_size.h; 
	bool completed_lines[nr_lines]; 
	get_completed_lines(state, completed_lines); 
	//pentru fiecare linie completata
	for (i = 0; i < nr_lines; i++)
	{
		if (completed_lines[i])
		{
			//sterge liniile completate
			delete_line(i, state->bd); 
			
			//muta restul patratelelor in jos
			move_down_all(i, state->bd); 

			//actualizeaza scorul
			increment_score(state); 
		}
	}
}

void display_score(game_state *state)
{
	scr_pos screen_position = state->scr->score_pos; 
	unsigned int r = screen_position.r; 
	unsigned int c = screen_position.c; 
	char score_string[20]; 
	int score = state->score; 
	sprintf(score_string, "Score: %d", score); 
	mvaddstr(r, c, score_string); 
}

bool is_game_over(game_state *state)
{
	return (state->tile_position.y >= state->bd->board_size.h - 2); 
}

void game_over()
{
	mvaddstr(0, 0, "GAME OVER"); 
	get_key_pressed(); 
}

//------------main----------------------------------
int main(int argc, char **argv)
{

	WINDOW *wnd = initscr(); 
	int nrows, ncols; 
	getmaxyx(wnd, nrows, ncols); 
	curs_set(0); 
	clear(); 
	noecho(); 
	cbreak(); 
	nodelay(wnd, TRUE); 

	time_t last_move = 0; 
	time_t interval = INIT_FALL_INTERVAL;  
	time_t current_time; 

	char p; 

	//declaratii variabile
	tile tiles[NR_TILES]; //vectorul cu piese
	board bd; 
	screen scr; 
	game_state state; 

	//initializare
	srand(time(NULL)); 
	init(tiles, &bd, &scr, &state, nrows, ncols); 
	display_board(&bd, &scr); 


	// cat timp nu s-a terminat jocul
	while (true)
	{
		p = get_key_pressed(); 

		if (p != NO_KEY_PRESSED)
		{
			if (tolower(p) == 'q')
				break; 

			// daca am apasat stanga
			if (tolower(p) == 'a')
			{
				//deplasez piesa curenta la stanga
				move_current_tile(LEFT, &state); 
			}

			// daca am apasat dreapta
			if (tolower(p) == 'd')
			{
				// deplasez piesa curenta la dreapta
				move_current_tile(RIGHT, &state); 
			}

			//daca am apasat in sus
			if (tolower(p) == 'w')
			{
				// rotesc piesa
				rotate_current_tile(&state); 
			}

			if (tolower(p) == 's')
			{
				// cobor piesa
				// TODO: 
			}
		}

		current_time = time(NULL); 

		// execut la fiecare iteratie la un interval de timp dat de valoarea lui interval
		if (current_time - last_move > interval)
		{
			last_move = current_time; 

			//daca piesa curenta se mai poate deplasa in jos
			if (can_move(BOTTOM, &state))
			{
				//deplaseaza piesa curenta in jos cu o pozitie
				move_current_tile(BOTTOM, &state); 
			}

			//daca piesa curenta nu se mai poate deplasa in jos
			else
			{
				//daca jocul s-a terminat
				if (is_game_over(&state))
				{
					//iesi din joc
					game_over(); 
					break; 
				}

				// fixez piesa in pozitia in care a ramas
				fix_tile(&state); 

				manage_completed_lines(&state); 

				//alege o noua piesa si plaseaz-o in pozitia de plecare
				set_new_tile(&state); 
			}

		}

		// cod de randare
		render_board(&state); 
		render_current_tile(&state); 
		display_score(&state); 

		refresh(); 

		//display_board(&bd, &scr); 
	}

	endwin(); 
	return 0; 
}
