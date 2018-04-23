#pragma once
#include "icg_helper.h"

#include "constants.h"
#include "imgui.h"

class ScreenQuad {

    private:
        GLuint vertex_array_id_;        // vertex array object
        GLuint noise_program_id_;             // GLSL shader program ID
        GLuint normal_program_id_;
        GLuint vertex_buffer_object_;   // memory buffer

        GLuint height_map_texture_id;
        GLuint normal_texture_id;             // textures IDs
        GLuint noise_texture_id;

        float screenquad_width_;
        float screenquad_height_;

        float gauss_std_dev;

        // parameters for the fragment shader
        int seed = 1;
        int octavesNb = 8;
        int gridSizeExp;
        int cellSizeExp=9;
        float lacunarity=2.356; // was 1.9
        float hParameter=1.0;
        float ridgedMFractalOffest=1.5;


        float offset[2] = {0.0, 0.0}; // offset of the grid
        float noise_density = 2.0;
        float noise_gradient = 5.0;
        float noise_size = 5.0;
        int flatTerrainActivated = 0;
        float water_level;

    public:
        GLuint Init(float screenquad_width, float screenquad_height,
                  GLuint height_map_tex_id, GLuint normal_tex_id, float gridSizeExp, float water_level) {

            this->gridSizeExp = gridSizeExp;
            this->water_level = water_level;

            // set screenquad size
            this->screenquad_width_ = screenquad_width;
            this->screenquad_height_ = screenquad_height;

            // compile the shaders
            noise_program_id_ = icg_helper::LoadShaders("screenquad_vshader.glsl",
                                                  "screenquad_fshader.glsl");
            normal_program_id_ = icg_helper::LoadShaders("normal_vshader.glsl",
                                                  "normal_fshader.glsl");
            if(!noise_program_id_) {
                exit(EXIT_FAILURE);
            }
            if(!normal_program_id_) {
                exit(EXIT_FAILURE);
            }

            glUseProgram(noise_program_id_);

            // vertex one vertex Array
            glGenVertexArrays(1, &vertex_array_id_);
            glBindVertexArray(vertex_array_id_);

            // vertex coordinates
            {
                const GLfloat vertex_point[] = { /*V1*/ -1.0f, -1.0f, 0.0f,
                                                 /*V2*/ +1.0f, -1.0f, 0.0f,
                                                 /*V3*/ -1.0f, +1.0f, 0.0f,
                                                 /*V4*/ +1.0f, +1.0f, 0.0f};
                // buffer
                glGenBuffers(1, &vertex_buffer_object_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_point),
                             vertex_point, GL_STATIC_DRAW);

                // attribute
                GLuint vertex_point_id = glGetAttribLocation(noise_program_id_, "vpoint");
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
                glGenBuffers(1, &vertex_buffer_object_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_texture_coordinates),
                             vertex_texture_coordinates, GL_STATIC_DRAW);

                // attribute
                GLuint vertex_texture_coord_id = glGetAttribLocation(noise_program_id_,
                                                                     "vtexcoord");
                glEnableVertexAttribArray(vertex_texture_coord_id);
                glVertexAttribPointer(vertex_texture_coord_id, 2, GL_FLOAT,
                                      DONT_NORMALIZE, ZERO_STRIDE,
                                      ZERO_BUFFER_OFFSET);
            }

            // load/Assign texture
            load_standard_texture("tex_noise_0.png", &noise_texture_id);
            this->height_map_texture_id = height_map_tex_id;
            this->normal_texture_id = normal_tex_id;

            // to avoid the current object being polluted
            glBindVertexArray(0);
            glUseProgram(0);

            return noise_texture_id;
        }

        void Cleanup() {
            glBindVertexArray(0);
            glUseProgram(0);
            glDeleteBuffers(1, &vertex_buffer_object_);
            glDeleteProgram(noise_program_id_);
            glDeleteProgram(normal_program_id_);
            glDeleteVertexArrays(1, &vertex_array_id_);
        }

        void UpdateSize(int screenquad_width, int screenquad_height) {
            this->screenquad_width_ = screenquad_width;
            this->screenquad_height_ = screenquad_height;
        }

        void Draw(DrawType drawType) {
            if (drawType == NOISE) {
                // bind textures
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, noise_texture_id);
                GLuint noise_tex_location = glGetUniformLocation(noise_program_id_, "noise_tex");
                glUniform1i(noise_tex_location, 1 /*GL_TEXTURE1*/);

                glUseProgram(noise_program_id_);
                glBindVertexArray(vertex_array_id_);

                glUniform1i(glGetUniformLocation(noise_program_id_, "flatTerrainActivated"), flatTerrainActivated);
                glUniform1i(glGetUniformLocation(noise_program_id_, "SEED"), seed);
                glUniform1i(glGetUniformLocation(noise_program_id_, "OCTAVES_NB"), octavesNb);
                glUniform1i(glGetUniformLocation(noise_program_id_, "GRID_SIZE_EXP"), gridSizeExp);
                glUniform1i(glGetUniformLocation(noise_program_id_, "CELL_SIZE_EXP"), cellSizeExp);
                glUniform1f(glGetUniformLocation(noise_program_id_, "LACUNARITY"), lacunarity);
                glUniform1f(glGetUniformLocation(noise_program_id_, "H"), hParameter);
                glUniform1f(glGetUniformLocation(noise_program_id_, "RIDGED_MFRACTAL_OFFSET"), ridgedMFractalOffest);

                glUniform2fv(glGetUniformLocation(noise_program_id_, "offset"), 1, offset);
                glUniform1f(glGetUniformLocation(noise_program_id_, "noise_density"), noise_density);
                glUniform1f(glGetUniformLocation(noise_program_id_, "noise_gradient"), noise_gradient);
                glUniform1f(glGetUniformLocation(noise_program_id_, "noise_size"), noise_size);
                glUniform1f(glGetUniformLocation(noise_program_id_, "water_level"), water_level);

                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
            else if (drawType == NORMAL) {

                glUseProgram(normal_program_id_);
                glBindVertexArray(vertex_array_id_);

                // window size uniforms
                glUniform1f(glGetUniformLocation(normal_program_id_, "tex_width"),
                            this->screenquad_width_);
                glUniform1f(glGetUniformLocation(normal_program_id_, "tex_height"),
                            this->screenquad_height_);

                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, height_map_texture_id);
                GLuint my_tex_id = glGetUniformLocation(normal_program_id_, "tex");
                glUniform1i(my_tex_id, 1);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }

            glBindVertexArray(0);
            glUseProgram(0);
        }


    int* getSeed() {
        return &seed;
    }

    int* getOctaveNb() {
        return &octavesNb;
    }

    int* getGridSizeExp() {
        return &gridSizeExp;
    }

    int* getCellSizeExp(){
        return &cellSizeExp;
    }

    float* getLacunarity(){
        return &lacunarity;
    }

    float* getH(){
        return &hParameter;
    }

    float* getRidgedFractalOffset(){
        return &ridgedMFractalOffest;
    }

    float* getOffset() {
        return offset;
    }

    float* getNoiseDensity() {
        return &noise_density;
    }

    float* getNoiseGradient() {
        return &noise_gradient;
    }

    float* getNoiseSize() {
        return &noise_size;
    }

    void setOffset(float dx, float dy) {
        offset[0] += dx;
        offset[1] += dy;
    }

    void setGridSizeExp(float value) {
        this->gridSizeExp = value;
    }

    void setWaterLevel(float value) {
        this->water_level = value;
    }

    void setFlatTerrainActivated(bool activated) {
        if (activated == true) {
            this->flatTerrainActivated = 1;
        }
        else {
            this->flatTerrainActivated = 0;
        }
    }

    void load_standard_texture(string filename, GLuint* tex_id) {
        int width;
        int height;
        int nb_component;
        // set stb_image to have the same coordinates as OpenGL
        stbi_set_flip_vertically_on_load(1);

        unsigned char* image = stbi_load(filename.c_str(), &width,
                                         &height, &nb_component, 0);
        if(image == nullptr) {
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
};
