#pragma once
#include "icg_helper.h"
#include "constants.h"
#include <glm/gtc/type_ptr.hpp>

class Grid {

    private:
        GLuint vertex_buffer_object_index_;     // memory buffer for indices
        GLuint num_indices_;                    // number of vertices to render
        GLuint MVPReversed_id_;                         // model, view, proj matrix ID
        GLuint MVPNormal_id_;

        GLuint vertex_array_id_;                // vertex array object
        GLuint program_id_;                     // GLSL shader program ID
        GLuint vertex_buffer_object_position_;  // memory buffer for positions

        GLuint height_map_texture_id;           // computed textures
        GLuint normal_texture_id;
        GLuint sky_texture_id;
        GLuint noise_texture_id;

        GLuint bedrock_tex_id;                  // terrain textures, from bottom to top
        GLuint seaside_tex_id;                  // to change them, copy/paste the wanted file to the existing filename, it's easier & no need to change any code
        GLuint plain_tex_id;
        GLuint mountainside_tex_id;
        GLuint rock_tex_id;
        GLuint snowyrock_tex_id;
        GLuint snow_tex_id;

        GLuint vertex_buffer_object_;

        float offset[2] = {0.0, 0.0};

        // parameters for the shaders
        float water_level;
        float amplitude = AMPLITUDE;
        int gridSizeExp;
        int LOD_active;
        bool wireframeMode = false;
        glm::vec3 camPos;
        float fog_density = FOG_DENSITY;
        float fog_gradient = FOG_GRADIENT;
        float steepness_threshold_mixedrock = 0.838;
        float steepness_threshold_purerock = 0.852;

    public:
        void Init(GLuint height_map_tex_id, GLuint normal_tex_id, GLuint sky_texture_id, int gridSizeExp, float water_level, glm::vec3 camPos,
                  int LOD_active) {
            this->camPos = camPos;
            this->gridSizeExp = gridSizeExp;
            this->water_level = water_level;
            this->LOD_active = LOD_active;
            // compile the shaders.
            program_id_ = icg_helper::LoadShaders("grid_vshader.glsl",
                                                  "grid_fshader.glsl");
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
                if (LOD_active == 1) {
                    int big_grid_dim = 512;
                    int grid_dim = 256;
                    float size_big_square = 2.0 / grid_dim;

                    int indicesIndex = 0;

                    for (int i = 0 ; i < grid_dim ; ++i) {
                        for (int j = 0 ; j < grid_dim ; ++j) {
                            int resolution_big_square = big_grid_dim/grid_dim;
                            // First we compute the distance from the origin
                            float distance = sqrt(pow((i - (grid_dim/2)), 2) + pow((j - (grid_dim/2)), 2));

                            if (distance < grid_dim*0.1) {
                                resolution_big_square = resolution_big_square + 4;
                            }
                            else if (distance < grid_dim*0.15) {
                                resolution_big_square = resolution_big_square + 3;
                            }
                            else if (distance < grid_dim*0.2) {
                                resolution_big_square = resolution_big_square + 2;
                            }
                            else if (distance < grid_dim*0.25) {
                                resolution_big_square = resolution_big_square + 1;
                            }
                            addSquareOfDim(&vertices, &indices, resolution_big_square, size_big_square,
                                                                   -1 + j*size_big_square, 1 - i*size_big_square, &indicesIndex);
                        }
                    }
                }
                else{
                // generate mesh
                    int grid_dim = 512;
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
                }

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

            // texture coordinates
            {
                const GLfloat vertex_texture_coordinates[] = { /*V1*/ 0.0f, 0.0f,
                                                               /*V2*/ 1.0f, 0.0f,
                                                               /*V3*/ 0.0f, 1.0f,
                                                               /*V4*/ 1.0f, 1.0f};

                // buffer
                glGenBuffers(1, &vertex_buffer_object_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_texture_coordinates),
                vertex_texture_coordinates, GL_STATIC_DRAW);

                // attribute
                GLuint vertex_texture_coord_id = glGetAttribLocation(program_id_,
                                                                     "vtexcoord");
                glEnableVertexAttribArray(vertex_texture_coord_id);
                glVertexAttribPointer(vertex_texture_coord_id, 2, GL_FLOAT,
                                      DONT_NORMALIZE, ZERO_STRIDE,
                                      ZERO_BUFFER_OFFSET);
            }

            // load terrain textures

            load_standard_texture("tex_bedrock.png", &bedrock_tex_id);
            load_standard_texture("tex_seaside.png", &seaside_tex_id);
            load_standard_texture("tex_plain.png", &plain_tex_id);
            load_standard_texture("tex_mountainside.png", &mountainside_tex_id);
            load_standard_texture("tex_rock.png", &rock_tex_id);
            load_standard_texture("tex_snowyrock.png", &snowyrock_tex_id);
            load_standard_texture("tex_snow.png", &snow_tex_id);
            load_standard_texture("tex_noise_0.png", &noise_texture_id);

            // save texture reference
            this->height_map_texture_id = height_map_tex_id;
            this->normal_texture_id = normal_tex_id;
            this->sky_texture_id = sky_texture_id;

            // other uniforms
            MVPReversed_id_ = glGetUniformLocation(program_id_, "MVPReversed");
            MVPNormal_id_ = glGetUniformLocation(program_id_, "MVPNormal");

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
        }

        void Draw(float time, glm::vec4 plane, const glm::mat4 &model = IDENTITY_MATRIX,
                  const glm::mat4 &viewReversed = IDENTITY_MATRIX,
                  const glm::mat4 &viewNormal = IDENTITY_MATRIX,
                  const glm::mat4 &projection = IDENTITY_MATRIX,
                  float drawFogEnabled = 1.0) {

            glUseProgram(program_id_);
            glBindVertexArray(vertex_array_id_);

            // bind textures
            bindTexture2D(GL_TEXTURE0, height_map_texture_id, "height_tex", 0 /*GL_TEXTURE0*/);
            bindTexture2D(GL_TEXTURE1, normal_texture_id, "normal_tex", 1 /*GL_TEXTURE1*/);
            bindTexture2D(GL_TEXTURE2, noise_texture_id, "noise_tex", 2 /*GL_TEXTURE2*/);
            bindTexture2D(GL_TEXTURE3, sky_texture_id, "sky_tex", 3 /*GL_TEXTURE3*/);
            bindTexture2D(GL_TEXTURE4, bedrock_tex_id, "bedrock_tex", 4 /*GL_TEXTURE4*/);
            bindTexture2D(GL_TEXTURE5, seaside_tex_id, "seaside_tex", 5 /*GL_TEXTURE5*/);
            bindTexture2D(GL_TEXTURE6, plain_tex_id, "plain_tex", 6 /*GL_TEXTURE6*/);
            bindTexture2D(GL_TEXTURE7, mountainside_tex_id, "mountainside_tex", 7 /*GL_TEXTURE7*/);
            bindTexture2D(GL_TEXTURE8, rock_tex_id, "rock_tex", 8 /*GL_TEXTURE8*/);
            bindTexture2D(GL_TEXTURE9, snowyrock_tex_id, "snowyrock_tex", 9 /*GL_TEXTURE9*/);
            bindTexture2D(GL_TEXTURE10, snow_tex_id, "snow_tex", 10 /*GL_TEXTURE10*/);


            // uniforms
            glUniform1f(glGetUniformLocation(program_id_, "water_level"), water_level);
            glUniform1f(glGetUniformLocation(program_id_, "gridSizeExp"), gridSizeExp);
            glUniform1f(glGetUniformLocation(program_id_, "amplitude"), amplitude);
            glUniform4f(glGetUniformLocation(program_id_, "plane"), plane.x, plane.y, plane.z, plane.w);
            glUniform2fv(glGetUniformLocation(program_id_, "offset"), 1, offset);
            glUniform1f(glGetUniformLocation(program_id_, "fog_density"), fog_density);
            glUniform1f(glGetUniformLocation(program_id_, "fog_gradient"), fog_gradient);
            glUniform1f(glGetUniformLocation(program_id_, "steepness_threshold_mixedrock"), steepness_threshold_mixedrock);
            glUniform1f(glGetUniformLocation(program_id_, "steepness_threshold_purerock"), steepness_threshold_purerock);
            glUniform1f(glGetUniformLocation(program_id_, "drawFogEnabled"), drawFogEnabled);

            GLint model_id = glGetUniformLocation(program_id_, "model");
            glUniformMatrix4fv(model_id, ONE, DONT_TRANSPOSE, glm::value_ptr(model));
            GLint viewReversed_id = glGetUniformLocation(program_id_, "viewReversed");
            glUniformMatrix4fv(viewReversed_id, ONE, DONT_TRANSPOSE, glm::value_ptr(viewReversed));
            GLint viewNormal_id = glGetUniformLocation(program_id_, "viewNormal");
            glUniformMatrix4fv(viewNormal_id, ONE, DONT_TRANSPOSE, glm::value_ptr(viewNormal));

            // setup MVP
            glm::mat4 MVPReversed = projection*viewReversed*model;
            glUniformMatrix4fv(MVPReversed_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(MVPReversed));
            glm::mat4 MVPNormal = projection*viewNormal*model;
            glUniformMatrix4fv(MVPNormal_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(MVPNormal));

            // pass the current time stamp to the shader.
            glUniform1f(glGetUniformLocation(program_id_, "time"), time);


            // uncomment to draw in wireframe mode.
            if (wireframeMode) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            // Depending on how you set up your vertex index buffer, you
            // might have to change GL_TRIANGLE_STRIP to GL_TRIANGLES.
            if (LOD_active) {
                glDrawElements(GL_TRIANGLES, num_indices_, GL_UNSIGNED_INT, 0);
            }
            else {
                glDrawElements(GL_TRIANGLE_STRIP, num_indices_, GL_UNSIGNED_INT, 0);
            }
            if (wireframeMode) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Because the polygon mode wireframe is only for this draw !
            }

            glBindVertexArray(0);
            glUseProgram(0);
        }



/*
 *  HELPERS
 */
        // Loads content from the given file as a texture
        void load_standard_texture(string filename, GLuint* tex_id) {
            int width;
            int height;
            int nb_component;
            // set stb_image to have the same coordinates as OpenGL
            stbi_set_flip_vertically_on_load(1);

            unsigned char* image = stbi_load(filename.c_str(), &width,
                                             &height, &nb_component, 0);
            if(image == nullptr) {
                cout << "\ngrid.g : Failed to load texture " << filename << "\n";
                throw(string("Failed to load texture"));
            }

            glGenTextures(1, tex_id);
            glBindTexture(GL_TEXTURE_2D, *tex_id);
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

        // binds the texture according the given parameters
        void bindTexture2D(GLuint GL_TEXTUREX, GLuint texture_id, const char * uniform_name, int texture_nb){
            glActiveTexture(GL_TEXTUREX);
            glBindTexture(GL_TEXTURE_2D, texture_id);
            GLuint tex_location = glGetUniformLocation(program_id_, uniform_name);
            glUniform1i(tex_location, texture_nb);
        }

        // Adds matching vertices / square number in vertices and indices for a rectangle of given position and dimension
        void addSquareOfDim(std::vector<GLfloat> *vertices, std::vector<GLuint> *indices,
                            int dimension, float sideSize,
                            float upperLeftX, float upperLeftY,
                            int *curIndice) {

            // for each line
            for (int j = 0 ; j < dimension ; ++j) {
                // We put a full line, for example for dimension 2 we need 4 triangles
                // to have two square on one line
                for (int i = 0 ; i < dimension ; ++i) {
                    float sizeSmallSquare = sideSize/dimension;
                    addVerticesSimpleSquare(vertices, indices, upperLeftX + i*sizeSmallSquare,
                                                               upperLeftY - j*sizeSmallSquare,
                                                               upperLeftX + (i+1)*sizeSmallSquare,
                                                               upperLeftY - (j+1)*sizeSmallSquare,
                                                               curIndice);
                }
            }
        }

        /* Add the square into the vertices array (using two triangles),
         * add matching indice to indices array, and increment indices
         *      vertices    : array of triangle's vertices
         *      indices     : contains the matching rectangle number for each triangle vertex (matching by indices with vertices)
         */
        void addVerticesSimpleSquare(std::vector<GLfloat> *vertices, std::vector<GLuint> *indices,
                                  float upperLeftX, float upperLeftY,
                                  float lowerRightX, float lowerRightY,
                                  int *curIndice) {
            // Upper left vertex
            vertices->push_back(upperLeftX);
            vertices->push_back(upperLeftY);
            // Lower left vertex
            vertices->push_back(upperLeftX);
            vertices->push_back(lowerRightY);
            // Upper right vertex
            vertices->push_back(lowerRightX);
            vertices->push_back(upperLeftY);
            // Note that the second triangle shares 2 common vertices
            // Lower left vertex (copied from above)
            vertices->push_back(upperLeftX);
            vertices->push_back(lowerRightY);
            // Upper right vertex (copied from above)
            vertices->push_back(lowerRightX);
            vertices->push_back(upperLeftY);
            // Lower right vertex
            vertices->push_back(lowerRightX);
            vertices->push_back(lowerRightY);

            for (int i = 0 ; i < 6 ; ++i) {
                indices->push_back(*curIndice);
                (*curIndice)++;
            }
        }


/*
 *  SETTERS / GETTERS
 */

        void setWaterLevel(float water_level) {
            this->water_level = water_level;
        }

        bool* getWireFrameMode() {
            return &wireframeMode;
        }

        float*getAmplitude() {
            return &amplitude;
        }

        int *getLOD() {
            return &LOD_active;
        }

        float*getFogDensity() {
            return &fog_density;
        }

        float*getFogGradient() {
            return &fog_gradient;
        }

        float*getSteepnessThresholdMixRock() {
            return &steepness_threshold_mixedrock;
        }

        float*getSteepnessThresholdPureRock() {
            return &steepness_threshold_purerock;
        }

        glm::vec2 getOffset() {
            return glm::vec2(offset[0], offset[1]);
        }

        void setGridSizeExp(float value) {
            this->gridSizeExp = value;
        }

        void setCamPos(glm::vec3 value) {
            this->camPos = value;
        }

        void setOffset(float dx, float dy) {
            offset[0] += dx;
            offset[1] += dy;
        }

        void setFog(float density, float gradient) {
            fog_density = density;
            fog_gradient = gradient;
        }
};
