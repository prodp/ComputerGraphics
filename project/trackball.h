#pragma once
#include "icg_helper.h"


using namespace glm;

class Trackball {
public:
    Trackball() : radius_(1.0f) {}

    // this function is called when the user presses the left mouse button down.
    // x, and y are in [-1, 1]. (-1, -1) is the bottom left corner while (1, 1)
    // is the top right corner.
    void BeingDrag(float x, float y) {
      anchor_pos_ = vec3(x, y, 0.0f);
      ProjectOntoSurface(anchor_pos_);
    }

    // this function is called while the user moves the curser around while the
    // left mouse button is still pressed.
    // x, and y are in [-1, 1]. (-1, -1) is the bottom left corner while (1, 1)
    // is the top right corner.
    // returns the rotation of the trackball in matrix form.
    mat4 Drag(float x, float y) {
      vec3 current_pos = vec3(x, y, 0.0f);
      ProjectOntoSurface(current_pos);

      mat4 rotation = IDENTITY_MATRIX;

      if (anchor_pos_.x != current_pos.x || anchor_pos_.y != current_pos.y) {
          // 1. get rotation axis by cross product origin->anchor_pos_ x origin-> currentPos
          vec3 rotationAxis = glm::normalize( glm::cross(anchor_pos_,current_pos) );
          float u = rotationAxis.x;
          float v = rotationAxis.y;
          float w = rotationAxis.z;

          // 2. get rotationAngle
          float rotAngle = acos( glm::dot(anchor_pos_, current_pos) / (glm::length(current_pos)*glm::length(anchor_pos_)));

          // 3. compute the matrix (copypasta from http://inside.mines.edu/fs_home/gmurray/ArbitraryAxisRotation/)
          rotation = mat4( u*u + (1-u*u)*cos(rotAngle),             u*v*(1-cos(rotAngle)) + w*sin(rotAngle),    u*w*(1-cos(rotAngle))-v*sin(rotAngle),   0,
                           u*v*(1-cos(rotAngle)) - w*sin(rotAngle), v*v + (1-v*v)*cos(rotAngle),                v*w*(1-cos(rotAngle))+u*sin(rotAngle),   0,
                           u*w*(1-cos(rotAngle)) + v*sin(rotAngle), v*w*(1-cos(rotAngle))-u*sin(rotAngle),      w*w + (1-w*w)*cos(rotAngle),             0,
                           0,   0,  0,  1);
      }
      return rotation;
    }

private:
    // projects the point p (whose z coordiante is still empty/zero) onto the
    // trackball surface. If the position at the mouse cursor is outside the
    // trackball, use a hyberbolic sheet as explained in:
    // https://www.opengl.org/wiki/Object_Mouse_Trackball.
    // The trackball radius is given by 'radius_'.
    void ProjectOntoSurface(vec3& p) const {
      // TODO 2: Implement this function. Read above link for details.
      if( (p.x*p.x + p.y*p.y) < radius_*radius_/2 ) { // intersection of sphere/hyperplan
        projectOntoSphere(p);
      }else{
        projectOntoHyperplan(p);
      }
    }

    void projectOntoSphere(vec3& p) const {
        p.z = sqrt( radius_*radius_ - (p.x*p.x + p.y*p.y) );
    }

    void projectOntoHyperplan(vec3& p) const {
        p.z = radius_*radius_ / (2*sqrt(p.x*p.x + p.y*p.y));
    }

    float radius_;
    vec3 anchor_pos_;
    mat4 rotation_;
};
