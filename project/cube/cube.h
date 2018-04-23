#pragma once
#include "icg_helper.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

static const unsigned int NbCubeVertices = 36;
static const glm::vec3 CubeVertices[] =
{
    glm::vec3(-0.5, -0.5, -0.5),
    glm::vec3(-0.5, 0.502, -0.5),
    glm::vec3(0.5, -0.5, -0.5),
    glm::vec3(-0.5, 0.502, -0.5),
    glm::vec3(0.5, -0.5, -0.5),
    glm::vec3(0.5, 0.502, -0.5),

    glm::vec3(0.5, 0.5, 0.5),
    glm::vec3(0.5, -0.5, 0.5),
    glm::vec3(0.5, 0.5, -0.5),
    glm::vec3(0.5, -0.5, 0.5),
    glm::vec3(0.5, 0.5, -0.5),
    glm::vec3(0.5, -0.5, -0.5),

    glm::vec3(0.5, 0.5, 0.5),
    glm::vec3(-0.5, 0.5, 0.5),
    glm::vec3(0.5, -0.5, 0.5),
    glm::vec3(-0.5, 0.5, 0.5),
    glm::vec3(0.5, -0.5, 0.5),
    glm::vec3(-0.5, -0.5, 0.5),

    glm::vec3(-0.5, -0.5, 0.5),
    glm::vec3(-0.5, -0.5, -0.5),
    glm::vec3(0.5, -0.5, 0.5),
    glm::vec3(-0.5, -0.5, -0.5),
    glm::vec3(0.5, -0.5, 0.5),
    glm::vec3(0.5, -0.5, -0.5),

    glm::vec3(-0.5, 0.5, -0.5),
    glm::vec3(-0.5, -0.5, -0.5),
    glm::vec3(-0.5, 0.5, 0.5),
    glm::vec3(-0.5, -0.5, -0.5),
    glm::vec3(-0.5, 0.5, 0.5),
    glm::vec3(-0.5, -0.5, 0.5),

    glm::vec3(0.502, 0.5, -0.502),
    glm::vec3(-0.502, 0.5, -0.502),
    glm::vec3(0.502, 0.5, 0.502),
    glm::vec3(-0.502, 0.5, -0.502),
    glm::vec3(0.502, 0.5, 0.502),
    glm::vec3(-0.502, 0.5, 0.502)
};

static const unsigned int NbCubeColors = 36;
static const unsigned int NbCubeUVs = 36;

static const glm::vec2 CubeUVs[] =
{
    // left
    glm::vec2(0.0, 0.334),
    glm::vec2(0.0, 0.666),
    glm::vec2(0.251, 0.334),
    glm::vec2(0.0, 0.666),
    glm::vec2(0.251, 0.334),
    glm::vec2(0.251, 0.666),

    // middle
    glm::vec2(0.499, 0.666),
    glm::vec2(0.499, 0.334),
    glm::vec2(0.251, 0.666),
    glm::vec2(0.499, 0.334),
    glm::vec2(0.251, 0.666),
    glm::vec2(0.251, 0.334),

    // right 1
    glm::vec2(0.499, 0.666),
    glm::vec2(0.75, 0.666),
    glm::vec2(0.499, 0.334),
    glm::vec2(0.75, 0.666),
    glm::vec2(0.499, 0.334),
    glm::vec2(0.75, 0.334),

    // bottom
    glm::vec2(0.499, 0.0),
    glm::vec2(0.251, 0.0),
    glm::vec2(0.499, 0.334),
    glm::vec2(0.251, 0.0),
    glm::vec2(0.499, 0.334),
    glm::vec2(0.251, 0.334),

    // right 2
    glm::vec2(1.0, 0.666),
    glm::vec2(1.0, 0.334),
    glm::vec2(0.75, 0.666),
    glm::vec2(1.0, 0.334),
    glm::vec2(0.75, 0.666),
    glm::vec2(0.75, 0.334),

    // up
    glm::vec2(0.251, 0.666),
    glm::vec2(0.251, 1.0),
    glm::vec2(0.499, 0.666),
    glm::vec2(0.251, 1.0),
    glm::vec2(0.499, 0.666),
    glm::vec2(0.499, 1.0)
};

class Cube {

    private:
        GLuint vertex_array_id_;        // vertex array object
        GLuint program_id_;             // GLSL shader program ID
        GLuint vertex_buffer_object_;   // memory buffer
        GLuint texture_id_;             // texture ID
        glm::mat4 model_matrix_;        // model matrix

    public:
        void Init() {
            // compile the shaders.
            program_id_ = icg_helper::LoadShaders("cube_vshader.glsl",
                                                  "cube_fshader.glsl");
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
                glBufferData(GL_ARRAY_BUFFER, NbCubeVertices * sizeof(glm::vec3),
                             &CubeVertices[0], GL_STATIC_DRAW);

                // attribute
                GLuint vertex_point_id = glGetAttribLocation(program_id_, "vpoint");
                glEnableVertexAttribArray(vertex_point_id);
                glVertexAttribPointer(vertex_point_id, 3, GL_FLOAT, DONT_NORMALIZE,
                                      ZERO_STRIDE, ZERO_BUFFER_OFFSET);
            }

            // texture coordinates
            {
                // buffer
                glGenBuffers(1, &vertex_buffer_object_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
                glBufferData(GL_ARRAY_BUFFER, NbCubeUVs * sizeof(glm::vec2),
                             &CubeUVs[0], GL_STATIC_DRAW);

                // attribute
                GLuint vertex_texture_coord_id = glGetAttribLocation(program_id_,
                                                                     "vtexcoord");
                glEnableVertexAttribArray(vertex_texture_coord_id);
                glVertexAttribPointer(vertex_texture_coord_id, 2, GL_FLOAT,
                                      DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);
            }

            // load texture
            {
                int width;
                int height;
                int nb_component;
                string texture_filename = "tex_sky_0.tga";
                stbi_set_flip_vertically_on_load(1);
                unsigned char* image = stbi_load(texture_filename.c_str(),
                                                 &width, &height, &nb_component, 0);

                if(image == nullptr) {
                    throw(std::string("Failed to load texture"));
                }

                glGenTextures(1, &texture_id_);
                glBindTexture(GL_TEXTURE_2D, texture_id_);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

                if(nb_component == 3) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                                 GL_RGB, GL_UNSIGNED_BYTE, image);
                } else if(nb_component == 4) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, image);
                }

                GLuint tex_id = glGetUniformLocation(program_id_, "tex");
                glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);

                // cleanup
                glBindTexture(GL_TEXTURE_2D, 0);
                stbi_image_free(image);

            }

            // create the model matrix
            model_matrix_ = glm::scale(IDENTITY_MATRIX, glm::vec3(0.5f));
            model_matrix_ = glm::translate(model_matrix_,
                                           glm::vec3(0.0f, 0.0f, 0.6f));
        }

        void Cleanup() {
            glBindVertexArray(0);
            glUseProgram(0);
            glDeleteBuffers(1, &vertex_buffer_object_);
            glDeleteProgram(program_id_);
            glDeleteVertexArrays(1, &vertex_array_id_);
            glDeleteTextures(1, &texture_id_);
        }

        void Draw(const glm::mat4& view_projection){
            glUseProgram(program_id_);
            glBindVertexArray(vertex_array_id_);

            // bind textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_id_);

            // time
            glUniform1f(glGetUniformLocation(program_id_, "time"), glfwGetTime());

            // setup MVP
            glm::mat4 MVP = view_projection * model_matrix_;
            GLuint MVP_id = glGetUniformLocation(program_id_, "MVP");
            glUniformMatrix4fv(MVP_id, 1, GL_FALSE, value_ptr(MVP));

            // draw
            glDrawArrays(GL_TRIANGLES,0, NbCubeVertices);

            glBindVertexArray(0);
            glUseProgram(0);
        }
};
