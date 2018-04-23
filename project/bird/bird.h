#pragma once
#include "icg_helper.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

static const unsigned int nbBirdVertices = 102;
static const glm::vec3 birdVertices[] =
{
    // Left wing
    glm::vec3(0.0, 0.0, 7.0),
    glm::vec3(2.0, 0.0, 5.0),
    glm::vec3(2.0, 0.0, 7.5),

    glm::vec3(2.0, 0.0, 5.0),
    glm::vec3(2.0, 0.0, 7.5),
    glm::vec3(4.0, 0.0, 4.5),

    glm::vec3(2.0, 0.0, 7.5),
    glm::vec3(4.0, 0.0, 4.5),
    glm::vec3(4.0, 0.0, 8.0),

    glm::vec3(4.0, 0.0, 4.5),
    glm::vec3(4.0, 0.0, 8.0),
    glm::vec3(6.0, 0.0, 4.0),

    glm::vec3(4.0, 0.0, 8.0),
    glm::vec3(6.0, 0.0, 4.0),
    glm::vec3(6.0, 0.0, 8.0),

    glm::vec3(6.0, 0.0, 4.0),
    glm::vec3(6.0, 0.0, 8.0),
    glm::vec3(8.0, 0.0, 4.0),

    glm::vec3(6.0, 0.0, 8.0),
    glm::vec3(8.0, 0.0, 4.0),
    glm::vec3(8.0, 0.0, 8.0),

    // Right wing
    glm::vec3(18.0, 0.0, 7.0),
    glm::vec3(16.0, 0.0, 5.0),
    glm::vec3(16.0, 0.0, 7.5),

    glm::vec3(16.0, 0.0, 5.0),
    glm::vec3(16.0, 0.0, 7.5),
    glm::vec3(14.0, 0.0, 4.5),

    glm::vec3(16.0, 0.0, 7.5),
    glm::vec3(14.0, 0.0, 4.5),
    glm::vec3(14.0, 0.0, 8.0),

    glm::vec3(14.0, 0.0, 4.5),
    glm::vec3(14.0, 0.0, 8.0),
    glm::vec3(12.0, 0.0, 4.0),

    glm::vec3(14.0, 0.0, 8.0),
    glm::vec3(12.0, 0.0, 4.0),
    glm::vec3(12.0, 0.0, 8.0),

    glm::vec3(12.0, 0.0, 4.0),
    glm::vec3(12.0, 0.0, 8.0),
    glm::vec3(10.0, 0.0, 4.0),

    glm::vec3(12.0, 0.0, 8.0),
    glm::vec3(10.0, 0.0, 4.0),
    glm::vec3(10.0, 0.0, 8.0),

    // Head
    glm::vec3(9.0, 0.0, 10.0),
    glm::vec3(8.0, 0.0, 8.0),
    glm::vec3(9.0, 1.0, 8.0),

    glm::vec3(9.0, 0.0, 10.0),
    glm::vec3(9.0, 1.0, 8.0),
    glm::vec3(10.0, 0.0, 8.0),

    glm::vec3(9.0, 0.0, 10.0),
    glm::vec3(10.0, 0.0, 8.0),
    glm::vec3(9.0, -1.0, 8.0),

    glm::vec3(9.0, 0.0, 10.0),
    glm::vec3(9.0, -1.0, 8.0),
    glm::vec3(8.0, 0.0, 8.0),

    // Body
    glm::vec3(8.0, 0.0, 4.0),
    glm::vec3(8.0, 0.0, 8.0),
    glm::vec3(9.0, 1.0, 4.0),

    glm::vec3(8.0, 0.0, 8.0),
    glm::vec3(9.0, 1.0, 4.0),
    glm::vec3(9.0, 1.0, 8.0),

    glm::vec3(9.0, 1.0, 4.0),
    glm::vec3(9.0, 1.0, 8.0),
    glm::vec3(10.0, 0.0, 4.0),

    glm::vec3(9.0, 1.0, 8.0),
    glm::vec3(10.0, 0.0, 4.0),
    glm::vec3(10.0, 0.0, 8.0),

    glm::vec3(10.0, 0.0, 4.0),
    glm::vec3(10.0, 0.0, 8.0),
    glm::vec3(9.0, -1.0, 4.0),

    glm::vec3(10.0, 0.0, 8.0),
    glm::vec3(9.0, -1.0, 4.0),
    glm::vec3(9.0, -1.0, 8.0),

    glm::vec3(9.0, -1.0, 4.0),
    glm::vec3(9.0, -1.0, 8.0),
    glm::vec3(8.0, 0.0, 4.0),

    glm::vec3(9.0, -1.0, 8.0),
    glm::vec3(8.0, 0.0, 4.0),
    glm::vec3(8.0, 0.0, 8.0),

    // Back
    glm::vec3(8.0, 0.0, 4.0),
    glm::vec3(8.0, 0.0, 3.0),
    glm::vec3(9.0, 1.0, 4.0),

    glm::vec3(8.0, 0.0, 3.0),
    glm::vec3(9.0, 1.0, 4.0),
    glm::vec3(10.0, 0.0, 3.0),

    glm::vec3(9.0, 1.0, 4.0),
    glm::vec3(10.0, 0.0, 3.0),
    glm::vec3(10.0, 0.0, 4.0),

    glm::vec3(10.0, 0.0, 4.0),
    glm::vec3(10.0, 0.0, 3.0),
    glm::vec3(9.0, -1.0, 4.0),

    glm::vec3(10.0, 0.0, 3.0),
    glm::vec3(9.0, -1.0, 4.0),
    glm::vec3(8.0, 0.0, 3.0),

    glm::vec3(9.0, -1.0, 4.0),
    glm::vec3(8.0, 0.0, 3.0),
    glm::vec3(8.0, 0.0, 4.0),

    // Queue
    glm::vec3(7.0, 0.0, 0.0),
    glm::vec3(8.0, 0.0, 3.0),
    glm::vec3(11.0, 0.0, 0.0),

    glm::vec3(8.0, 0.0, 3.0),
    glm::vec3(11.0, 0.0, 0.0),
    glm::vec3(10.0, 0.0, 3.0),
};


class Bird {

    private:
        GLuint vertex_array_id_;        // vertex array object
        GLuint program_id_;             // GLSL shader program ID
        GLuint vertex_buffer_object_;   // memory buffer
        glm::mat4 model_matrix_;        // model matrix

        GLuint sky_texture_id;

        float fog_density = FOG_DENSITY-0.2;
        float fog_gradient = FOG_GRADIENT;

        float offset[2] = {0.0, 0.0};
        glm::vec2 direction = glm::vec2(0.0, 0.0);
        float speed = 1.0;
        float radiusPath;
        float phaseWing;


    public:
        void Init(GLuint sky_texture_id, float radiusPath, float phaseWing) {
            this->sky_texture_id = sky_texture_id;
            this->radiusPath = radiusPath;
            this->phaseWing = phaseWing;

            // compile the shaders.
            program_id_ = icg_helper::LoadShaders("bird_vshader.glsl",
                                                  "bird_fshader.glsl");
            if(!program_id_) {
                exit(EXIT_FAILURE);
            }

            glUseProgram(program_id_);

            // vertex one vertex array
            glGenVertexArrays(1, &vertex_array_id_);
            glBindVertexArray(vertex_array_id_);

            // vertex coordinates
            {
                // buffer
                glGenBuffers(1, &vertex_buffer_object_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
                glBufferData(GL_ARRAY_BUFFER, nbBirdVertices * sizeof(glm::vec3),
                             &birdVertices[0], GL_STATIC_DRAW);

                // attribute
                GLuint vertex_point_id = glGetAttribLocation(program_id_, "vpoint");
                glEnableVertexAttribArray(vertex_point_id);
                glVertexAttribPointer(vertex_point_id, 3, GL_FLOAT, DONT_NORMALIZE,
                                      ZERO_STRIDE, ZERO_BUFFER_OFFSET);
            }
        }

        void Cleanup() {
            glBindVertexArray(0);
            glUseProgram(0);
            glDeleteBuffers(1, &vertex_buffer_object_);
            glDeleteProgram(program_id_);
            glDeleteVertexArrays(1, &vertex_array_id_);
        }

        void Draw(const glm::mat4 &model = IDENTITY_MATRIX,
                  const glm::mat4 &view = IDENTITY_MATRIX,
                  const glm::mat4 &projection = IDENTITY_MATRIX){
            glUseProgram(program_id_);
            glBindVertexArray(vertex_array_id_);

            // time
            glUniform1f(glGetUniformLocation(program_id_, "time"), glfwGetTime());

            // setup MVP
            glm::mat4 MVP = projection*view*model;
            GLuint MVP_id = glGetUniformLocation(program_id_, "MVP");
            glUniformMatrix4fv(MVP_id, 1, GL_FALSE, value_ptr(MVP));

            glUniform2fv(glGetUniformLocation(program_id_, "offset"), 1, offset);
            glUniform1f(glGetUniformLocation(program_id_, "fog_density"), fog_density);
            glUniform1f(glGetUniformLocation(program_id_, "fog_gradient"), fog_gradient);
            glUniform1f(glGetUniformLocation(program_id_, "radiusPath"), radiusPath);
            glUniform1f(glGetUniformLocation(program_id_, "phaseWing"), phaseWing);

            GLint model_id = glGetUniformLocation(program_id_, "model");
            glUniformMatrix4fv(model_id, ONE, DONT_TRANSPOSE, glm::value_ptr(model));
            GLint view_id = glGetUniformLocation(program_id_, "view");
            glUniformMatrix4fv(view_id, ONE, DONT_TRANSPOSE, glm::value_ptr(view));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, sky_texture_id);
            GLuint sky_tex_location = glGetUniformLocation(program_id_, "sky_tex");
            glUniform1i(sky_tex_location, 0);

            // draw
            glDrawArrays(GL_TRIANGLES,0, nbBirdVertices);

            glBindVertexArray(0);
            glUseProgram(0);
        }

        void setOffset(float dx, float dy) {
            offset[0] += dx;
            offset[1] += dy;
        }

        void setFogDensity(float value) {
            fog_density = value;
        }

        void setFogGradient(float value) {
            fog_gradient = value;
        }
};
