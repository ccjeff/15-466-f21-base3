#pragma once
#include "Scene.hpp"
#include "Sound.hpp"
#include <glm/glm.hpp>

struct Stone {
    /* data */
    Scene::Transform *t;
    glm::vec3 speed;
    std::shared_ptr< Sound::PlayingSample > words;
    Stone(Scene::Transform *t_, glm::vec3 &speed_, std::shared_ptr< Sound::PlayingSample > words_) : t(t_), speed(speed_), words(words_) {}
    float distance(Scene::Transform *trans);
    void move(glm::vec3 &dest);
    void update(float elapsed);
};
