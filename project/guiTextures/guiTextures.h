#ifndef GUITEXTURES_H
#define GUITEXTURES_H

#pragma once
#include "icg_helper.h"

class GuiTextures {

    private:
        GLuint vertex_array_id;
        GLuint program_id;
        GLuint vertex_buffer_object;

        class GuiTexture {
            private:
                float x;
                float y;
                float width;
                float height;
                int texture_id;
                int dimension; // 1D, 2D...
                int rotate180;

            public:
                GuiTexture(float x, float y, float width, float height, int texture_id, int dimension, int rotate180) {
                    this->x = x;
                    this->y = y;
                    this->width = width;
                    this->height = height;
                    this->texture_id = texture_id;
                    this->dimension = dimension;
                    this->rotate180 = rotate180;
                }

                void draw() {
                    glViewport(x, y, width, height);
                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                }

                GLuint getTexId() {
                    return texture_id;
                }

                int getDimension() {
                    return dimension;
                }

                int getRotate180() {
                    return rotate180;
                }
        };

        std::vector<GuiTexture> textures;

    public:
        void Init() {
            // compile the shaders
            program_id = icg_helper::LoadShaders("guiTextures_vshader.glsl",
                                                  "guiTextures_fshader.glsl");

            if(!program_id) {
                exit(EXIT_FAILURE);
            }

            glUseProgram(program_id);
            // generate one vertex Array
            glGenVertexArrays(1, &vertex_array_id);
            glBindVertexArray(vertex_array_id);

            // vertex coordinates
            {
                const GLfloat vertex_point[] = { /*V1*/ -1.0f, -1.0f, 0.0f,
                                                 /*V2*/ +1.0f, -1.0f, 0.0f,
                                                 /*V3*/ -1.0f, +1.0f, 0.0f,
                                                 /*V4*/ +1.0f, +1.0f, 0.0f};
                // buffer
                glGenBuffers(1, &vertex_buffer_object);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_point),
                             vertex_point, GL_STATIC_DRAW);

                // attribute
                GLuint vertex_point_id = glGetAttribLocation(program_id, "vertexCoord");
                glEnableVertexAttribArray(vertex_point_id);
                glVertexAttribPointer(vertex_point_id, 3, GL_FLOAT, DONT_NORMALIZE,
                                      ZERO_STRIDE, ZERO_BUFFER_OFFSET);
            }

            // texture coordinates
            {
                const GLfloat vertex_texture_coordinates[] = { /*V1*/ 0.0f, 0.0f,
                                                               /*V2*/ 1.0f, 0.0f,
                                                               /*V3*/ 0.0f, 1.0f,
                                                               /*V4*/ 1.0f, 1.0f};

                // buffer
                glGenBuffers(1, &vertex_buffer_object);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_texture_coordinates),
                             vertex_texture_coordinates, GL_STATIC_DRAW);

                // attribute
                GLuint vertex_texture_coord_id = glGetAttribLocation(program_id, "textureCoord");
                glEnableVertexAttribArray(vertex_texture_coord_id);
                glVertexAttribPointer(vertex_texture_coord_id, 2, GL_FLOAT,
                                      DONT_NORMALIZE, ZERO_STRIDE,
                                      ZERO_BUFFER_OFFSET);
            }

            // to avoid the current object being polluted
            glBindVertexArray(0);
            glUseProgram(0);
        }

        void Cleanup() {
            glBindVertexArray(0);
            glUseProgram(0);
            glDeleteBuffers(1, &vertex_buffer_object);
            glDeleteProgram(program_id);
            glDeleteVertexArrays(1, &vertex_array_id);
        }

        void add(float x, float y, float width, float height, int texture_id, int dimension, int rotate180 = 0) {
            textures.push_back(GuiTexture(x, y, width, height, texture_id, dimension, rotate180));
        }

        void clear() {
            while(textures.size() > 0) {
                textures.pop_back();
            }
        }

        void display() {
            glUseProgram(program_id);
            glBindVertexArray(vertex_array_id);

            for (int i = 0 ; i < textures.size() ; ++i) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, textures[i].getTexId());
                GLuint tex_id = glGetUniformLocation(program_id, "tex");
                glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);

                glUniform1i(glGetUniformLocation(program_id, "dimension"), textures[i].getDimension());
                glUniform1i(glGetUniformLocation(program_id, "rotate180"), textures[i].getRotate180());

                textures[i].draw();
            }

            glBindVertexArray(0);
            glUseProgram(0);
        }
};

#endif
