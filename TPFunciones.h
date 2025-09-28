#ifndef TPFUNCIONES_H_INCLUDED
#define TPFUNCIONES_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#ifdef __MINGW32__
    #define SDL_MAIN_HANDLED
    #include <SDL_main.h>
#endif

#include <SDL.h>

// Definiciones de constantes
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 700
#define CELL_SIZE 60
#define STATUS_BAR_HEIGHT 100
#define MAX_BOARD_SIZE 10

// Enumeraciones
typedef enum {
    EASY = 0,
    NORMAL = 1,
    HARD = 2
} Difficulty;

typedef enum {
    EMPTY = 0,
    KNIGHT = 1,
    DEMON = 2
} CellType;

typedef enum {
    MENU_SCREEN = 0,
    GAME_SCREEN = 1,
    GAME_OVER_SCREEN = 2
} GameScreen;

// Estructuras
typedef struct {
    int board_size;
    int time_limit;
    int **board;
    int remaining_time;
    int knight_count;
    int demon_count;
    int pikas_remaining;
    float current_meter;
    bool game_over;
    bool level_complete;
    int selected_i, selected_j;
} LevelState;

typedef struct {
    Difficulty difficulty;
    float global_meter;
    int total_levels_played;
    LevelState *current_level;
    bool game_finished;
    bool player_won;
    GameScreen current_screen;
} GameState;

// Colores del juego (extern para acceso global)
extern SDL_Color colors[];

// Prototipos de funciones

// Gestión del juego
GameState* init_game(Difficulty difficulty);
void free_game(GameState *game);
void reset_game(GameState *game, Difficulty difficulty);

// Gestión de niveles
LevelState* init_level(Difficulty difficulty, float start_meter);
void free_level(LevelState *level);
void generate_board_from_meter(LevelState *level);
void update_current_meter(LevelState *level);
void process_level_completion(GameState *game);

// Mecánicas del juego
void invert_adjacent_cells(LevelState *level, int i, int j);
bool use_pika(LevelState *level, int i, int j);
bool check_level_complete(LevelState *level);
void move_selection(LevelState *level, int di, int dj);

// Renderizado
void draw_board(SDL_Renderer *renderer, LevelState *level);
void draw_real_time_meter(SDL_Renderer *renderer, LevelState *level, GameState *game);
void draw_game_info(SDL_Renderer *renderer, GameState *game);
void draw_menu(SDL_Renderer *renderer, Difficulty selected_difficulty);
void draw_game_over(SDL_Renderer *renderer, bool player_won);
void draw_text_block(SDL_Renderer *renderer, int x, int y, const char* text);
#endif // TPFUNCIONES_H_INCLUDED
