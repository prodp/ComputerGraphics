#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "icg_helper.h"

class FrameBuffer {

    private:
        int width_;
        int height_;
        GLuint framebuffer_object_id_;
        GLuint depth_render_buffer_id_;
        GLuint height_map_texture_id;
        GLuint normal_texture_id;

    public:

        // warning: overrides viewport!!
        void Bind(GLenum attachment) {
            glViewport(0, 0, width_, height_);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object_id_);
            const GLenum buffers[] = { attachment };
            glDrawBuffers(1 /*length of buffers[]*/, buffers);
        }

        void Unbind() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        std::tuple<int, int> Init(int image_width, int image_height, bool use_interpolation = true) {
            this->width_ = image_width;
            this->height_ = image_height;

            // create height map attachment
            {
                glActiveTexture(GL_TEXTURE0);  // texture for pass 1 will be stored in GL_TEXTURE_0
                glGenTextures(1, &height_map_texture_id);
                glBindTexture(GL_TEXTURE_2D, height_map_texture_id);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                if(use_interpolation){
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                } else {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                }

                // create texture for the color attachment
                // see Table.2 on
                // khronos.org/opengles/sdk/docs/man3/docbook4/xhtml/glTexImage2D.xml
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width_, height_, 0,
                             GL_RGB, GL_UNSIGNED_BYTE, NULL);
                // how to load from buffer
            }

            // create normal attachment
            {
                glActiveTexture(GL_TEXTURE1);
                glGenTextures(1, &normal_texture_id);
                glBindTexture(GL_TEXTURE_2D, normal_texture_id);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                if(use_interpolation){
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                } else {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                }

                // create texture for the color attachment
                // see Table.2 on
                // khronos.org/opengles/sdk/docs/man3/docbook4/xhtml/glTexImage2D.xml
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width_, height_, 0,
                             GL_RGB, GL_UNSIGNED_BYTE, NULL);
                // how to load from buffer
            }

            // create render buffer (for depth channel)
            {
                glGenRenderbuffers(1, &depth_render_buffer_id_);
                glBindRenderbuffer(GL_RENDERBUFFER, depth_render_buffer_id_);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width_, height_);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
            }

            // tie it all together
            {
                glGenFramebuffers(1, &framebuffer_object_id_);
                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object_id_);
                glFramebufferTexture2D(GL_FRAMEBUFFER,
                                       GL_COLOR_ATTACHMENT0 /*location = 0*/,
                                       GL_TEXTURE_2D, height_map_texture_id,
                                       0 /*level*/);

                glFramebufferTexture2D(GL_FRAMEBUFFER,
                                       GL_COLOR_ATTACHMENT1 /*location = 0*/,
                                       GL_TEXTURE_2D, normal_texture_id,
                                       0 /*level*/);

                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                          GL_RENDERBUFFER, depth_render_buffer_id_);

                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
                    GL_FRAMEBUFFER_COMPLETE) {
                    cerr << "!!!ERROR: Framebuffer not OK :(" << endl;
                }
                glBindFramebuffer(GL_FRAMEBUFFER, 0); // avoid pollution
            }

            return std::make_tuple(height_map_texture_id, normal_texture_id);
        }

        float getHeight() {
            GLint format;
            glBindTexture(GL_TEXTURE_2D, height_map_texture_id);
            int size = width_ * height_;

            GLfloat* array = new GLfloat[size];
            for (int i = 0 ; i < size ; ++i) {
                array[i] = 1.0;
            }

            glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, array);

            int index = width_ * (height_/2) + (width_/2);

            return array[index];
        }

        void Cleanup() {
            glDeleteTextures(1, &height_map_texture_id);
            glDeleteTextures(1, &normal_texture_id);
            glDeleteRenderbuffers(1, &depth_render_buffer_id_);
            glBindFramebuffer(GL_FRAMEBUFFER, 0 /*UNBIND*/);
            glDeleteFramebuffers(1, &framebuffer_object_id_);
        }
};


#endif // FRAMEBUFFER.H
