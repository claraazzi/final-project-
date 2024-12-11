/**
* Author: [Clara EL Azzi]
* Assignment: Rise of the AI
* Date due: 2024-11-9, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "Menu.h"
#include "Utility.h"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.h"
#include "Scene.h"



Scene* g_current_scene = nullptr; // Pointer to the current scene
Menu g_menu;
bool g_in_menu = true; // Tracks if we are in the menu screen

#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include <cmath>
#include <vector>

// ————— VARIABLES ————— //
SDL_Window* g_display_window;
ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;
float g_previous_ticks = 0.0f, g_accumulator = 0.0f;
GLuint g_font_texture_id;

enum AppStatus { RUNNING, TERMINATED };
AppStatus g_app_status = RUNNING;

// ————— CONSTANTS ————— //
constexpr int WINDOW_WIDTH = 1280, WINDOW_HEIGHT = 960;
constexpr char GAME_WINDOW_NAME[] = "Rise of the AI!";
constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl";
constexpr char F_SHADER_PATH[] = "shaders/fragment_textured.glsl";
constexpr char FONT_FILEPATH[] = "assets/fonts/font1.png";
int g_lives = 3; // Global variable to track lives


LevelA levelA; // Instance of LevelA
LevelB levelB; // Instance of LevelB
LevelC levelC; // Instance of LevelC

void initialise();
void process_input();
void update();
void render();
void shutdown();

void initialise();
void process_input();
void update();
void render();
void shutdown();






void initialise() {
    

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow(GAME_WINDOW_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    // Initialize view and projection matrices
    g_view_matrix = glm::mat4(1.0f);

    // Adjust projection matrix to move the maze up
    float zoom_factor = 1.2f; 

    float left = -7.0f / zoom_factor;
    float right = 7.0f / zoom_factor;
    float bottom = -8.0f / zoom_factor; // Move the bottom up
    float top = 0.7f / zoom_factor;  // Adjust top accordingly
    g_projection_matrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    glUseProgram(g_shader_program.get_program_id());
    glClearColor(0.1922f, 0.549f, 0.9059f, 1.0f); // Light blue background

    // Initialize audio
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    // Initialize menu and font texture
    g_menu.initialise();
    g_font_texture_id = Utility::load_texture(FONT_FILEPATH);

    if (!g_font_texture_id) {
        std::cerr << "Failed to load font texture from " << FONT_FILEPATH << std::endl;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set the initial scene to LevelA
    g_current_scene = &levelA;
    g_current_scene->initialise();
    Mix_Chunk* kill_sfx = Mix_LoadWAV("assets/audio/kill.wav");
    
}


void process_input() {
    if (g_in_menu) {
        // Pass the menu's process_input function the g_in_menu flag
        g_menu.process_input(g_in_menu);
        return; // Skip further input processing while in the menu
    }

    // Reset movement
    g_current_scene->get_game_state().player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        g_app_status = TERMINATED;
                        break;
                    case SDLK_a: { // Attack key
                        // Loop through all enemies and deactivate if within range
                        for (int i = 0; i < g_current_scene->get_game_state().enemy_count; ++i) {
                            Entity& enemy = g_current_scene->get_game_state().enemies[i];
                            glm::vec3 player_position = g_current_scene->get_game_state().player->get_position();
                            glm::vec3 enemy_position = enemy.get_position();
                            if (enemy.is_active() && glm::distance(player_position, enemy_position) < 1.0f) {
                                enemy.deactivate(); // Deactivate enemy
                                Mix_PlayChannel(-1, g_current_scene->get_game_state().kill_sfx, 0);
                            }
                        }
                        break;
                    }
                }
                break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    // Handle movement in all four directions
    if (key_state[SDL_SCANCODE_LEFT]) {
        g_current_scene->get_game_state().player->move_left(); // Move left
    }
    if (key_state[SDL_SCANCODE_RIGHT]) {
        g_current_scene->get_game_state().player->move_right(); // Move right
    }
    if (key_state[SDL_SCANCODE_UP]) {
        g_current_scene->get_game_state().player->move_up(); // Move up
    }
    if (key_state[SDL_SCANCODE_DOWN]) {
        g_current_scene->get_game_state().player->move_down(); // Move down
    }

    // Normalize movement vector for consistent speed in diagonal directions
    if (glm::length(g_current_scene->get_game_state().player->get_movement()) > 1.0f) {
        g_current_scene->get_game_state().player->normalise_movement();
    }
}


void update() {
    if (g_in_menu) return;

    // Calculate delta time
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    delta_time += g_accumulator;
    if (delta_time < FIXED_TIMESTEP) {
        g_accumulator = delta_time;
        return;
    }

    // Fixed time step update
    while (delta_time >= FIXED_TIMESTEP) {
        g_current_scene->update(FIXED_TIMESTEP);

        // Handle player lives
        if (!g_current_scene->get_game_state().player->is_active()) {
            g_lives--;
            if (g_lives > 0) {
                g_current_scene->initialise();
            } else {
                Utility::draw_text(&g_shader_program, g_font_texture_id, "YOU LOSE", 1.0f, 0.1f,
                                   glm::vec3(-3.0f, 0.0f, 0.0f));
                SDL_GL_SwapWindow(g_display_window);
                SDL_Delay(3000);
                shutdown();
                exit(0);
            }
        }

        // Handle scene transitions
        if (g_current_scene->get_game_state().next_scene_id == 2) {
            g_current_scene = &levelB;
            g_current_scene->initialise();
        } else if (g_current_scene->get_game_state().next_scene_id == 3) {
            g_current_scene = &levelC;
            g_current_scene->initialise();
        }

        delta_time -= FIXED_TIMESTEP;
    }
    g_accumulator = delta_time;

    // Smooth camera movement
    glm::vec3 player_pos = g_current_scene->get_game_state().player->get_position();
    Map* current_map = g_current_scene->get_game_state().map;

    float map_left = current_map->get_left_bound();
    float map_right = current_map->get_right_bound();
    float visible_width = 10.0f; // Width of the visible area

    float camera_x = -player_pos.x; // Center the player horizontally
    float camera_y = 0.0f; // Fixed vertically for horizontal scrolling

    // Ensure the camera doesn't show outside the map
    camera_x = glm::clamp(camera_x, -(map_right - visible_width / 2), map_left + visible_width / 2);

    g_view_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(camera_x, camera_y, 0.0f));
    g_shader_program.set_view_matrix(g_view_matrix);
    
    if (g_current_scene->get_game_state().player != nullptr) {
           glm::vec3 player_position = g_current_scene->get_game_state().player->get_position();
           glm::vec4 view_position = g_view_matrix * glm::vec4(player_position, 1.0f);
           g_shader_program.set_light_position(glm::vec2(view_position.x, view_position.y));
       }

    
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (g_in_menu) {
        // Reset the view and projection matrices for the menu
        g_view_matrix = glm::mat4(1.0f);
        g_shader_program.set_view_matrix(g_view_matrix);

        g_menu.render(&g_shader_program, g_font_texture_id);
        SDL_GL_SwapWindow(g_display_window);
        return;
    }

    // Render the game scene
    g_shader_program.set_view_matrix(g_view_matrix);
    g_current_scene->render(&g_shader_program);

    // Render lives count
   // Utility::draw_text(&g_shader_program, g_font_texture_id,
                      // "Lives: " + std::to_string(g_lives), 0.5f, 0.1f,
                      // glm::vec3(-4.5f, 3.0f, 0.0f)); // Position text as needed

    SDL_GL_SwapWindow(g_display_window);
}



void shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    initialise();

    while (g_app_status == RUNNING) {
        process_input();
        update();
        render();
    }
    shutdown();
    return 0;
}






