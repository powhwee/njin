#pragma once
#include <vector>

#include "core/Renderable.h"

namespace njin::core {
    class Renderable;

    /**
    * Shared buffer between the renderer and any system that wants to write
    * to it. The renderer will read from this buffer when it wants to gather
    * information to draw a frame.
    * At its core, the RenderBuffer oversees an array of @see Renderables
    * Renderables are of a fixed and well-defined format,
    * and the renderer is responsible for reading the array of Renderables
    * in and deciding how to render them.
    * In essence, the RenderBuffer acts as a communication interface to the
    * renderer, to tell it what exactly to render.
    */
    class RenderBuffer {
        public:
        /**
         * Replace the contents of the current buffer
         * @param renderables New set of Renderables
         */
        void replace(const std::vector<Renderable>& renderables);

        /**
         * Set the current view matrix
         * @param view_matrix View matrix
         */
        void set_view_matrix(const math::njMat4f& view_matrix);

        /**
         * Set the current projection matrix
         * @param projection_matrix Projection matrix
         */
        void set_projection_matrix(const math::njMat4f& projection_matrix);

        std::vector<Renderable> get_renderables() const;

        math::njMat4f get_view_matrix() const;

        math::njMat4f get_projection_matrix() const;

        private:
        std::vector<Renderable> renderables_{};
        math::njMat4f view_{};
        math::njMat4f projection_{};
    };
}  // namespace njin::core