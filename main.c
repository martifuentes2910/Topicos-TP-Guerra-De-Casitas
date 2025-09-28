#include "TPFunciones.h"


/**
Pablo Soligo. Plantilla de proyecto codeblocks para juego de la vida.
Funciona con mingw 64 bits y no requiere tener instalado SDL.
Los fuentes son multiplataforma (windows/linux Debian;Ubuntu). Para ubuntu se
requiere de diferente configuración de proyecto.
*/

//CUANDO TERMINE EL NIVEL TIENE QUE VOLVER AL MENU, FALTA QUE MUESTRE EL MENU O POR CONSOLA O POR GRAFICOS
// HAY QUE INCORPORAR LA BIBLIOTECA SDL TFF QUE INCLUYE EL TEXTO PARA EL DISPLAY
///Y AÑADIR TODOS LOS TEXTOS Y CONTADORES
//VOLVER A INCLUIR EL MOVIMIENTO POR MOUSE

/*void drawFilledCircle(SDL_Renderer* renderer, int cx, int cy, int radius) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            }
        }
    }
}*/

int main(int argc, char *argv[]) {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    // Inicializar SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Error inicializando SDL: %s\n", SDL_GetError());
        return 1;
    }

    // Crear ventana
    window = SDL_CreateWindow("Knights & Demons - Kabuto Factory",
                             SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED,
                             WINDOW_WIDTH,
                             WINDOW_HEIGHT,
                             SDL_WINDOW_SHOWN);

    if (!window) {
        printf("Error creando ventana: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Crear renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Error creando renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    srand(time(NULL));

    // Inicializar juego
    GameState *game = init_game(EASY);
    if (!game) {
        printf("Error inicializando juego\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Variables de control
    bool running = true;
    Uint32 last_time = SDL_GetTicks();
    bool using_pika = false;
    Difficulty menu_selection = EASY;

    // Bucle principal del juego
    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }

                switch (game->current_screen) {
                    case MENU_SCREEN:
                        if (event.key.keysym.sym == SDLK_UP) {
                            menu_selection = (menu_selection - 1 + 3) % 3;
                        }
                        else if (event.key.keysym.sym == SDLK_DOWN) {
                            menu_selection = (menu_selection + 1) % 3;
                        }
                        else if (event.key.keysym.sym == SDLK_RETURN) {
                            reset_game(game, menu_selection);
                            game->current_screen = GAME_SCREEN;
                        }
                        break;

                    case GAME_SCREEN:
                        if (event.key.keysym.sym == SDLK_p) {
                            using_pika = !using_pika;
                        }
                        else if (event.key.keysym.sym == SDLK_RETURN) {
                            // Ejecutar acción en celda seleccionada
                            if (using_pika) {
                                use_pika(game->current_level, game->current_level->selected_i, game->current_level->selected_j);
                            } else {
                                invert_adjacent_cells(game->current_level, game->current_level->selected_i, game->current_level->selected_j);
                            }

                            if (game->current_level->level_complete || game->current_level->remaining_time <= 0) {
                                process_level_completion(game);
                                if (game->game_finished) {
                                    game->current_screen = GAME_OVER_SCREEN;
                                }
                            }
                        }
                        // Movimiento con WASD
                        else if (event.key.keysym.sym == SDLK_w) {
                            move_selection(game->current_level, -1, 0);
                        }
                        else if (event.key.keysym.sym == SDLK_s) {
                            move_selection(game->current_level, 1, 0);
                        }
                        else if (event.key.keysym.sym == SDLK_a) {
                            move_selection(game->current_level, 0, -1);
                        }
                        else if (event.key.keysym.sym == SDLK_d) {
                            move_selection(game->current_level, 0, 1);
                        }
                        break;

                    case GAME_OVER_SCREEN:
                        if (event.key.keysym.sym == SDLK_RETURN) {
                            game->current_screen = MENU_SCREEN;
                        }
                        break;
                }
            }
        }

        // Actualizar tiempo en juego
        if (game->current_screen == GAME_SCREEN && game->current_level) {
            Uint32 current_time = SDL_GetTicks();
            if (current_time - last_time >= 1000) {
                game->current_level->remaining_time--;
                last_time = current_time;

                if (game->current_level->remaining_time <= 0) {
                    process_level_completion(game);
                    if (game->game_finished) {
                        game->current_screen = GAME_OVER_SCREEN;
                    }
                }
            }
        }

        // Renderizar según pantalla actual
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        switch (game->current_screen) {
            case MENU_SCREEN:
                draw_menu(renderer, menu_selection);
                break;

            case GAME_SCREEN:
                if (game->current_level) {
                    draw_real_time_meter(renderer, game->current_level, game);
                    draw_board(renderer, game->current_level);
                    draw_game_info(renderer, game);

                    // Indicador de modo actual
                    SDL_Rect mode_bg = {WINDOW_WIDTH/2 - 100, STATUS_BAR_HEIGHT - 25, 200, 20};
                    SDL_SetRenderDrawColor(renderer, 60, 60, 100, 255);
                    SDL_RenderFillRect(renderer, &mode_bg);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    SDL_RenderDrawRect(renderer, &mode_bg);

                    // Indicador visual del modo
                    SDL_Rect mode_indicator = {WINDOW_WIDTH/2 - 90, STATUS_BAR_HEIGHT - 20, 10, 10};
                    if (using_pika) {
                        SDL_SetRenderDrawColor(renderer, colors[3].r, colors[3].g, colors[3].b, 255);
                    } else {
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    }
                    SDL_RenderFillRect(renderer, &mode_indicator);
                }
                break;

            case GAME_OVER_SCREEN:
                draw_game_over(renderer, game->player_won);
                break;
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    // Limpiar recursos
    free_game(game);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}


/*
int main(int argc, char *argv[])
{
    unsigned char done;
    int k;
    //Mucha de esta parametrizacion puede venir por linea de comando!!
    int delay               = 100;
    SDL_Window* window      = NULL;
    SDL_Renderer* renderer  = NULL;
    SDL_Event e;
    SDL_Rect fillRect;

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("SDL No se ha podido inicializar! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("SDL No se ha podido inicializar! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }if (SDL_Init(SDL_INIT_TIMER) != 0)
    {
        printf("SDL No se ha podido inicializar! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    //Create window
    window = SDL_CreateWindow("Juego de la vida",
                                                SDL_WINDOWPOS_UNDEFINED,
                                                SDL_WINDOWPOS_UNDEFINED,
                                                640,
                                                480,
                                                SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Log("Error en la creacion de la ventana: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // Creamos el lienzo
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("No se ha podido crear el lienzo! SDL Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    while (!done){ //Se puede parar tambien cuando no se observen cambios!
        while (SDL_PollEvent(&e) != 0) {
            // Salida del usuario
            if (e.type == SDL_QUIT) {
                done = 1;
            }
        }
        // Se limpia la pantalla
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(renderer);

        fillRect.x = 0; //Pos X
        fillRect.y = 0; //Pos Y
        fillRect.h = 0; //Alto
        fillRect.w = 0; //Ancho

        //Plantilla para pintar cuadrados
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderFillRect(renderer, &fillRect);

        //Plantilla para pintar circulos si gusta mas
        //drawFilledCircle(renderer, X+e_size_w/2, Y+e_size_h/2, (e_size_w>e_size_h?e_size_h/2:e_size_w/2));

        // Actualizacion del "lienzo"
        SDL_RenderPresent(renderer);
        k++;
        //SDL_UpdateWindowSurface(window);

        //Titulo/caption de la ventana
        SDL_SetWindowTitle(window, "Porque no usar esto para poner alguna info del proceso?");
        SDL_Delay(delay);

        //Procesamiento de matriz?
    }

    //destruyo todos los elementos creados
    //Observar ni mas ni menos que destructores, en la asignatura no inventamos nada!
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
*/

