#include "input/EventHandler.h"

#include <iostream>

namespace njin::input {
    void EventHandler::handle(const SDL_Event& event) {
        if (event.type == SDL_EVENT_KEY_DOWN) {
            std::cout << "here" << std::endl;
        }
    }
}  // namespace njin::input