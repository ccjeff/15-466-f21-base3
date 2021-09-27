#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "Stone.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("hexapod.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("hexapod.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("bgm.opus"));
});

Load< Sound::Sample> engine_sample(LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("propeller.wav"));
});

Load< Sound::Sample> word1(LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("words1.wav"));
});

Load< Sound::Sample> word2(LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("words2.wav"));
});

Load< Sound::Sample> word3(LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("words3.wav"));
});

Load< Sound::Sample> word4(LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("words4.wav"));
});

Load< Sound::Sample> word5(LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("words5.wav"));
});

Load< Sound::Sample> word6(LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("words6.wav"));
});

PlayMode::PlayMode() : scene(*hexapod_scene) {
	//get pointers to leg for convenience:
	int counter = 0;
	for (auto &transform : scene.transforms) {
		std::cout << transform.name.substr(0,5) << '\n';
		if (transform.name == "propeller") propeller = &transform;
		else if (transform.name == "plane") plane = &transform;
		else if (transform.name.substr(0,5) == "stone") {
			// have tried to replace this with looping vector, failed somehow
			transform.position = glm::vec3(99999999.0f, 99999999.0f, 99999999.0f);
			if (counter == 0) {
				stones.emplace_back(std::make_shared<Stone>(&transform, propeller->position, Sound::loop_3D(*word1, 0.0f, transform.position, 2.0f)));
				counter++;
			} else if (counter == 1) {
				stones.emplace_back(std::make_shared<Stone>(&transform, propeller->position, Sound::loop_3D(*word2, 0.0f, transform.position, 2.0f)));
				counter++;
			} else if (counter == 2) {
				stones.emplace_back(std::make_shared<Stone>(&transform, propeller->position, Sound::loop_3D(*word3, 0.0f, transform.position, 2.0f)));
				counter++;
			} else if (counter == 3) {
				stones.emplace_back(std::make_shared<Stone>(&transform, propeller->position, Sound::loop_3D(*word4, 0.0f, transform.position, 2.0f)));
				counter++;
			} else if (counter == 4) {
				stones.emplace_back(std::make_shared<Stone>(&transform, propeller->position, Sound::loop_3D(*word5, 0.0f, transform.position, 2.0f)));
				counter++;
			} else if (counter == 5) {
				stones.emplace_back(std::make_shared<Stone>(&transform, propeller->position, Sound::loop_3D(*word6, 0.0f, transform.position, 2.0f)));
				counter++;
			}
			counter = counter % 6;
		};
	}
	if (propeller == nullptr) throw std::runtime_error("propeller not found.");
	if (plane == nullptr) throw std::runtime_error("plane not found.");
	if (stones.size() == 0) throw std::runtime_error("stone not found.");

	prop_base_rotation = propeller->rotation;
	plane_base_rotation = plane->rotation;
	plane_curr_rotation = plane_base_rotation;

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	//start music loop playing:
	// (note: position will be over-ridden in update())
	leg_tip_loop = Sound::loop_3D(*dusty_floor_sample, 1.0f, get_prop_position(), 2.0f);
	engine_play = Sound::loop_3D(*engine_sample, 0.0f, get_prop_position(), 2.0f);
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.downs += 1;
			space.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_LEFT) {
			larr.downs += 1;
			larr.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			rarr.downs += 1;
			rarr.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			uarr.downs += 1;
			uarr.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			darr.downs += 1;
			darr.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_LEFT) {
			larr.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			rarr.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			uarr.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			darr.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	wobble += elapsed / 10.0f;
	wobble -= std::floor(wobble);
	countdown -= elapsed;
	if (countdown < 0 && space.pressed) {
		countdown = 15.0f;
		// reset count down, push new ball to move;
		for (auto s : stones) {
			std::cout << "px: " << propeller->position.x << "py: " << propeller->position.y << "pz: " << propeller->position.z << '\n';
			if (s->distance(propeller) > 250) {
				std::cout << "push stone" << '\n';
				// should either move the init stones or reuse some old ones
				s->move(propeller->position);	
				std::cout << "x: " << s->t->position.x << "y: " << s->t->position.y << "z: " << s->t->position.z << '\n';
				break;
			}
		}
	}
	for (auto s : stones) {
		if (space.pressed) {
			s->update(elapsed);
		} else {
			s->words->set_volume(0.0f);
		}
		if (s->distance(propeller) < 10) {
			std::cout << "hit stone" << '\n';
			// well....I am not trying to save the effort here but just...There is nothing you can do when being hit
			// call stone tracebacks
			for (auto s : stones) {
				s->speed = -(s->speed*10.0f);
			}
			break;
		}
	}
	if (space.pressed) {
		brightness = std::max(brightness - elapsed * 0.01f, 0.0f);
		engine_vol = std::min(5.0f, engine_vol+elapsed*10);
		engine_play->set_volume(engine_vol, 2.0f);
		propeller->rotation = prop_base_rotation * glm::angleAxis(
			glm::radians(50000.0f * wobble),
			glm::vec3(0.0f, 0.0f, 1.0f)
		);
	} else if (space.downs > 0) {
		propeller->rotation = prop_base_rotation * glm::angleAxis(
			glm::radians(50000.0f * wobble * (space.downs * 0.0001f)),
			glm::vec3(0.0f, 0.0f, 1.0f)
		);
		space.downs = space.downs / (1.1);
	} else {
		brightness = std::min(brightness + elapsed, 1.0f);
		engine_vol = std::max(0.0f, engine_vol-elapsed*10);
		engine_play->set_volume(engine_vol, 2.0f);
	}
 	
	leg_tip_loop->set_position(get_prop_position(), 1.0f / 60.0f);

	if (larr.pressed) {
		plane->rotation = plane->rotation * glm::angleAxis(
			glm::radians(50.0f * elapsed),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
	}
	if (rarr.pressed) {
		plane->rotation = plane->rotation * glm::angleAxis(
			glm::radians(-50.0f * elapsed),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
	}
	if (uarr.pressed) {
		if (plane->position.z < 3) {
			plane->position.z += 2*elapsed;
		}
	}
	if (darr.pressed) {
		if (plane->position.z > -3) {
			plane->position.z -= 2*elapsed;
		}
	}

	//move camera:
	{
		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 forward = -frame[2];

		camera->transform->position += move.x * right + move.y * forward;
	}

	{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		glm::vec3 at = frame[3];
		Sound::listener.set_position_right(at, right, 1.0f / 60.0f);
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	//consider using the Light(s) in the scene to do this
	//brightness change reference: https://github.com/wrystal/CrazyDriver 20Fall game
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f*brightness, 1.0f*brightness, 0.95f*brightness)));
	glUseProgram(0);

	glClearColor(0.5f*brightness, 0.5f*brightness, 0.5f*brightness, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();
}

glm::vec3 PlayMode::get_prop_position() {
	//the vertex position here was read from the model in blender:
	return propeller->make_local_to_world() * glm::vec4(-1.26137f, -11.861f, 0.0f, 1.0f);
}
