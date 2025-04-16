#pragma once
#include <SDL3/SDL_events.h>

namespace njin::input {
    class EventHandler {
        public:
        void handle(const SDL_Event& event);
    };

}  // namespace njin::input