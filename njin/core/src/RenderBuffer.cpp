#include "core/RenderBuffer.h"

namespace njin::core {
    void RenderBuffer::replace(const std::vector<Renderable>& renderables) {
        renderables_ = renderables;
    }

    void RenderBuffer::set_view_matrix(const math::njMat4f& view_matrix) {
        view_ = view_matrix;
    }

    void RenderBuffer::set_projection_matrix(const math::njMat4f&
                                             projection_matrix) {
        projection_ = projection_matrix;
    }

    std::vector<Renderable> RenderBuffer::get_renderables() const {
        return renderables_;
    }

    math::njMat4f RenderBuffer::get_view_matrix() const {
        return view_;
    }

    math::njMat4f RenderBuffer::get_projection_matrix() const {
        return projection_;
    }
}  // namespace njin::core