#include "LevelA.h"
#include "Utility.h"
#include "Map.h"
#include <string> // For std::to_string

extern int g_lives; // Declare the global lives variable

unsigned int LEVEL_1_DATA[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1,
    1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1,
    1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1,
    1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1,
    1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

constexpr char JUMP_SFX_FILEPATH[] = "assets/audio/jump.wav";

void LevelA::initialise() {
    m_game_state.next_scene_id = -1;

    // Load the map with the same tileset and data
    // Load the background texture
    m_game_state.background_texture_id = Utility::load_texture("assets/images/background.png");

    GLuint map_texture_id = Utility::load_texture("assets/images/blacktile.png");
    m_game_state.map = new Map(14, 9, LEVEL_1_DATA, map_texture_id, 1.0f, 2, 2);

    // Initialize player
    GLuint player_texture_id = Utility::load_texture("assets/images/player.png");
    int player_walking_animation[4][4] = {
        { 1, 5, 9, 13 }, // DOWN
        { 3, 7, 11, 15 }, // UP
        { 2, 6, 10, 14 }, // LEFT
        { 0, 4, 8, 12 }   // RIGHT
    };
    glm::vec3 player_acceleration = glm::vec3(0.0f, -4.905f, 0.0f);
    m_game_state.player = new Entity(
        player_texture_id,          // Texture ID
        4.0f,                       // Speed
        player_acceleration,        // Acceleration
        6.0f,                       // Jump Power
        player_walking_animation,   // Walking animation frames
        0.0f,                       // Animation Time
        4,                          // Animation Frames
        0,                          // Animation Index
        4,                          // Animation Cols
        4,                          // Animation Rows
        0.5f, 0.5f,                 // Width, Height
        PLAYER                      // Entity Type
    );
    m_game_state.player->set_position(glm::vec3(1.0f, -1.0f, 0.0f));
    m_game_state.player->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));

    // Initialize one vertical-moving enemy
    GLuint enemy_texture_id = Utility::load_texture("assets/images/alien.png");
    m_game_state.enemy_count = 2; // LevelA has 1 enemy
    m_game_state.enemies = new Entity[m_game_state.enemy_count];
    m_game_state.enemies[0] = Entity(
        enemy_texture_id,           // Texture ID
        1.0f,                       // Speed
        0.7f,                       // Width
        0.7f,                       // Height
        ENEMY,                      // Entity Type
        VERTICAL_MOVER,             // AI Type
        WALKING                     // AI State
    );
    m_game_state.enemies[0].set_jumping_power(0.0f);
    m_game_state.enemies[0].set_scale(glm::vec3(0.75f, 0.75f, 0.0f));
    m_game_state.enemies[0].set_position(glm::vec3(4.0f, -3.0f, 0.0f)); // Adjusted to fit in the maze
    
    m_game_state.enemies[1] = Entity(
        enemy_texture_id,           // Texture ID
        1.0f,                       // Speed
        0.7f,                       // Width
        0.7f,                       // Height
        ENEMY,                      // Entity Type
        VERTICAL_MOVER,             // AI Type
        WALKING// AI State
    );
    m_game_state.enemies[1].set_jumping_power(0.0f);
    m_game_state.enemies[1].set_scale(glm::vec3(0.75f, 0.75f, 0.0f));
    m_game_state.enemies[1].set_position(glm::vec3(9.0f, -3.0f, 0.0f)); // Adjusted to fit in the maze

    
    // Initialize treasure
    GLuint treasure_texture_id = Utility::load_texture("assets/images/collectible.png");
    m_game_state.collectibles = new Entity[1];
    m_game_state.collectibles[0] = Entity(
                                     treasure_texture_id,           // Texture ID
        2.0f,                       // Speed
        0.7f,                       // Width
        0.7f,                       // Height
                                     COLLECTIBLE,                      // Entity Type
        NONE,             // AI Type
                                     WALKING                     // AI State
    );
    
    
    m_game_state.collectibles[0].set_jumping_power(0.0f);
    m_game_state.collectibles[0].set_scale(glm::vec3(0.75f, 0.75f, 0.0f));
    m_game_state.collectibles[0].set_position(glm::vec3(4.0f, -3.0f, 0.0f)); // Adjusted to fit in the maze
  


    // Initialize background music and sound effects
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    m_game_state.bgm = Mix_LoadMUS("assets/audio/galaxyloop.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(50.0f);
    m_game_state.jump_sfx = Mix_LoadWAV(JUMP_SFX_FILEPATH);
    m_game_state.die_sfx = Mix_LoadWAV("assets/audio/die.wav");
    
    m_game_state.collect_sfx = Mix_LoadWAV("assets/audio/collect.wav");
    m_game_state.kill_sfx = Mix_LoadWAV("assets/audio/kill.wav");
    

}


void LevelA::update(float delta_time) {
    m_game_state.player->update(delta_time, m_game_state.player, nullptr, 0, m_game_state.map);

    // Update the single enemy
    if (m_game_state.enemies[0].is_active()) {
        m_game_state.enemies[0].update(delta_time, m_game_state.player, nullptr, 0, m_game_state.map);

        // Check collision with the enemy
        if (m_game_state.player->check_collision(&m_game_state.enemies[0])) {
                g_lives--; // Decrement lives
                Mix_PlayChannel(-1, m_game_state.die_sfx, 0);
                if (g_lives > 0) {
                    initialise(); // Restart the level if lives remain
                } else {
                    m_game_state.next_scene_id = -1; // Trigger "Game Over"
                }
//            }
        }
    }
    
    //second enemy
    if (m_game_state.enemies[1].is_active()) {
        m_game_state.enemies[1].update(delta_time, m_game_state.player, nullptr, 0, m_game_state.map);

        // Check collision with the enemy
        if (m_game_state.player->check_collision(&m_game_state.enemies[1])) {
                g_lives--; // Decrement lives
                Mix_PlayChannel(-1, m_game_state.die_sfx, 0);
                if (g_lives > 0) {
                    initialise(); // Restart the level if lives remain
                } else {
                    m_game_state.next_scene_id = -1; // Trigger "Game Over"
                }
//            }
        }
    }
    
    if(m_game_state.collectibles[0].is_active()){
        m_game_state.collectibles[0].update(delta_time, m_game_state.player, nullptr, 0, m_game_state.map);
    }
    
    // Check if the player collects the treasure
    if (m_game_state.collectibles[0].is_active() &&
        m_game_state.player->check_collision(&m_game_state.collectibles[0])) {
        m_game_state.collectibles[0].deactivate();
        Mix_PlayChannel(-1, m_game_state.collect_sfx, 0);
    }

    
}


void LevelA::render(ShaderProgram* program) {
    // Render the background first
    glm::mat4 model_matrix = glm::mat4(1.0f);
    program->set_model_matrix(model_matrix);

    float screen_width = 40.0f;  // Adjust based on your map's width
    float screen_height = 30.0f; // Adjust based on your map's height

    std::vector<float> vertices = {
        -screen_width / 2, -screen_height / 2,
        screen_width / 2, -screen_height / 2,
        screen_width / 2, screen_height / 2,

        -screen_width / 2, -screen_height / 2,
        screen_width / 2, screen_height / 2,
        -screen_width / 2, screen_height / 2,
    };

    std::vector<float> texture_coords = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,

        0.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
    };

    glUseProgram(program->get_program_id());

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coords.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, m_game_state.background_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
    
    

    // Render the rest of the scene
    m_game_state.map->render(program);
    m_game_state.player->render(program);
    m_game_state.enemies[0].render(program);
    m_game_state.enemies[1].render(program);

    // Render the treasure
    if (m_game_state.collectibles[0].is_active()) {
        m_game_state.collectibles[0].render(program);
    }

    // Display remaining lives
    Utility::draw_text(program, Utility::load_texture("assets/fonts/font1.png"),
                       "Health: " + std::to_string(g_lives), 0.5f, 0.1f, glm::vec3(1.5f, -1.0f, 0.0f));

    // Game Over condition
    if (g_lives <= 0) {
        Utility::draw_text(program, Utility::load_texture("assets/fonts/font1.png"), "ALIENS!!!!", 0.4f, 0.5f,
                           glm::vec3(1.5f, -2.5f, 0.0f));
        SDL_GL_SwapWindow(SDL_GL_GetCurrentWindow());
        SDL_Delay(3000);
        exit(0);
    }

    // Win condition: Transition to next level
    if ((!m_game_state.enemies[0].is_active() && !m_game_state.enemies[1].is_active() && !m_game_state.collectibles[0].is_active())) {
        Utility::draw_text(program, Utility::load_texture("assets/fonts/font1.png"),
                           "NEXT PLANET", 0.5f, 0.1f,
                           glm::vec3(2.5f, -2.5f, 0.0f));
        SDL_GL_SwapWindow(SDL_GL_GetCurrentWindow());
        SDL_Delay(3000);
        m_game_state.next_scene_id = 2;
    }
}

LevelA::~LevelA() {
    // Delete dynamically allocated player
    if (m_game_state.player) {
        delete m_game_state.player;
        m_game_state.player = nullptr;
    }

    // Delete dynamically allocated enemies
    if (m_game_state.enemies) {
        delete[] m_game_state.enemies;
        m_game_state.enemies = nullptr;
    }

    // Delete dynamically allocated collectibles
    if (m_game_state.collectibles) {
        delete[] m_game_state.collectibles;
        m_game_state.collectibles = nullptr;
    }

    // Delete dynamically allocated map
    if (m_game_state.map) {
        delete m_game_state.map;
        m_game_state.map = nullptr;
    }

    // Free audio resources
    if (m_game_state.bgm) {
        Mix_FreeMusic(m_game_state.bgm);
        m_game_state.bgm = nullptr;
    }
    if (m_game_state.jump_sfx) {
        Mix_FreeChunk(m_game_state.jump_sfx);
        m_game_state.jump_sfx = nullptr;
    }
    if (m_game_state.die_sfx) {
        Mix_FreeChunk(m_game_state.die_sfx);
        m_game_state.die_sfx = nullptr;
    }
    if (m_game_state.collect_sfx) {
        Mix_FreeChunk(m_game_state.collect_sfx);
        m_game_state.collect_sfx = nullptr;
    }
    if (m_game_state.kill_sfx) {
        Mix_FreeChunk(m_game_state.kill_sfx);
        m_game_state.kill_sfx = nullptr;
    }

    // Close SDL audio
    Mix_CloseAudio();
}


