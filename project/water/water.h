#pragma once
#include "icg_helper.h"
#include "constants.h"
#include <glm/gtc/type_ptr.hpp>

class Water {

    private:
        GLuint vertex_array_id_;                // vertex array object
        GLuint vertex_buffer_object_position_;  // memory buffer for positions
        GLuint vertex_buffer_object_index_;     // memory buffer for indices
        GLuint program_id_;                     // GLSL shader program ID
        GLuint reflection_texture_id;
        GLuint refraction_texture_id;
        GLuint height_map_texture_id;
        GLuint dudv_map_id;
        GLuint normal_map_texture_id;
        GLuint sky_texture_id;

        GLuint num_indices_;                    // number of vertices to render
        GLuint MVP_id_;                         // model, view, proj matrix ID

        float offset[2] = {0.0, 0.0};
        float water_level;
        float wave_speed = 0.01;
        int dudv_map_size = 15;
        float wave_strenght = 0.01;
        float reflectiveness = 5.0;
        glm::vec3 light_pos;
        glm::vec3 light_color;
        glm::vec3 light_reflectivity;
        float fog_density = FOG_DENSITY;
        float fog_gradient = FOG_GRADIENT;
        float amplitude = AMPLITUDE;

    public:
        void Init(float water_level, GLuint reflection_texture_id, GLuint refraction_texture_id, GLuint height_map_texture_id,
                  GLuint sky_texture_id, glm::vec3 light_pos, glm::vec3 light_color, glm::vec3 light_reflectivity) {
            this->water_level = water_level;
            this->reflection_texture_id = reflection_texture_id;
            this->refraction_texture_id = refraction_texture_id;
            this->height_map_texture_id = height_map_texture_id;
            this->sky_texture_id = sky_texture_id;
            this->light_pos = light_pos;
            this->light_color = light_color;
            this->light_reflectivity = light_reflectivity;

            // compile the shaders.
            program_id_ = icg_helper::LoadShaders("water_vshader.glsl",
                                                  "water_fshader.glsl");
            if(!program_id_) {
                exit(EXIT_FAILURE);
            }

            glUseProgram(program_id_);

            // vertex one vertex array
            glGenVertexArrays(1, &vertex_array_id_);
            glBindVertexArray(vertex_array_id_);

            // vertex coordinates and indices
            {
                std::vector<GLfloat> vertices;
                std::vector<GLuint> indices;
                int grid_dim = 100;

                for (int i = 0 ; i < grid_dim-1 ; ++i) {
                    if (i % 2 == 0) {
                        for (int j = 0 ; j < grid_dim*2-1 ; ++j) {
                            if (j % 2 == 0) {
                                vertices.push_back(-1.0f + ((float)j)/(grid_dim-1));
                                vertices.push_back( -1.0f + ((float)i*2.0f)/(grid_dim-1));
                            }
                            else {
                                vertices.push_back(-1.0f + ((float)(j-1))/(grid_dim-1));
                                vertices.push_back( -1.0f + ((i+1)*2.0)/(grid_dim-1));
                            }
                            indices.push_back(i*(2*grid_dim-1) + j);
                        }
                    }
                    else {
                        for (int j = 0 ; j < grid_dim*2-1 ; ++j) {
                            if (j % 2 == 0) {
                                vertices.push_back(1.0f - ((float)j)/(grid_dim-1));
                                vertices.push_back( -1.0f + (float)i*2.0f/(grid_dim-1));
                            }
                            else {
                                vertices.push_back(1.0f - (float)(j-1)/(grid_dim-1));
                                vertices.push_back( -1.0f + (i+1)*2.0f/(grid_dim-1));
                            }
                            indices.push_back(i*(2*grid_dim-1) + j);
                        }
                    }
                }

                if (grid_dim % 2 == 1) {
                    vertices.push_back(-1.0f);
                    vertices.push_back(1.0f);
                }
                else {
                    vertices.push_back(1.0f);
                    vertices.push_back(1.0f);
                }
                indices.push_back((grid_dim-1)*(grid_dim*2-1));

                num_indices_ = indices.size();

                // position buffer
                glGenBuffers(1, &vertex_buffer_object_position_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_position_);
                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat),
                             &vertices[0], GL_STATIC_DRAW);

                // vertex indices
                glGenBuffers(1, &vertex_buffer_object_index_);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffer_object_index_);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                             &indices[0], GL_STATIC_DRAW);

                // position shader attribute
                GLuint loc_position = glGetAttribLocation(program_id_, "position");
                glEnableVertexAttribArray(loc_position);
                glVertexAttribPointer(loc_position, 2, GL_FLOAT, DONT_NORMALIZE,
                                      ZERO_STRIDE, ZERO_BUFFER_OFFSET);
            }

            // load dudv texture
            {
                int width;
                int height;
                int nb_component;
                string filename = "water_dudv_6.png";
                // set stb_image to have the same coordinates as OpenGL
                stbi_set_flip_vertically_on_load(1);

                unsigned char* image = stbi_load(filename.c_str(), &width,
                                                 &height, &nb_component, 0);
                if(image == nullptr) {
                    throw(string("Failed to load texture"));
                }

                glGenTextures(1, &dudv_map_id);
                glBindTexture(GL_TEXTURE_2D, dudv_map_id);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

                if(nb_component == 3) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                                 GL_RGB, GL_UNSIGNED_BYTE, image);
                } else if(nb_component == 4) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, image);
                }

                // cleanup
                glBindTexture(GL_TEXTURE_2D, 0);
                stbi_image_free(image);
            }

            // load normal map texture
            {
                int width;
                int height;
                int nb_component;
                string filename = "normal_map_6.png";
                // set stb_image to have the same coordinates as OpenGL
                stbi_set_flip_vertically_on_load(1);

                unsigned char* image = stbi_load(filename.c_str(), &width,
                                                 &height, &nb_component, 0);
                if(image == nullptr) {
                    throw(string("Failed to load texture"));
                }

                glGenTextures(1, &normal_map_texture_id);
                glBindTexture(GL_TEXTURE_2D, normal_map_texture_id);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

                if(nb_component == 3) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                                 GL_RGB, GL_UNSIGNED_BYTE, image);
                } else if(nb_component == 4) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, image);
                }

                // cleanup
                glBindTexture(GL_TEXTURE_2D, 0);
                stbi_image_free(image);
            }

            // other uniforms
            MVP_id_ = glGetUniformLocation(program_id_, "MVP");
            glUniform1f(glGetUniformLocation(program_id_, "water_level"), water_level);

            // to avoid the current object being polluted
            glBindVertexArray(0);
            glUseProgram(0);
        }

        void Cleanup() {
            glBindVertexArray(0);
            glUseProgram(0);
            glDeleteBuffers(1, &vertex_buffer_object_position_);
            glDeleteBuffers(1, &vertex_buffer_object_index_);
            glDeleteVertexArrays(1, &vertex_array_id_);
            glDeleteProgram(program_id_);
            glDeleteTextures(1, &reflection_texture_id);
            glDeleteTextures(1, &refraction_texture_id);
            glDeleteTextures(1, &dudv_map_id);
        }

        void Draw(float time,
                  glm::vec3 camPos,
                  const glm::mat4 &model = IDENTITY_MATRIX,
                  const glm::mat4 &view = IDENTITY_MATRIX,
                  const glm::mat4 &projection = IDENTITY_MATRIX) {
            glUseProgram(program_id_);
            glBindVertexArray(vertex_array_id_);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, reflection_texture_id);
            GLuint reflect_tex_id = glGetUniformLocation(program_id_, "reflectionTexture");
            glUniform1i(reflect_tex_id, 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, refraction_texture_id);
            GLuint refract_tex_id = glGetUniformLocation(program_id_, "refractionTexture");
            glUniform1i(refract_tex_id, 1);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, dudv_map_id);
            GLuint dudv_map_location = glGetUniformLocation(program_id_, "dudv_map");
            glUniform1i(dudv_map_location, 2);

            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, height_map_texture_id);
            GLuint height_map_texture_location = glGetUniformLocation(program_id_, "heightTexture");
            glUniform1i(height_map_texture_location, 3);

            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, normal_map_texture_id);
            GLuint normal_map_texture_location = glGetUniformLocation(program_id_, "normal_map");
            glUniform1i(normal_map_texture_location, 4);

            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, sky_texture_id);
            GLuint sky_texture_location = glGetUniformLocation(program_id_, "sky_texture");
            glUniform1i(sky_texture_location, 5);

            // setup MVP
            glm::mat4 MVP = projection*view*model;
            glUniformMatrix4fv(MVP_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(MVP));

            // uniforms
            glUniform1f(glGetUniformLocation(program_id_, "time"), time);
            glUniform2fv(glGetUniformLocation(program_id_, "offset"), 1, offset);
            glUniform1f(glGetUniformLocation(program_id_, "water_level"), water_level);
            float move_factor = fmod((wave_speed * time/10.0), 1.0);
            glUniform1f(glGetUniformLocation(program_id_, "move_factor"), move_factor);
            glUniform1f(glGetUniformLocation(program_id_, "wave_strenght"), wave_strenght);
            glUniform1i(glGetUniformLocation(program_id_, "dudv_map_size"), dudv_map_size);
            glUniform1f(glGetUniformLocation(program_id_, "water_level"), water_level);
            glUniform1f(glGetUniformLocation(program_id_, "reflectiveness"), reflectiveness);
            glUniform3f(glGetUniformLocation(program_id_, "light_pos"), light_pos.x, light_pos.y, light_pos.z);
            glUniform3f(glGetUniformLocation(program_id_, "light_color"), light_color.x, light_color.y, light_color.z);
            glUniform3f(glGetUniformLocation(program_id_, "light_reflectivity"), light_reflectivity.x, light_reflectivity.y, light_reflectivity.z);
            glUniform1f(glGetUniformLocation(program_id_, "fog_density"), fog_density);
            glUniform1f(glGetUniformLocation(program_id_, "fog_gradient"), fog_gradient);
            glUniform1f(glGetUniformLocation(program_id_, "amplitude"), amplitude);

            glUniformMatrix4fv(glGetUniformLocation(program_id_, "model"), ONE, DONT_TRANSPOSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(program_id_, "view"), ONE, DONT_TRANSPOSE, glm::value_ptr(view));

            // draw only the wireframe. : uncomment the next line.
            //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            glDrawElements(GL_TRIANGLE_STRIP, num_indices_, GL_UNSIGNED_INT, 0);

            glBindVertexArray(0);
            glUseProgram(0);
        }

        void setOffset(float dx, float dy) {
            offset[0] += dx;
            offset[1] += dy;
        }

        float* getWaterLevel() {
            return &water_level;
        }

        float* getWaveSpeed() {
            return &wave_speed;
        }

        float* getWaveStrenght() {
            return &wave_strenght;
        }

        int* getDudvMapSize() {
            return &dudv_map_size;
        }

        float* getReflectiveness() {
            return &reflectiveness;
        }

        float* getFogDensity() {
            return &fog_density;
        }

        float* getFogGradient() {
            return &fog_gradient;
        }

        void setAmplitude(float value) {
            amplitude = value;
        }

        void setFogDensity(float value) {
            fog_density = value;
        }

        void setFogGradient(float value) {
            fog_gradient = value;
        }
};
