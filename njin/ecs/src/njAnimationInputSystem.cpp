#include "ecs/njAnimationInputSystem.h"

#include <iostream>

#include <SDL3/SDL_keyboard.h>

#include "ecs/Components.h"

namespace njin::ecs {

    void njAnimationInputSystem::update(const njEntityManager& entity_manager) {
        // Query for Root entities that have bindings and animation state
        auto view = entity_manager.get_views<njAnimationComponent,
                                             njAnimationBindingsComponent>();

        const bool* state = SDL_GetKeyboardState(NULL);

        for (auto [entity_id, components] : view) {
            auto anim_comp = std::get<0>(components);
            auto bindings_comp = std::get<1>(components);

            // Check all bindings
            for (const auto& binding : bindings_comp->key_to_animation) {
                SDL_Scancode scancode = binding.first;
                const std::string& animation_name = binding.second;
                if (state[scancode]) {
                    // Start animation if not already playing or different
                    if (anim_comp->current_animation != animation_name) {
                        anim_comp->current_animation = animation_name;
                        anim_comp->current_time = 0.0f;
                        anim_comp->playing = true;
                        anim_comp->loop = true;
                        std::cout
                        << "Switching to animation: " << animation_name
                        << std::endl;
                    }
                    if (!anim_comp->playing) {
                        anim_comp->playing = true;
                    }
                }
            }
        }
    }

}  // namespace njin::ecs
