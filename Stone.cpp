#include "Stone.hpp"
#include <cmath>
#include <random>

float Stone::distance(Scene::Transform *trans) {
    auto this_pos = this->t->position;
    auto t_pos = trans->position;
    return std::sqrt((this_pos.x - t_pos.x) * (this_pos.x - t_pos.x) 
        + (this_pos.y - t_pos.y) * (this_pos.y - t_pos.y)
        + (this_pos.z - t_pos.z) * (this_pos.z - t_pos.z));
}

void Stone::move(glm::vec3 &dest) {
    // REFERENCE: random number generator from src: https://stackoverflow.com/questions/7560114/random-number-c-in-some-range
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> xdistr(-5, 5); // define the range
    std::uniform_int_distribution<> ydistr(150, 200); // define the range
    std::uniform_int_distribution<> heightDist(-1, 1);
    this->t->position.x = dest.x - xdistr(gen);
    this->t->position.y = dest.y - ydistr(gen);
    this->t->position.z = dest.z + heightDist(gen);
    words->set_position(this->t->position);
    words->set_volume(5.0f);
}

void Stone::update(float elapsed) {
    this->t->position -= this->speed * elapsed * 3.0f;
    words->set_position(this->t->position);
}