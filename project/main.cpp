// glew must be before glfw
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// contains helper functions such as shader compiler
#include "icg_helper.h"

#include <glm/gtc/matrix_transform.hpp>

#include "grid/grid.h"
#include "water/water.h"
#include "screenquad/screenquad.h"
#include "framebuffer.h"
#include "water_FB.h"
#include "sky_FB.h"
#include "constants.h"
#include "guiTextures/guiTextures.h"
#include "speedTest.h"
#include "cube/cube.h"
#include "bird/bird.h"

#include "trackball.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

Grid grid;
Water water;
ScreenQuad screenquad;
FrameBuffer framebuffer;
WaterFramebuffer waterFramebuffer;
SkyFramebuffer skyFramebuffer;
Cube sky;
Bird bird1;  // line 1 (the front bird)
Bird bird2a; // line 2 in swarm   a: left side
Bird bird2b; // line 2 in swarm   b: right side
Bird bird3a;
Bird bird3b;
GuiTextures guiTextures;
Trackball trackball;

int gridSizeExp = 9; // used in grid and screenquad
bool explore_mode = true;
bool bezier_mode = false;
bool fps_mode = false;
double animationClock = -1.0;
float totalOffsetX = 0.0;
float totalOffsetY = 0.0;

float water_level = 0.452; // was 0.4
bool flatTerrainActivated = false;
bool customKeys = false;
bool enableGUITextures = false;
bool turnAroundCamUp = false;

int window_width = 1800;
int window_height = 1000;

int map_width = 1000;
int map_length = 1000;

using namespace glm;

mat4 projection_matrix;
mat4 view_matrix;
mat4 trackball_matrix;
mat4 old_trackball_matrix;
mat4 grid_model_matrix = mat4(1.0);

GLuint height_map_texture_id;
GLuint normal_texture_id;
GLuint reflection_texture_id;
GLuint refraction_texture_id;
GLuint sky_texture_id;
GLuint noise_texture_id;

void gui_IO();
void move();
void fps();

float offset_at_start_x = -9.556;
float offset_at_start_y = 5.806;
float heightAtStart = 0.55;
vec3 cam_pos(0.00001f, heightAtStart, 0.0f);
vec3 cam_look(-0.1305, 0.452f, 0.9915);
vec3 cam_up(0.0f, 1.0f, 0.0f);

int move_forward = 0;
int move_side = 0;
int move_height = 0;
int turn_up_down = 0;
int pivot = 0;

float speed_move;
float speed_rotate;
float speed_pitch;

float bezier_cam_speed = 1.0;
float bezier_time_last_speed_change = -1.0;     // need to keep track of time spent at a given speed
float bezier_progress_accumulator;              // when we change speed, we put the time spent * old speed / 5 in this accumulator
float bezier_anim_length = 15.0;                 // expected length of animation
bool animationRunning = false;

//vec3 camPosBeforeAnimation = vec3(0.0, 0.0, 0.0);
float heightBeforeAnimation = heightAtStart;
vec2 offsetBeforeAnimation = vec2(offset_at_start_x, offset_at_start_x);
vec3 camLookBeforeAnimation = vec3(0.0, 0.0, 0.0);
vec3 animationStartPosition = vec3(-0.004, 0.6, -0.066);
vec3 animationControlePoint1 = vec3(-0.098, 0.7, -0.502);
vec3 animationControlePoint2 =  vec3(0.58, 0.7, -0.833);
vec3 animationEndPosition = vec3(0.806, 0.6, -0.306);

vec3 animationStartLook = vec3(-0.13, 0.45, 0.991);
vec3 animationLookControlePoint1 = vec3(-0.8, 0.45, 0.482);
vec3 animationLookControlePoint2 =  vec3(0.886, 0.45, -0.294);
vec3 animationEndLook = vec3(-0.832, 0.45, -1.828);

float base_time = glfwGetTime();

// sun
vec3 light_pos(0.0, -2.0, 1.0);
vec3 light_color(1.0, 1.0, 1.0);
vec3 light_reflectivity(0.7, 0.7, 0.7);

// forward declarations
double bezier(double A, double C1, double C2, double D, double t);
vec3 bezier3D(vec3 A, vec3 C1, vec3 C2, vec3 D, float t);
vec3 bezier3Ddv(vec3 A, vec3 C1, vec3 C2, vec3 D, float t);
void bezier_update_progress_accumulator();
void animation();

void Init(GLFWwindow* window) {
    glClearColor(0.4, 0.9, 1.0 /*white*/, 1.0 /*solid*/);
    glEnable(GL_DEPTH_TEST);

    // setup view and projection matrices
    view_matrix = lookAt(cam_pos, cam_look, cam_up);
    float ratio = window_width / (float) window_height;
    projection_matrix = perspective(45.0f, ratio, 0.001f, 10.0f);

    trackball_matrix = IDENTITY_MATRIX;

    grid_model_matrix = IDENTITY_MATRIX;

    // on retina/hidpi displays, pixels != screen coordinates
    // this unsures that the framebuffer has the same size as the window
    // (see http://www.glfw.org/docs/latest/window.html#window_fbsize)
    glfwGetFramebufferSize(window, &window_width, &window_height);

    // init custom objects
    std::tie(height_map_texture_id, normal_texture_id) = framebuffer.Init(map_width, map_length, true);
    std::tie(reflection_texture_id, refraction_texture_id) = waterFramebuffer.Init(window_width, window_height, false);
    sky_texture_id = skyFramebuffer.Init(window_width, window_height, true);
    noise_texture_id = screenquad.Init(window_width, window_height, height_map_texture_id, normal_texture_id, gridSizeExp, water_level);
    grid.Init(height_map_texture_id, normal_texture_id, sky_texture_id, gridSizeExp, water_level, cam_pos, 1);
    water.Init(water_level, reflection_texture_id, refraction_texture_id, height_map_texture_id, sky_texture_id, light_pos, light_color, light_reflectivity);
    sky.Init();
    bird1.Init(sky_texture_id, 0.4, 0.0);
    bird2a.Init(sky_texture_id, 0.38, 0.5);
    bird2a.setOffset(0.05, -0.05);
    bird2b.Init(sky_texture_id, 0.38, 1.0);
    bird2b.setOffset(-0.05, 0.05);
    bird3a.Init(sky_texture_id, 0.36, 1.5);
    bird3a.setOffset(0.05, -0.1);
    bird3b.Init(sky_texture_id, 0.36, 2.0);
    bird3b.setOffset(-0.05, -0.1);
    //bird1.Init(0.0);
    //bird1.Init(sky_texture_id);
    //bird1.Init(sky_texture_id);

    screenquad.setOffset(offset_at_start_x, offset_at_start_y);
    water.setOffset(offset_at_start_x, offset_at_start_y);
    grid.setOffset(offset_at_start_x, offset_at_start_y);

    guiTextures.Init();
    guiTextures.add(50, 50, 100, 100, height_map_texture_id, 1, 1);
    guiTextures.add(50, 200, 100, 100, normal_texture_id, 3, 1);
    guiTextures.add(1150, 656, 200, 114, reflection_texture_id, 3);
    guiTextures.add(1150, 512, 200, 114, refraction_texture_id, 3);
    guiTextures.add(1150, 368, 200, 114, sky_texture_id, 3);
    guiTextures.add(1150, 224, 100, 100, noise_texture_id, 1);
}

// gets called for every frame.
void Display() {
    const float time = glfwGetTime();

    glEnable(GL_CLIP_DISTANCE0);

    // draw perlin noise to render buffer
    framebuffer.Bind(GL_COLOR_ATTACHMENT0);
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        screenquad.Draw(NOISE);
    }
    framebuffer.Unbind();

    framebuffer.Bind(GL_COLOR_ATTACHMENT1);
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        screenquad.Draw(NORMAL);
    }
    framebuffer.Unbind();

    // We draw the reflection texture in the waterFramebuffer
    // The reflection texture correspond to everything that is above the water
    waterFramebuffer.Bind(GL_COLOR_ATTACHMENT0);
    {
        mat4 view_matrix_normal = view_matrix;
        // We mirror the camera with respect to the floor
        float dist_cam_reflect = 2*(cam_pos.y - water_level);
        cam_pos.y -= dist_cam_reflect;
        // We place the look_at point above the water
        float dist_look_at_point = 2 * (water_level - cam_look.y);
        cam_look.y += dist_look_at_point;
        // We invert x and z coordinate for cam_up
        cam_up.x *= -1;
        cam_up.z *= -1;
        view_matrix = lookAt(cam_pos, cam_look, cam_up);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        float adaptationFactor = pow(2, gridSizeExp-9);
        grid.Draw(time, vec4(0.0, 1.0, 0.0, -water_level),trackball_matrix * grid_model_matrix, view_matrix, view_matrix_normal, projection_matrix, -1.0);
        sky.Draw(projection_matrix*view_matrix*trackball_matrix*grid_model_matrix);

        // Get back to default value
        cam_pos.y += dist_cam_reflect;
        cam_look.y -= dist_look_at_point;
        cam_up.x *= -1;
        cam_up.z *= -1;
        view_matrix = lookAt(cam_pos, cam_look, cam_up);
    }
    waterFramebuffer.Unbind();

    // We draw the refraction texture in the waterFramebuffer
    // The refraction texture correspond to everything that is under the water
    waterFramebuffer.Bind(GL_COLOR_ATTACHMENT1);
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Note the 0.01 term added to water_level, it is used to fix glitch that
        // appear when we use the dudv map for reflection
        float adaptationFactor = pow(2, gridSizeExp-9);
        grid.Draw(time, vec4(0.0, -1.0, 0.0, (water_level+0.01)), trackball_matrix * grid_model_matrix, view_matrix, view_matrix, projection_matrix);
        sky.Draw(projection_matrix*view_matrix*trackball_matrix);
    }
    waterFramebuffer.Unbind();

    skyFramebuffer.Bind(GL_COLOR_ATTACHMENT0);
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        sky.Draw(projection_matrix*view_matrix*trackball_matrix);
    }
    skyFramebuffer.Unbind();

    // We draw the terrain itself on the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, window_width, window_height);

    grid.Draw(time, vec4(0.0, -1.0, 0.0, 10000), trackball_matrix * grid_model_matrix, view_matrix, view_matrix, projection_matrix);
    // We draw the sky
    sky.Draw(projection_matrix*view_matrix*trackball_matrix);

    // We draw the water
    glViewport(0, 0, window_width, window_height);
    water.Draw(time, cam_pos, trackball_matrix * grid_model_matrix, view_matrix, projection_matrix);

    // We draw the birds
    bird1.Draw(trackball_matrix * grid_model_matrix, view_matrix, projection_matrix);
    bird2a.Draw(trackball_matrix * grid_model_matrix, view_matrix, projection_matrix);
    bird2b.Draw(trackball_matrix * grid_model_matrix, view_matrix, projection_matrix);
    bird3a.Draw(trackball_matrix * grid_model_matrix, view_matrix, projection_matrix);
    bird3b.Draw(trackball_matrix * grid_model_matrix, view_matrix, projection_matrix);
}

// transforms glfw screen coordinates into normalized OpenGL coordinates.
vec2 TransformScreenCoords(GLFWwindow* window, int x, int y) {
    // the framebuffer and the window doesn't necessarily have the same size
    // i.e. hidpi screens. so we need to get the correct one
    int width;
    int height;
    glfwGetWindowSize(window, &width, &height);
    return vec2(2.0f * (float)x / width - 1.0f,
                1.0f - 2.0f * (float)y / height);
}

void MouseButton(GLFWwindow* window, int button, int action, int mod) {
    ImGui_ImplGlfwGL3_MouseButtonCallback(window, button, action, mod);

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS
            && ImGui::IsMouseHoveringWindow() == true && !explore_mode) {
        double x_i, y_i;
        glfwGetCursorPos(window, &x_i, &y_i);
        vec2 p = TransformScreenCoords(window, x_i, y_i);
        trackball.BeingDrag(p.x, p.y);
        old_trackball_matrix = trackball_matrix;
        // Store the current state of the model matrix.
    }
}

void MousePos(GLFWwindow* window, double x, double y) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS
            && ImGui::IsMouseHoveringWindow() == true && !explore_mode) {
        vec2 p = TransformScreenCoords(window, x, y);
        mat4 drag_matrix = trackball.Drag(p.x, p.y);
        trackball_matrix = drag_matrix * old_trackball_matrix;
    }

    // zoom
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS
            && ImGui::IsMouseHoveringWindow() == true && !explore_mode) {
        mat4 translation = IDENTITY_MATRIX;
        static float pos_y = y;

        if (y < pos_y) {
            pos_y = y;
            translation[3][2] = 0.05;
            view_matrix = translation * view_matrix;
        }
        else if (y > pos_y) {
            pos_y = y;
            translation[3][2] = -0.05;
            view_matrix = translation * view_matrix;
        }
    }
}


void ErrorCallback(int error, const char* description) {
    fputs(description, stderr);
}


void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mods);
    const float frame_duration = glfwGetTime() - base_time;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    else if (bezier_mode){
        switch(key){
        case GLFW_KEY_W :
            if(action == GLFW_RELEASE){
                bezier_update_progress_accumulator();
                bezier_cam_speed += 0.25;
                bezier_time_last_speed_change = glfwGetTime();
            }
            break;
        case GLFW_KEY_S :
            if(action == GLFW_RELEASE && bezier_cam_speed>=0.2){
                bezier_update_progress_accumulator();
                bezier_cam_speed -= 0.25;
                bezier_time_last_speed_change = glfwGetTime();
            }
            break;
        }
    }
    else if (explore_mode) {
        if (key == GLFW_KEY_UP){
            if (customKeys == true) {
                if (action == GLFW_PRESS) {
                    turn_up_down = -1;
                }
                else if (action == GLFW_RELEASE) {
                    turn_up_down = 0;
                }
            }
        }
        else if (key == GLFW_KEY_DOWN){
            if (customKeys == true) {
                if (action == GLFW_PRESS) {
                    turn_up_down = 1;
                }
                else if (action == GLFW_RELEASE) {
                    turn_up_down = 0;
                }
            }
        }
        if (key == GLFW_KEY_A){
            if (customKeys == true) {
                if (action == GLFW_PRESS) {
                    move_side = -1;
                }
                else if (action == GLFW_RELEASE) {
                    move_side = 0;
                }
            }
            else {
                if (action == GLFW_PRESS) {
                    pivot = -1;
                }
                else if (action == GLFW_RELEASE) {
                    pivot = 0;
                }
            }
        }
        else if (key == GLFW_KEY_D){
            if (customKeys == true) {
                if (action == GLFW_PRESS) {
                    move_side = 1;
                }
                else if (action == GLFW_RELEASE) {
                    move_side = 0;
                }
            }
            else {
                if (action == GLFW_PRESS) {
                    pivot = 1;
                }
                else if (action == GLFW_RELEASE) {
                    pivot = 0;
                }
            }
        }
        if (key == GLFW_KEY_W){
            if (action == GLFW_PRESS) {
                move_forward = 1;
            }
            else if (action == GLFW_RELEASE) {
                move_forward = 0;
            }
        }
        else if (key == GLFW_KEY_S){
            if (action == GLFW_PRESS) {
                move_forward = -1;
            }
            else if (action == GLFW_RELEASE) {
                move_forward = 0;
            }
        }
        if (key == GLFW_KEY_Q){
            if (customKeys == true) {
                if (action == GLFW_PRESS) {
                    pivot = -1;
                }
                else if (action == GLFW_RELEASE) {
                    pivot = 0;
                }
            }
            else {
                if (action == GLFW_PRESS) {
                    turn_up_down = -1;
                }
                else if (action == GLFW_RELEASE) {
                    turn_up_down = 0;
                }
            }
        }
        else if (key == GLFW_KEY_E){
            if (customKeys == true) {
                if (action == GLFW_PRESS) {
                    pivot = 1;
                }
                else if (action == GLFW_RELEASE) {
                    pivot = 0;
                }
            }
            else {
                if (action == GLFW_PRESS) {
                    turn_up_down = 1;
                }
                else if (action == GLFW_RELEASE) {
                    turn_up_down = 0;
                }
            }
        }
        if (key == GLFW_KEY_2){
            if (customKeys == true) {
                if (action == GLFW_PRESS) {
                    move_height = -1;
                }
                else if (action == GLFW_RELEASE) {
                    move_height = 0;
                }
            }
        }
        else if (key == GLFW_KEY_X){
            if (customKeys == true) {
                if (action == GLFW_PRESS) {
                    move_height = 1;
                }
                else if (action == GLFW_RELEASE) {
                    move_height = 0;
                }
            }
        }
    }
}

// gets called when the windows/framebuffer is resized.
void ResizeCallback(GLFWwindow* window, int width, int height) {
    window_width = width;
    window_height = height;

    float ratio = window_width / (float) window_height;
    projection_matrix = perspective(45.0f, ratio, 0.1f, 10.0f);

    glViewport(0, 0, window_width, window_height);
}


int main(int argc, char *argv[]) {
    // GLFW Initialization
    if(!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }

    glfwSetErrorCallback(ErrorCallback);

    // hint GLFW that we would like an OpenGL 3 context (at least)
    // http://www.glfw.org/faq.html#how-do-i-create-an-opengl-30-context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // attempt to open the window: fails if required version unavailable
    // note some Intel GPUs do not support OpenGL 3.2
    // note update the driver of your graphic card
    GLFWwindow* window = glfwCreateWindow(window_width, window_height,
                                          "Terrain", NULL, NULL);
    if(!window) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // makes the OpenGL context of window current on the calling thread
    glfwMakeContextCurrent(window);

    // set the callback for escape key
    glfwSetKeyCallback(window, KeyCallback);

    // set the framebuffer resize callback
    glfwSetFramebufferSizeCallback(window, ResizeCallback);

    // set the mouse press and position callback
    glfwSetMouseButtonCallback(window, MouseButton);
    glfwSetCursorPosCallback(window, MousePos);

    // GLEW Initialization (must have a context)
    // https://www.opengl.org/wiki/OpenGL_Loading_Library
    glewExperimental = GL_TRUE; // fixes glew error (see above link)
    if(glewInit() != GLEW_NO_ERROR) {
        fprintf( stderr, "Failed to initialize GLEW\n");
        return EXIT_FAILURE;
    }

    // Setup ImGui binding
    ImGui_ImplGlfwGL3_Init(window, false);

    cout << "OpenGL" << glGetString(GL_VERSION) << endl;

    // initialize our OpenGL program
    Init(window);

    // update the window size with the framebuffer size (on hidpi screens the
    // framebuffer is bigger)
    glfwGetFramebufferSize(window, &window_width, &window_height);
    //SetupProjection(window, window_width, window_height);

    SpeedTest speedTest;
    // render loop
    while(!glfwWindowShouldClose(window)){
        speedTest.begin();
        glfwPollEvents();
        move();
        fps();
        animation();
        base_time = glfwGetTime();
        //speedTest.step("Poll events");
        Display();
        //speedTest.step("Display terrain");

        if (enableGUITextures == true) {
            guiTextures.display();
        }

        //speedTest.step("Display gui textures");
        ImGui_ImplGlfwGL3_NewFrame();
        gui_IO();
        ImGui::Render();

        //speedTest.step("Display gui");

        glfwSwapBuffers(window);

        //speedTest.step("Draw on screen");
        speedTest.finish();
        //speedTest.display();
    }

    grid.Cleanup();
    guiTextures.Cleanup();
    framebuffer.Cleanup();
    waterFramebuffer.Cleanup();
    skyFramebuffer.Cleanup();

    // close OpenGL window and terminate GLFW
    glfwDestroyWindow(window);
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();
    return EXIT_SUCCESS;
}

void animation() {
    // if the animation has just begun
    if (bezier_time_last_speed_change < 0.0 && animationRunning == true) {
        offsetBeforeAnimation = grid.getOffset();
        camLookBeforeAnimation = cam_look;
        heightBeforeAnimation = cam_pos.y;

        float offset_x = -grid.getOffset().x + offset_at_start_x;
        float offset_y = -grid.getOffset().y + offset_at_start_y;

        screenquad.setOffset(offset_x, offset_y);
        water.setOffset(offset_x, offset_y);
        grid.setOffset(offset_x, offset_y);
        bird1.setOffset(offset_x, offset_y);
        bird2a.setOffset(offset_x, offset_y);
        bird2b.setOffset(offset_x, offset_y);
        bird3a.setOffset(offset_x, offset_y);
        bird3b.setOffset(offset_x, offset_y);



        animationClock = glfwGetTime();
        bezier_cam_speed = 1.0;
        bezier_progress_accumulator = 0.0;
        bezier_time_last_speed_change = glfwGetTime();
        explore_mode=false;
        fps_mode=false;
        bezier_mode=true;
    }
    else if (animationRunning == true){
        float progress = bezier_progress_accumulator + ((glfwGetTime() - bezier_time_last_speed_change) * bezier_cam_speed)/bezier_anim_length;
        // if the animation is going on
        if ( progress <= 1.0 ){
            vec3 A = animationStartPosition;
            vec3 C1 = animationControlePoint1;
            vec3 C2 = animationControlePoint2;
            vec3 B = animationEndPosition;
            vec3 bezierOffset = bezier3D(A, C1, C2, B, progress);
            float offset_x = bezierOffset.x - totalOffsetX;
            cam_pos.y = bezierOffset.y;                     // cam_pos.y is the 'height' axis right ?
            float offset_y = bezierOffset.z - totalOffsetY;

            screenquad.setOffset(offset_x, offset_y);
            water.setOffset(offset_x, offset_y);
            grid.setOffset(offset_x, offset_y);
            bird1.setOffset(offset_x, offset_y);
            bird2a.setOffset(offset_x, offset_y);
            bird2b.setOffset(offset_x, offset_y);
            bird3a.setOffset(offset_x, offset_y);
            bird3b.setOffset(offset_x, offset_y);

            totalOffsetX += offset_x;
            totalOffsetY += offset_y;

            // invert the z and y axis
            //vec3 bezierDv = bezier3Ddv(A, C1, C2, B, progress);
            //cam_look = vec3(-bezierDv.x, cam_pos.y - 0.1/*cam_pos.y+bezierDv.z*/, -bezierDv.y);

            cam_look = bezier3D(animationStartLook, animationLookControlePoint1,
                                animationLookControlePoint2, animationEndLook, progress);

            view_matrix = lookAt(cam_pos, cam_look, cam_up);
        }
        // if the animation is finished
        else {
            bezier_time_last_speed_change = -1.0;
            animationRunning = false;

            float offset_x = -grid.getOffset().x + offsetBeforeAnimation.x;
            float offset_y = -grid.getOffset().y + offsetBeforeAnimation.y;

            screenquad.setOffset(offset_x, offset_y);
            water.setOffset(offset_x, offset_y);
            grid.setOffset(offset_x, offset_y);
            bird1.setOffset(offset_x, offset_y);
            bird2a.setOffset(offset_x, offset_y);
            bird2b.setOffset(offset_x, offset_y);
            bird3a.setOffset(offset_x, offset_y);
            bird3b.setOffset(offset_x, offset_y);
            totalOffsetX = 0.0;
            totalOffsetY = 0.0;

            cam_look = camLookBeforeAnimation;
            cam_pos.y = heightBeforeAnimation;

            explore_mode=true;
            fps_mode=false;
            bezier_mode=false;
        }
    }
}

void fps() {
    if (fps_mode) {
        float adaptationFactor = pow(2, gridSizeExp-9);
        float height = ((framebuffer.getHeight() - 0.5) / adaptationFactor) + 0.5 +0.01;
        float diff = cam_pos.y - height;
        cam_pos.y = height;
        cam_look.y += diff;
    }
}

void pitch(int direction, float speed) {
    const float frame_duration = glfwGetTime() - base_time;
    vec3 rotation_vector = cross(cam_up, cam_look);
    cam_up = vec3(rotate(mat4(1.0), static_cast<float>(direction*speed*frame_duration*25*M_PI/64), rotation_vector) *
            vec4(cam_up.x, cam_up.y, cam_up.z, 0.0));
    cam_look = vec3(rotate(mat4(1.0), static_cast<float>(direction*speed*frame_duration*25*M_PI/64), rotation_vector) *
            vec4(cam_look.x, cam_look.y, cam_look.z, 0.0));
    view_matrix = lookAt(cam_pos, cam_look, cam_up);
}

void move() {
    const float frame_duration = glfwGetTime() - base_time;
    if (customKeys == true) {
        if (turn_up_down != 0) {
            vec3 cam_look_vector = cam_look - cam_pos;
            float look_at_offset_y = length(cam_look_vector) * frame_duration*20.0*sin(M_PI / 48.0);
            cam_look.y += turn_up_down*look_at_offset_y;
            view_matrix = lookAt(cam_pos, cam_look, cam_up);
        }
    }
    else {
        if (turn_up_down != 0 || speed_pitch != 0.0) {
            if (turn_up_down != 0) {
                if (abs(speed_pitch) < 0.2) {
                    speed_pitch += -0.75 * turn_up_down * frame_duration;
                }
                else {
                    speed_pitch += -1.5 * turn_up_down * frame_duration;
                }
                if (speed_pitch > 1.0) {
                    speed_pitch = 1.0;
                }
                else if (speed_pitch < -1.0) {
                    speed_pitch = -1.0;
                }
            }
            else if (speed_pitch > 0.0) {
                if (speed_pitch > 1.0 * frame_duration) {
                    if (speed_pitch < 0.2) {
                        speed_pitch -= 0.5 * frame_duration;
                    }
                    else {
                        speed_pitch -= 1.0 * frame_duration;
                    }
                }
                else {
                    speed_pitch = 0.0;
                }
            }
            else if (speed_pitch < 0.0) {
                if (abs(speed_pitch) > 1.0 * frame_duration) {
                    if (abs(speed_pitch) < 0.2) {
                        speed_pitch += 0.5 * frame_duration;
                    }
                    else {
                        speed_pitch += 1.0 * frame_duration;
                    }
                }
                else {
                    speed_pitch = 0.0;
                }
            }

            vec3 rotation_vector = cross(cam_up, cam_look);
            cam_up = vec3(rotate(mat4(1.0), static_cast<float>(speed_pitch*frame_duration*25*M_PI/64), rotation_vector) *
                    vec4(cam_up.x, cam_up.y, cam_up.z, 0.0));
            cam_look = vec3(rotate(mat4(1.0), static_cast<float>(speed_pitch*frame_duration*25*M_PI/64), rotation_vector) *
                    vec4(cam_look.x, cam_look.y, cam_look.z, 0.0));
            view_matrix = lookAt(cam_pos, cam_look, cam_up);
        }
    }
    if (customKeys == true) {
        if (move_forward != 0) {
            float offset_x = move_forward*frame_duration/6.0*cam_look.x;
            float offset_y = -move_forward*frame_duration/6.0*cam_look.z;
            screenquad.setOffset(offset_x, offset_y);
            water.setOffset(offset_x, offset_y);
            grid.setOffset(offset_x, offset_y);
            bird1.setOffset(offset_x, offset_y);
            bird2a.setOffset(offset_x, offset_y);
            bird2b.setOffset(offset_x, offset_y);
            bird3a.setOffset(offset_x, offset_y);
            bird3b.setOffset(offset_x, offset_y);
        }
    }
    else {
        if (move_forward != 0 || speed_move != 0.0) {
            if (move_forward != 0) {
                if (abs(speed_move) < 0.2) {
                    speed_move += -0.75 * move_forward * frame_duration;
                }
                else {
                    speed_move += -1.5 * move_forward * frame_duration;
                }
                if (speed_move > 1.0) {
                    speed_move = 1.0;
                }
                else if (speed_move < -1.0) {
                    speed_move = -1.0;
                }
            }
            else if (speed_move > 0.0) {
                if (speed_move > 1.0 * frame_duration) {
                    if (speed_move < 0.2) {
                        speed_move -= 0.5 * frame_duration;
                    }
                    else {
                        speed_move -= 1.0 * frame_duration;
                    }
                }
                else {
                    speed_move = 0.0;
                }
            }
            else if (speed_move < 0.0) {
                if (abs(speed_move) > 1.0 * frame_duration) {
                    if (abs(speed_move) < 0.2) {
                        speed_move += 0.5 * frame_duration;
                    }
                    else {
                        speed_move += 1.0 * frame_duration;
                    }
                }
                else {
                    speed_move = 0.0;
                }
            }

            float offset_x = -speed_move*frame_duration/6.0*cam_look.x;
            float offset_y = speed_move*frame_duration/6.0*cam_look.z;
            screenquad.setOffset(offset_x, offset_y);
            water.setOffset(offset_x, offset_y);
            grid.setOffset(offset_x, offset_y);
            bird1.setOffset(offset_x, offset_y);
            bird2a.setOffset(offset_x, offset_y);
            bird2b.setOffset(offset_x, offset_y);
            bird3a.setOffset(offset_x, offset_y);
            bird3b.setOffset(offset_x, offset_y);

            // For custom keys, the movement is always horizontal, for default keys
            // we have to move also up and down depending on the cam_look
            float horizontal_distance = sqrt(offset_x*offset_x + offset_y*offset_y);
            float height_cam_pos_to_cam_look = (cam_look.y - cam_pos.y);
            float height_offset = height_cam_pos_to_cam_look * horizontal_distance / sqrt(cam_look.x*cam_look.x + cam_look.z*cam_look.z);
            cam_pos.y -= height_offset * speed_move;
        }
    }
    if (move_side != 0) {
        if (customKeys == true) {
            vec3 direction = vec3(rotate(mat4(1.0), static_cast<float>(3*M_PI/2), cam_up) *
                                   vec4(cam_look.x, cam_look.y, cam_look.z, 0.0));
            float offset_x = move_side*frame_duration/4.0*direction.x;
            float offset_y = -move_side*frame_duration/4.0*direction.z;
            screenquad.setOffset(offset_x, offset_y);
            water.setOffset(offset_x, offset_y);
            grid.setOffset(offset_x, offset_y);
            bird1.setOffset(offset_x, offset_y);
            bird2a.setOffset(offset_x, offset_y);
            bird2b.setOffset(offset_x, offset_y);
            bird3a.setOffset(offset_x, offset_y);
            bird3b.setOffset(offset_x, offset_y);
        }
    }
    if (move_height != 0) {
        if (customKeys == true) {
            cam_pos.y -= frame_duration/6.0*move_height;
            cam_look.y = cam_look.y - frame_duration/6.0*move_height;//0.0f;
            view_matrix = lookAt(cam_pos, cam_look, cam_up);
        }
    }

    if (pivot != 0 || speed_rotate != 0.0) {
        if (pivot != 0) {
            if (abs(speed_rotate) < 0.2) {
                speed_rotate += -0.75 * pivot * frame_duration;
            }
            else {
                speed_rotate += -1.5 * pivot * frame_duration;
            }
            if (speed_rotate > 1.0) {
                speed_rotate = 1.0;
            }
            else if (speed_rotate < -1.0) {
                speed_rotate = -1.0;
            }
        }
        else if (speed_rotate > 0.0) {
            if (speed_rotate > 1.0 * frame_duration) {
                if (speed_rotate < 0.2) {
                    speed_rotate -= 0.5 * frame_duration;
                }
                else {
                    speed_rotate -= 1.0 * frame_duration;
                }
            }
            else {
                speed_rotate = 0.0;
            }
        }
        else if (speed_rotate < 0.0) {
            if (abs(speed_rotate) > 1.0 * frame_duration) {
                if (abs(speed_rotate) < 0.2) {
                    speed_rotate += 0.5 * frame_duration;
                }
                else {
                    speed_rotate += 1.0 * frame_duration;
                }
            }
            else {
                speed_rotate = 0.0;
            }
        }

        if (turnAroundCamUp == true) {
            cam_look = vec3(rotate(mat4(1.0), static_cast<float>(frame_duration*speed_rotate*25*M_PI/32), cam_up) *
                        vec4(cam_look.x, cam_look.y, cam_look.z, 0.0));
        }
        else {
            cam_look = vec3(rotate(mat4(1.0), static_cast<float>(frame_duration*speed_rotate*25*M_PI/32), vec3(0.0, 1.0, 0.0)) *
                        vec4(cam_look.x, cam_look.y, cam_look.z, 0.0));
            cam_up = vec3(rotate(mat4(1.0), static_cast<float>(frame_duration*speed_rotate*25*M_PI/32), vec3(0.0, 1.0, 0.0)) *
                          vec4(cam_up.x, cam_up.y, cam_up.z, 0.0));
        }
        view_matrix = lookAt(cam_pos, cam_look, cam_up);
    }
}

void gui_IO() {
    ImGui::Text("General");
    ImGui::Checkbox("Activate wireframe mode", grid.getWireFrameMode());
    if (ImGui::Checkbox("Activate explore mode", &explore_mode)){
        if (explore_mode) {
            cam_pos.x = 0.00001f;
            cam_pos.y = 0.5f;
            cam_pos.z = 0.0f;
            cam_look.x = 0.0f;
            cam_look.y = cam_pos.y - 0.5;//0.0f;
            cam_look.z = 1.0f;
            grid.setFog(FOG_DENSITY, FOG_GRADIENT);
            water.setFogDensity(*(grid.getFogDensity()));
        }
        else {
            cam_pos.x = 1.15f;
            cam_pos.y = 1.55f;
            cam_pos.z = 1.15f;
            cam_look.x = 0.0f;
            cam_look.y = -0.2f;
            cam_look.z = 0.0f;
            grid.setFog(0.0, 1.0);
            water.setFogDensity(*(grid.getFogDensity()));
        }
        view_matrix = lookAt(cam_pos, cam_look, cam_up);
    }
    if (ImGui::Checkbox("Activate custom keys", &customKeys)) {
        if (customKeys) {
            cam_up.x = 0.0;
            cam_up.y = 1.0;
            cam_up.z = 0.0;
        }
    }
    ImGui::Checkbox("Activate turn around cam_up", &turnAroundCamUp);
    ImGui::Checkbox("Activate FPS", &fps_mode);
    ImGui::Checkbox("Activate animation", &animationRunning);
    ImGui::Checkbox("Show debug textures", &enableGUITextures);

    ImGui::Text("Position X : %f", screenquad.getOffset()[0]);
    ImGui::Text("Position Y : %f", screenquad.getOffset()[1]);
    ImGui::Text("Look at X : %f", cam_look.x);
    ImGui::Text("Look at Y : %f", cam_look.z);

    ImGui::Text("Heightmap");
    ImGui::InputInt("Seed", screenquad.getSeed());
    ImGui::SliderInt("Octaves nb", screenquad.getOctaveNb(), 1, 12);
    if (ImGui::SliderInt("Grid size expo.", &gridSizeExp, 1, 15)) {
        grid.setGridSizeExp(gridSizeExp);
        screenquad.setGridSizeExp(gridSizeExp);
    }
    ImGui::SliderInt("Cell Size Expo.", screenquad.getCellSizeExp(), 1, 15);
    ImGui::SliderFloat("Lacunarity", screenquad.getLacunarity(), 1.0, 10.0);
    ImGui::SliderFloat("H.", screenquad.getH(), 0.0, 2.0);
    ImGui::SliderFloat("Ridged offset", screenquad.getRidgedFractalOffset(), 1.0, 15);

    ImGui::Text("Grid");
    if(ImGui::SliderFloat("Water level", water.getWaterLevel(), 0.001, 1.0)) {
        grid.setWaterLevel(*(water.getWaterLevel()));
        water_level = *water.getWaterLevel();
        screenquad.setWaterLevel(water_level);
    }
    if (ImGui::SliderFloat("Ground height offset", grid.getAmplitude(), 0.0, 5.0)) {
        water.setAmplitude(*(grid.getAmplitude()));
    }
    ImGui::SliderFloat("Steepness threshold mixed rock", grid.getSteepnessThresholdMixRock(), 0.0, 1.0);
    ImGui::SliderFloat("Steepness threshold pure rock", grid.getSteepnessThresholdPureRock(), 0.0, 1.0);
    if (ImGui::Checkbox("Activate flat terrain", &flatTerrainActivated)) {
        screenquad.setFlatTerrainActivated(flatTerrainActivated);
    }
    ImGui::SliderFloat("Noise size", screenquad.getNoiseSize(), 0.1, 30.0);
    ImGui::SliderFloat("Noise density", screenquad.getNoiseDensity(), 0.001, 10.0);
    ImGui::SliderFloat("Noise gradient", screenquad.getNoiseGradient(), 0.001, 10.0);

    ImGui::Text("Water");
    ImGui::SliderInt("DuDv map size", water.getDudvMapSize(), 1, 30);
    ImGui::SliderFloat("Wave strenght", water.getWaveStrenght(), 0.0, 0.2);
    ImGui::SliderFloat("Wave speed", water.getWaveSpeed(), 0.0, 0.1);
    ImGui::SliderFloat("Reflectiveness", water.getReflectiveness(), 0.0, 10.0);

    ImGui::Text("Fog");
    if(ImGui::SliderFloat("Density", grid.getFogDensity(), 0.0, 2.0)) {
        water.setFogDensity(*(grid.getFogDensity()));
        bird1.setFogDensity(std::max(*(grid.getFogDensity()) -0.2, 0.0));
        bird2a.setFogDensity(std::max(*(grid.getFogDensity()) -0.2, 0.0));
        bird2b.setFogDensity(std::max(*(grid.getFogDensity()) -0.2, 0.0));
        bird3a.setFogDensity(std::max(*(grid.getFogDensity()) -0.2, 0.0));
        bird3b.setFogDensity(std::max(*(grid.getFogDensity()) -0.2, 0.0));
    }
    if(ImGui::SliderFloat("Gradient", grid.getFogGradient(), 0.0, 15.0)) {
        water.setFogGradient(*(grid.getFogGradient()));
        bird1.setFogGradient(*(grid.getFogGradient()));
        bird2a.setFogGradient(*(grid.getFogGradient()));
        bird2b.setFogGradient(*(grid.getFogGradient()));
        bird3a.setFogGradient(*(grid.getFogGradient()));
        bird3b.setFogGradient(*(grid.getFogGradient()));
    }
}

/*  calculates a 1-D (i.e. will be called once for each dimension) bezier curve
    using the 'de Casteljau' algorithm
    A is the arrival, C1 and C2 the 'corners'/control points, D the departure, t in [0,1] the progress
*/
double bezier(double A, double C1, double C2, double D, double t)
{
    double s = 1 - t;
    double AC1 = A*s + C1*t;
    double C1C2 = C1*s + C2*t;
    double C2D = C2*s + D*t;
    double AC1C2 = AC1*s + C1C2*t;
    double C1C2D = C1C2*s + C2D*t;
    return AC1C2*s + C1C2D*t;
}

/*  calculates a 1-D (i.e. will be called once for each dimension) bezier curve
    using the 'de Casteljau' algorithm
    A is the arrival, C1 and C2 the 'corners'/control points, D the departure, t in [0,1] the progress
*/
vec3 bezier3D(vec3 A, vec3 C1, vec3 C2, vec3 D, float progress)
{
    float t = progress;
    float s = 1.0f - t;
    vec3 AC1 = s*A + t*C1;
    vec3 C1C2 = C1*s + C2*t;
    vec3 C2D = C2*s + D*t;
    vec3 AC1C2 = AC1*s + C1C2*t;
    vec3 C1C2D = C1C2*s + C2D*t;
    return AC1C2*s + C1C2D*t;
}

/*  Calculates the derivative of the bezier curve defined as in 'bezier()'
*/
vec3 bezier3Ddv(vec3 A, vec3 C1, vec3 C2, vec3 D, float progress){
    float t = progress;
    float s = 1.0f - t;
    vec3 AC1 = s*A + t*C1;
    vec3 C1C2 = C1*s + C2*t;
    vec3 C2D = C2*s + D*t;
    vec3 AC1C2 = AC1*s + C1C2*t;
    vec3 C1C2D = C1C2*s + C2D*t;
    return normalize(AC1C2-C1C2D);
}

/*  Update the progress acumulator, to be called before speed change
 */
void bezier_update_progress_accumulator(){
    bezier_progress_accumulator = bezier_progress_accumulator + ((glfwGetTime() - bezier_time_last_speed_change) * bezier_cam_speed)/bezier_anim_length;
}
