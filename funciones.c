#include "TPFunciones.h"
SDL_Color colors[] = {
    {30, 30, 30},        // EMPTY - Gris oscuro
    {255, 255, 0},       // KNIGHT - Amarillo caballero
    {200, 0, 0},         // DEMON - Rojo demonio
    {0, 100, 255},       // PIKA - Azul
    {0, 200, 0},         // STATUS_KNIGHT - Verde
    {200, 0, 0},         // STATUS_DEMON - Rojo
    {150, 150, 150},      // NEUTRAL - Gris
    {255, 255, 255},     // WHITE - Blanco
    {255, 200, 0}        // GOLD - Oro para la seleccion
};

// Gestión del juego
GameState* init_game(Difficulty difficulty) {
    GameState *game = malloc(sizeof(GameState));
    if (!game) return NULL;

    game->difficulty = difficulty;
    game->global_meter = 0.5f;
    game->total_levels_played = 0;
    game->game_finished = false;
    game->player_won = false;
    game->current_screen = MENU_SCREEN;
    game->current_level = NULL;

    return game;
}

void free_game(GameState *game) {
    if (game) {
        if (game->current_level) {
            free_level(game->current_level);
        }
        free(game);
    }
}

void reset_game(GameState *game, Difficulty difficulty) {
    if (game->current_level) {
        free_level(game->current_level);
    }

    game->difficulty = difficulty;
    game->global_meter = 0.5f;
    game->total_levels_played = 0;
    game->game_finished = false;
    game->player_won = false;
    game->current_level = init_level(difficulty, game->global_meter);

    if (game->current_level) {
        generate_board_from_meter(game->current_level);
    }
}

// Gestión de niveles
LevelState* init_level(Difficulty difficulty, float start_meter) {
    LevelState *level = malloc(sizeof(LevelState));
    if (!level) return NULL;

    switch (difficulty) {
        case EASY:
            level->board_size = 6;
            level->time_limit = 120;
            level->pikas_remaining = 3;
            break;
        case NORMAL:
            level->board_size = 8;
            level->time_limit = 180;
            level->pikas_remaining = 2;
            break;
        case HARD:
            level->board_size = 10;
            level->time_limit = 240;
            level->pikas_remaining = 1;
            break;
    }

    level->board = (int**)malloc(level->board_size * sizeof(int*));
    for (int i = 0; i < level->board_size; i++) {
        level->board[i] = (int*)malloc(level->board_size * sizeof(int));
    }

    level->remaining_time = level->time_limit;
    level->current_meter = start_meter;
    level->game_over = false;
    level->level_complete = false;
    level->selected_i = 0;
    level->selected_j = 0;

    return level;
}

void free_level(LevelState *level) {
    if (level) {
        for (int i = 0; i < level->board_size; i++) {
            free(level->board[i]);
        }
        free(level->board);
        free(level);
    }
}

void generate_board_from_meter(LevelState *level) {
    level->knight_count = 0;
    level->demon_count = 0;

    int total_cells = level->board_size * level->board_size;
    int target_pieces = total_cells;

    int target_knights = (int)(target_pieces * level->current_meter);
    int target_demons = target_pieces - target_knights;

    for (int i = 0; i < level->board_size; i++) {
        for (int j = 0; j < level->board_size; j++) {
            if (level->knight_count + level->demon_count < target_pieces) {
                if (level->knight_count < target_knights) {
                    level->board[i][j] = KNIGHT;
                    level->knight_count++;
                } else {
                    level->board[i][j] = DEMON;
                    level->demon_count++;
                }
            } else {
                level->board[i][j] = EMPTY;
            }
        }
    }
}

void update_current_meter(LevelState *level) {
    int total_pieces = level->knight_count + level->demon_count;

    if (total_pieces == 0) {
        level->current_meter = 0.5f;
    } else {
        level->current_meter = (float)level->knight_count / total_pieces;
    }

    if (level->current_meter >= 0.95f) {
        level->level_complete = true;
    } else if (level->current_meter <= 0.05f) {
        level->level_complete = true;
    }
}

void process_level_completion(GameState *game) {
    if (game->current_level->current_meter >= 0.95f) {
        game->game_finished = true;
        game->player_won = true;
    } else if (game->current_level->current_meter <= 0.05f) {
        game->game_finished = true;
        game->player_won = false;
    } else {
        game->global_meter = game->current_level->current_meter;
        game->total_levels_played++;

        free_level(game->current_level);
        game->current_level = init_level(game->difficulty, game->global_meter);
        if (game->current_level) {
            generate_board_from_meter(game->current_level);
        }
    }
}

// Mecánicas del juego
void invert_adjacent_cells(LevelState *level, int i, int j) {
    int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    for (int d = 0; d < 4; d++) {
        int ni = i + directions[d][0];
        int nj = j + directions[d][1];

        if (ni >= 0 && ni < level->board_size && nj >= 0 && nj < level->board_size) {
            if (level->board[ni][nj] == KNIGHT) {
                level->board[ni][nj] = DEMON;
                level->knight_count--;
                level->demon_count++;
            } else if (level->board[ni][nj] == DEMON) {
                level->board[ni][nj] = KNIGHT;
                level->knight_count++;
                level->demon_count--;
            }
        }
    }

    update_current_meter(level);
}

bool use_pika(LevelState *level, int i, int j) {
    if (level->pikas_remaining <= 0) return false;
    if (i < 0 || i >= level->board_size || j < 0 || j >= level->board_size) return false;
    if (level->board[i][j] == EMPTY) return false;

    if (level->board[i][j] == KNIGHT) {
        level->board[i][j] = DEMON;
        level->knight_count--;
        level->demon_count++;
    } else if (level->board[i][j] == DEMON) {
        level->board[i][j] = KNIGHT;
        level->knight_count++;
        level->demon_count--;
    }

    level->pikas_remaining--;
    update_current_meter(level);

    return true;
}

bool check_level_complete(LevelState *level) {
    return level->level_complete;
}

void move_selection(LevelState *level, int di, int dj) {
    int new_i = level->selected_i + di;
    int new_j = level->selected_j + dj;

    if (new_i >= 0 && new_i < level->board_size && new_j >= 0 && new_j < level->board_size) {
        level->selected_i = new_i;
        level->selected_j = new_j;
    }
}

// Función para dibujar bloques de texto simple (simulando texto)
void draw_text_block(SDL_Renderer *renderer, int x, int y, const char* text) {
    // Esta es una simulación muy básica - en un juego real usarías texturas pre-renderizadas
    // Por ahora solo dibujamos un rectángulo con borde para representar "texto"
    int width = strlen(text) * 8 + 10;
    int height = 20;

    SDL_Rect bg_rect = {x, y, width, height};
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_RenderFillRect(renderer, &bg_rect);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &bg_rect);

    // En un juego real, aquí cargarías texturas pre-renderizadas de cada texto
}

// Renderizado
void draw_board(SDL_Renderer *renderer, LevelState *level) {
    int offset_x = (WINDOW_WIDTH - level->board_size * CELL_SIZE) / 2;
    int offset_y = STATUS_BAR_HEIGHT + 20;

    for (int i = 0; i < level->board_size; i++) {
        for (int j = 0; j < level->board_size; j++) {
            SDL_Rect cell = {
                offset_x + j * CELL_SIZE,
                offset_y + i * CELL_SIZE,
                CELL_SIZE,
                CELL_SIZE
            };

            SDL_Color color = colors[EMPTY];
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
            SDL_RenderFillRect(renderer, &cell);

            if (level->board[i][j] != EMPTY) {
                color = colors[level->board[i][j]];
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);

                int center_x = offset_x + j * CELL_SIZE + CELL_SIZE / 2;
                int center_y = offset_y + i * CELL_SIZE + CELL_SIZE / 2;
                int radius = CELL_SIZE / 3;

                for (int w = -radius; w <= radius; w++) {
                    for (int h = -radius; h <= radius; h++) {
                        if (w*w + h*h <= radius*radius) {
                            SDL_RenderDrawPoint(renderer, center_x + w, center_y + h);
                        }
                    }
                }
            }

            if (level->selected_i == i && level->selected_j == j) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                SDL_RenderDrawRect(renderer, &cell);
                SDL_Rect inner = {cell.x + 2, cell.y + 2, cell.w - 4, cell.h - 4};
                SDL_RenderDrawRect(renderer, &inner);
            }

            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            SDL_RenderDrawRect(renderer, &cell);
        }
    }
}

void draw_real_time_meter(SDL_Renderer *renderer, LevelState *level, GameState *game) {
    // Fondo de la barra
    SDL_Rect status_bg = {0, 0, WINDOW_WIDTH, STATUS_BAR_HEIGHT};
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_RenderFillRect(renderer, &status_bg);

    // Barra de progreso principal
    int bar_width = WINDOW_WIDTH - 200;
    int bar_height = 30;
    int bar_x = 100;
    int bar_y = 20;

    // Fondo de la barra
    SDL_Rect progress_bg = {bar_x, bar_y, bar_width, bar_height};
    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
    SDL_RenderFillRect(renderer, &progress_bg);

    // Progreso actual (en tiempo real)
    int progress_width = (int)(bar_width * level->current_meter);
    SDL_Rect knight_progress = {bar_x, bar_y, progress_width, bar_height};
    SDL_SetRenderDrawColor(renderer, colors[KNIGHT].r, colors[KNIGHT].g, colors[KNIGHT].b, 255);
    SDL_RenderFillRect(renderer, &knight_progress);

    // Indicador de posición actual
    int indicator_x = bar_x + progress_width - 5;
    SDL_Rect indicator = {indicator_x, bar_y - 5, 10, bar_height + 10};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &indicator);

    // Marcas de extremos (victoria/derrota)
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderDrawLine(renderer, bar_x + (int)(bar_width * 0.95f), bar_y - 5,
                      bar_x + (int)(bar_width * 0.95f), bar_y + bar_height + 5);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderDrawLine(renderer, bar_x + (int)(bar_width * 0.05f), bar_y - 5,
                      bar_x + (int)(bar_width * 0.05f), bar_y + bar_height + 5);

    // Borde de la barra
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &progress_bg);

    // Etiquetas de los extremos
    SDL_Rect knight_label = {bar_x - 40, bar_y, 35, bar_height};
    SDL_SetRenderDrawColor(renderer, colors[KNIGHT].r, colors[KNIGHT].g, colors[KNIGHT].b, 255);
    SDL_RenderFillRect(renderer, &knight_label);

    SDL_Rect demon_label = {bar_x + bar_width + 5, bar_y, 35, bar_height};
    SDL_SetRenderDrawColor(renderer, colors[DEMON].r, colors[DEMON].g, colors[DEMON].b, 255);
    SDL_RenderFillRect(renderer, &demon_label);
}

void draw_game_info(SDL_Renderer *renderer, GameState *game) {
    if (!game->current_level) return;

    // Tiempo restante (representado con barra)
    int time_bar_width = 150;
    int time_progress = (time_bar_width * game->current_level->remaining_time) / game->current_level->time_limit;

    SDL_Rect time_bg = {20, 60, time_bar_width, 20};
    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
    SDL_RenderFillRect(renderer, &time_bg);

    SDL_Rect time_fill = {20, 60, time_progress, 20};
    SDL_SetRenderDrawColor(renderer, 0, 200, 255, 255);
    SDL_RenderFillRect(renderer, &time_fill);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &time_bg);

    // Pikas restantes (representadas con iconos)
    for (int i = 0; i < game->current_level->pikas_remaining; i++) {
        SDL_Rect pika_icon = {WINDOW_WIDTH - 60 - i * 30, 60, 20, 20};
        SDL_SetRenderDrawColor(renderer, colors[3].r, colors[3].g, colors[3].b, 255);
        SDL_RenderFillRect(renderer, &pika_icon);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &pika_icon);
    }

    // Nivel actual (representado con número de cuadrados)
    SDL_Rect level_bg = {WINDOW_WIDTH / 2 - 40, 60, 80, 20};
    SDL_SetRenderDrawColor(renderer, 60, 60, 100, 255);
    SDL_RenderFillRect(renderer, &level_bg);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &level_bg);

    // Puntos para representar el número de nivel
    for (int i = 0; i < game->total_levels_played + 1; i++) {
        if (i < 5) { // Máximo 5 puntos visibles
            SDL_Rect level_dot = {WINDOW_WIDTH / 2 - 30 + i * 15, 65, 8, 8};
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderFillRect(renderer, &level_dot);
        }
    }
}

void draw_menu(SDL_Renderer *renderer, Difficulty selected_difficulty) {
    // Fondo
    SDL_SetRenderDrawColor(renderer, 20, 20, 50, 255);
    SDL_RenderClear(renderer);

    // Título
    SDL_Rect title_bg = {WINDOW_WIDTH/2 - 150, 50, 300, 60};
    SDL_SetRenderDrawColor(renderer, 80, 80, 120, 255);
    SDL_RenderFillRect(renderer, &title_bg);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &title_bg);

    // Opciones de dificultad
    const char* difficulties[] = {"FACIL", "NORMAL", "DIFICIL"};
    const char* descriptions[] = {
        "6x6 - 2min - 3 PIKAS",
        "8x8 - 3min - 2 PIKAS",
        "10x10 - 4min - 1 PIKA"
    };

    for (int i = 0; i < 3; i++) {
        SDL_Rect option_bg = {WINDOW_WIDTH/2 - 100, 150 + i * 80, 200, 60};

        if (i == selected_difficulty) {
            SDL_SetRenderDrawColor(renderer, 100, 100, 200, 255); // Seleccionado
        } else {
            SDL_SetRenderDrawColor(renderer, 60, 60, 100, 255); // No seleccionado
        }

        SDL_RenderFillRect(renderer, &option_bg);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &option_bg);

        // Indicador de selección
        if (i == selected_difficulty) {
            SDL_Rect selector = {WINDOW_WIDTH/2 - 110, 155 + i * 80, 10, 50};
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderFillRect(renderer, &selector);
        }

        // Icono de dificultad (cuadrados de diferentes tamaños)
        int size = 8 + i * 4;
        SDL_Rect diff_icon = {WINDOW_WIDTH/2 - 80, 165 + i * 80, size, size};
        SDL_SetRenderDrawColor(renderer, 200, 200, 100, 255);
        SDL_RenderFillRect(renderer, &diff_icon);

        // Iconos de pika para cada dificultad
        for (int p = 0; p < 3 - i; p++) {
            SDL_Rect pika_icon = {WINDOW_WIDTH/2 + 40 + p * 15, 165 + i * 80, 10, 10};
            SDL_SetRenderDrawColor(renderer, colors[3].r, colors[3].g, colors[3].b, 255);
            SDL_RenderFillRect(renderer, &pika_icon);
        }
    }

    // Instrucciones
    SDL_Rect inst_bg = {WINDOW_WIDTH/2 - 200, 400, 400, 40};
    SDL_SetRenderDrawColor(renderer, 40, 40, 80, 255);
    SDL_RenderFillRect(renderer, &inst_bg);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &inst_bg);
}

void draw_game_over(SDL_Renderer *renderer, bool player_won) {
    // Fondo
    if (player_won) {
        SDL_SetRenderDrawColor(renderer, 20, 50, 20, 255); // Verde para victoria
    } else {
        SDL_SetRenderDrawColor(renderer, 50, 20, 20, 255); // Rojo para derrota
    }
    SDL_RenderClear(renderer);

    // Panel de resultado
    SDL_Rect result_bg = {WINDOW_WIDTH/2 - 150, 150, 300, 100};
    SDL_SetRenderDrawColor(renderer, 30, 30, 60, 255);
    SDL_RenderFillRect(renderer, &result_bg);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &result_bg);

    // Icono de resultado
    SDL_Rect icon_bg = {WINDOW_WIDTH/2 - 20, 170, 40, 40};
    if (player_won) {
        SDL_SetRenderDrawColor(renderer, colors[KNIGHT].r, colors[KNIGHT].g, colors[KNIGHT].b, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, colors[DEMON].r, colors[DEMON].g, colors[DEMON].b, 255);
    }
    SDL_RenderFillRect(renderer, &icon_bg);

    // Instrucciones
    SDL_Rect inst_bg = {WINDOW_WIDTH/2 - 150, 280, 300, 40};
    SDL_SetRenderDrawColor(renderer, 40, 40, 80, 255);
    SDL_RenderFillRect(renderer, &inst_bg);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &inst_bg);
}

// Resto de funciones permanecen igual...
void print_game_state(const GameState *game) {
    printf("=== ESTADO DEL JUEGO ===\n");
    printf("Dificultad: %d\n", game->difficulty);
    printf("Medidor Global: %.1f%%\n", game->global_meter * 100);
    printf("Niveles Jugados: %d\n", game->total_levels_played);
    printf("Juego Terminado: %s\n", game->game_finished ? "Sí" : "No");
    printf("Jugador Ganó: %s\n", game->player_won ? "Sí" : "No");
    printf("=======================\n");
}

void print_level_state(const LevelState *level) {
    printf("=== ESTADO DEL NIVEL ===\n");
    printf("Tamaño: %dx%d\n", level->board_size, level->board_size);
    printf("Tiempo: %d/%d\n", level->remaining_time, level->time_limit);
    printf("Caballeros: %d\n", level->knight_count);
    printf("Demonios: %d\n", level->demon_count);
    printf("Pikas: %d\n", level->pikas_remaining);
    printf("Medidor: %.1f%%\n", level->current_meter * 100);
    printf("Nivel Completado: %s\n", level->level_complete ? "Sí" : "No");
    printf("=======================\n");
}

Difficulty show_difficulty_menu(void) {
    return EASY; // Ya no se usa la consola
}
