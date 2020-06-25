#pragma once

#include "common.hpp"

struct camera {
    camera(GLFWwindow* window);

    virtual ~camera();

    void static keycallback(GLFWwindow *window, int key,float delta_time);

    glm::mat4
    view_matrix() const;

    glm::vec3
    position() const;
};
