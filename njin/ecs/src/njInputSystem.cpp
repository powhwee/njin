#include "ecs/njInputSystem.h"

#include <SDL3/SDL.h>

#include "ecs/Components.h"

namespace njin::ecs {
    njInputSystem::njInputSystem(bool& should_run) :
        njSystem{ TickGroup::Zero },
        should_run_{ &should_run }

    {
        // we use sdl's input subsystem
        SDL_InitSubSystem(SDL_INIT_EVENTS);
    }

    void njInputSystem::update(const ecs::njEntityManager& entity_manager) {
        std::vector<View<njInputComponent>> views{
            entity_manager.get_views<njInputComponent>()
        };

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            for (View<njInputComponent>& view : views) {
                const auto input{ std::get<njInputComponent*>(view.second) };

                switch (event.type) {
                case SDL_EVENT_QUIT:
                    *should_run_ = false;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    switch (event.key.key) {
                    case SDLK_W:
                        input->w = true;
                        break;
                    case SDLK_S:
                        input->s = true;
                        break;
                    case SDLK_A:
                        input->a = true;
                        break;
                    case SDLK_D:
                        input->d = true;
                        break;
                    }
                    if (event.key.key == SDLK_ESCAPE) {
                        *should_run_ = false;
                    }
                    break;
                case SDL_EVENT_KEY_UP:
                    switch (event.key.key) {
                    case SDLK_W:
                        input->w = false;
                        break;
                    case SDLK_S:
                        input->s = false;
                        break;
                    case SDLK_A:
                        input->a = false;
                        break;
                    case SDLK_D:
                        input->d = false;
                        break;
                    }
                }
            }
        }
    }
}  // namespace njin::ecs
