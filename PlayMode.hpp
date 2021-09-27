#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"
#include "Stone.hpp"
#include "Load.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, space, larr, rarr, uarr, darr;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//hexapod leg to wobble:
	Scene::Transform *propeller = nullptr;
	Scene::Transform *plane = nullptr;
	Scene::Transform *stone = nullptr;

	std::vector<std::shared_ptr<Stone>> stones; 
	// std::vector<std::shared_ptr<Sound::PlayingSample>> all_words;

	glm::quat prop_base_rotation;
	glm::quat plane_base_rotation;
	glm::quat plane_curr_rotation;

	float wobble = 0.0f;
	float countdown = 15.0f; // the time that a new stone is pushed to be moving. 
	uint8_t stone_number = 5;
	float engine_vol = 0.0f;

	static constexpr float LOW_BRIGHTNESS = 0.1f;
	static constexpr float MED_BRIGHTNESS = 0.4f;
	static constexpr float HIGH_BRIGHTNESS = 1.0f;
	float brightness = 1.0f;
	

	glm::vec3 get_prop_position();

	//music coming from the tip of the leg (as a demonstration):
	std::shared_ptr< Sound::PlayingSample > leg_tip_loop;
	std::shared_ptr< Sound::PlayingSample > engine_play;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
