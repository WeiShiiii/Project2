/**
 * Author: Wei Shi
 * Assignment: Pong Clone
 * Date due: 2025-3-01, 11:59pm
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
 **/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define GL_GLEXT_PROTOTYPES 1
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include <iostream>

enum AppStatus { RUNNING, TERMINATED };

constexpr int WINDOW_WIDTH = 640,
    WINDOW_HEIGHT = 480;
constexpr float BG_RED = 0.9765625f,
                BG_GREEN = 0.97265625f,
                BG_BLUE = 0.9609375f,
                BG_OPACITY = 1.0f;
constexpr float MILLISECONDS_IN_SECOND = 1000.0f;
constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl";
constexpr char F_SHADER_PATH[] = "shaders/fragment_textured.glsl";
constexpr char SHIELD_SPRITE_FILEPATH[] = "shield.png";
constexpr char SWORD_SPRITE_FILEPATH[] = "sword.png";

SDL_Window* g_display_window = nullptr;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program;

glm::mat4 g_view_matrix, g_projection_matrix;
float g_previous_ticks = 0.0f;

GLuint paddle_texture_id, ball_texture_id;

bool single_player = false;

glm::vec3 paddle_left_position = glm::vec3(-4.5f, 0, 0);
glm::vec3 paddle_left_movement = glm::vec3(0);
glm::vec3 paddle_left_scale = glm::vec3(0.2f, 1.0f, 1.0f);

glm::vec3 paddle_right_position = glm::vec3(4.5f, 0, 0);
glm::vec3 paddle_right_movement = glm::vec3(0);
glm::vec3 paddle_right_scale = glm::vec3(0.2f, 1.0f, 1.0f);

glm::vec3 ball_position = glm::vec3(0, 0, 0);
glm::vec3 ball_movement = glm::vec3(0.5f, 1.0f, 0);
glm::vec3 ball_scale = glm::vec3(0.3f, 0.3f, 1.0f);


GLuint load_texture(const char* filepath) {
    int width, height, num_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &num_components, STBI_rgb_alpha);
    if (image == NULL) {
        LOG("Unable to load image");
        exit(1);
    }
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    stbi_image_free(image);
    return texture;
}

void initialise() {
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Pong Clone", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_view_matrix(g_view_matrix);
    g_shader_program.set_projection_matrix(g_projection_matrix);

    glUseProgram(g_shader_program.get_program_id());
    glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    paddle_texture_id = load_texture(SWORD_SPRITE_FILEPATH);
    ball_texture_id = load_texture(SHIELD_SPRITE_FILEPATH);
}
void process_input() {
    SDL_Event event;
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    paddle_left_movement = glm::vec3(0);
    paddle_right_movement = glm::vec3(0);

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            g_app_status = TERMINATED;
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_t:
                    single_player = !single_player;
                    break;
                case SDLK_q:
                    g_app_status = TERMINATED;
                    break;
                default:
                    break;
            }
        }
    }

    if (keys[SDL_SCANCODE_W]) paddle_left_movement.y = 1.0f;
    if (keys[SDL_SCANCODE_S]) paddle_left_movement.y = -1.0f;

    if (!single_player) {
        if (keys[SDL_SCANCODE_UP]) paddle_right_movement.y = 1.0f;
        if (keys[SDL_SCANCODE_DOWN]) paddle_right_movement.y = -1.0f;
    }

    if (glm::length(paddle_left_movement) > 1.0f) {
        paddle_left_movement = glm::normalize(paddle_left_movement);
    }
    if (glm::length(paddle_right_movement) > 1.0f) {
        paddle_right_movement = glm::normalize(paddle_right_movement);
    }
}


void update() {
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    constexpr float PADDLE_SPEED = 3.0f;
    paddle_left_position += paddle_left_movement * delta_time * PADDLE_SPEED;
    paddle_right_position += paddle_right_movement * delta_time * PADDLE_SPEED;

    constexpr float PADDLE_LIMIT = 3.25f;
    paddle_left_position.y = glm::clamp(paddle_left_position.y, -PADDLE_LIMIT, PADDLE_LIMIT);
    paddle_right_position.y = glm::clamp(paddle_right_position.y, -PADDLE_LIMIT, PADDLE_LIMIT);

    constexpr float BALL_SPEED = 2.0f;
    ball_position += ball_movement * delta_time * BALL_SPEED;

    constexpr float CEILING_Y = 3.6f;
    constexpr float FLOOR_Y = -3.6f;

    if (ball_position.y + (ball_scale.y / 2) >= CEILING_Y) {
        ball_position.y = CEILING_Y - (ball_scale.y / 2);
        ball_movement.y = -fabs(ball_movement.y);
    }

    if (ball_position.y - (ball_scale.y / 2) <= FLOOR_Y) {
        ball_position.y = FLOOR_Y + (ball_scale.y / 2);
        ball_movement.y = fabs(ball_movement.y);
    }

    constexpr float PADDLE_WIDTH = 0.3f;
    constexpr float PADDLE_HEIGHT = 1.0f;

    if ((ball_position.x < paddle_left_position.x + PADDLE_WIDTH &&
         ball_position.x > paddle_left_position.x &&
         abs(ball_position.y - paddle_left_position.y) < PADDLE_HEIGHT / 2) ||
        (ball_position.x > paddle_right_position.x - PADDLE_WIDTH &&
         ball_position.x < paddle_right_position.x &&
         abs(ball_position.y - paddle_right_position.y) < PADDLE_HEIGHT / 2)) {
        
        ball_movement.x = -ball_movement.x;
        
        ball_movement.y += (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.2f;

        constexpr float MAX_SPEED = 3.0f;
        ball_movement.x = glm::clamp(ball_movement.x, -MAX_SPEED, MAX_SPEED);
        ball_movement.y = glm::clamp(ball_movement.y, -MAX_SPEED, MAX_SPEED);
    }

    if (single_player) {
        constexpr float AI_SPEED = 2.0f;
        if (ball_position.y > paddle_right_position.y + 0.2f && paddle_right_position.y < PADDLE_LIMIT) {
            paddle_right_position.y += AI_SPEED * delta_time;
        }
        if (ball_position.y < paddle_right_position.y - 0.2f && paddle_right_position.y > -PADDLE_LIMIT) {
            paddle_right_position.y -= AI_SPEED * delta_time;
        }
    }

    constexpr float GAME_OVER_X = 5.0f;
    if (ball_position.x < -GAME_OVER_X || ball_position.x > GAME_OVER_X) {
        
        g_app_status = TERMINATED;
    }
}



void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };

    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), paddle_left_position);
    model_matrix = glm::scale(model_matrix, paddle_left_scale);
    g_shader_program.set_model_matrix(model_matrix);
    glBindTexture(GL_TEXTURE_2D, paddle_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    model_matrix = glm::translate(glm::mat4(1.0f), paddle_right_position);
    model_matrix = glm::scale(model_matrix, paddle_right_scale);
    g_shader_program.set_model_matrix(model_matrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    model_matrix = glm::translate(glm::mat4(1.0f), ball_position);
    model_matrix = glm::scale(model_matrix, ball_scale);
    g_shader_program.set_model_matrix(model_matrix);
    glBindTexture(GL_TEXTURE_2D, ball_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }

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
