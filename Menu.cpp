

#include "Menu.h"
#include "Utility.h"
#include <SDL.h>
#include <cmath>

GLuint background_texture_id;
#define FIXED_TIMESTEP 0.0166666f // ~60 FPS

void Menu::initialise() {
    // Load the background texture
    background_texture_id = Utility::load_texture("assets/images/background3.png");
}

void Menu::process_input(bool& in_menu) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            exit(0); // Quit the application
        } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
            in_menu = false; // Exit the menu
        }
    }
}

void Menu::render(ShaderProgram* program, GLuint font_texture_id) {
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT);

    // Set a neutral spotlight position (center of the screen)
    program->set_light_position(glm::vec2(0.0f, 0.0f)); // Neutral light position for the menu

    // Render the background
    glm::mat4 model_matrix = glm::mat4(1.0f);
    program->set_model_matrix(model_matrix);

    float screen_width = 20.0f;  // Adjust based on your screen's width
    float screen_height = 15.0f; // Adjust based on your screen's height

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

    glBindTexture(GL_TEXTURE_2D, background_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());

    // Draw menu text
    Utility::draw_text(program, font_texture_id, "Welcome to COSMIC HEIST!", 0.75f, -0.3f, glm::vec3(-5.0f, -1.0f, 0.0f));

    // Animate "Press ENTER to Start" text
    static float animation_time = 0.0f; // Tracks the time for animation
    animation_time += FIXED_TIMESTEP;

    // Use a sine wave to make the text move slightly
    float y_offset = sin(animation_time * 3.0f) * 0.2f; // Adjust speed and amplitude as needed

    Utility::draw_text(program, font_texture_id, "Press ENTER to teleport to SPACE", 0.5f, -0.25f, glm::vec3(-4.0f, -2.0f + y_offset, 0.0f));
}
