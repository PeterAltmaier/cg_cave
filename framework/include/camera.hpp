#pragma once

#include "common.hpp"

struct camera {
    camera(GLFWwindow* window, unsigned int plane_width, unsigned int plane_depth);

    virtual ~camera();

    void static keycallback(GLFWwindow *window, int key,float delta_time);

    glm::mat4
    view_matrix() const;

    bool is_not_dragging_w_momentum();

    void apply_after_momentum();

    glm::vec3
    position() const;

    glm::vec3 getDirection();
};
