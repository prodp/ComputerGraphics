#ifndef WATERFRAMEBUFFER_H
#define WATERFRAMEBUFFER_H

#include "icg_helper.h"

class WaterFramebuffer {

    private:
        float width;
        float height;

        int REFLECTION_WIDTH = 320;
        int REFLECTION_HEIGHT = 320;

        int REFRACTION_WIDTH = 640;
        int REFRACTION_HEIGHT = 640;

        GLuint framebuffer_object_id;
        GLuint depth_render_buffer_id;
        GLuint reflection_texture_id;
        GLuint refraction_texture_id;

    public:
        void Bind(GLenum attachment) {
            if (attachment == GL_COLOR_ATTACHMENT0) {
                glViewport(0, 0, width, height);
            }
            else {
                glViewport(0, 0, width, height);
            }
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object_id);
            const GLenum buffers[] = { attachment };
            glDrawBuffers(1 /*length of buffers[]*/, buffers);
        }

        void Unbind() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        std::tuple<int, int> Init(float width, float height, bool use_interpolation) {
            this->width = width;
            this->height = height;

            // create reflection attachment
            {
                glActiveTexture(GL_TEXTURE0);
                glGenTextures(1, &reflection_texture_id);
                glBindTexture(GL_TEXTURE_2D, reflection_texture_id);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                if(use_interpolation){
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                } else {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                }

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0,
                             GL_RGB, GL_UNSIGNED_BYTE, NULL);
            }

            // create reflection attachment
            {
                glActiveTexture(GL_TEXTURE1);
                glGenTextures(1, &refraction_texture_id);
                glBindTexture(GL_TEXTURE_2D, refraction_texture_id);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                if(use_interpolation){
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                } else {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                }

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0,
                             GL_RGB, GL_UNSIGNED_BYTE, NULL);
            }

            // create render buffer (for depth channel)
            {
                glGenRenderbuffers(1, &depth_render_buffer_id);
                glBindRenderbuffer(GL_RENDERBUFFER, depth_render_buffer_id);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
            }

            // tie it all together
            {
                glGenFramebuffers(1, &framebuffer_object_id);
                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object_id);
                glFramebufferTexture2D(GL_FRAMEBUFFER,
                                       GL_COLOR_ATTACHMENT0 /*location = 0*/,
                                       GL_TEXTURE_2D,
                                       reflection_texture_id,
                                       0 /*level (mipmap)*/);

                glFramebufferTexture2D(GL_FRAMEBUFFER,
                                       GL_COLOR_ATTACHMENT1 /*location = 0*/,
                                       GL_TEXTURE_2D,
                                       refraction_texture_id,
                                       0 /*level (mipmap)*/);

                glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                          GL_DEPTH_ATTACHMENT,
                                          GL_RENDERBUFFER,
                                          depth_render_buffer_id);

                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
                    GL_FRAMEBUFFER_COMPLETE) {
                    cerr << "!!!ERROR: Framebuffer not OK :(" << endl;
                }
                glBindFramebuffer(GL_FRAMEBUFFER, 0); // avoid pollution
            }

            return std::make_tuple(reflection_texture_id, refraction_texture_id);
        }

        void Cleanup() {
            glDeleteTextures(1, &reflection_texture_id);
            glDeleteTextures(1, &refraction_texture_id);
            glDeleteRenderbuffers(1, &depth_render_buffer_id);
            glBindFramebuffer(GL_FRAMEBUFFER, 0 /*UNBIND*/);
            glDeleteFramebuffers(1, &framebuffer_object_id);
        }
};


#endif
