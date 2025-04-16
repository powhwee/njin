#pragma once
#include "ecs/njSystem.h"
#include "math/njMat4.h"

namespace njin::ecs {
    /**
     * Processes input components and turns them into actual changes in
     * local transform
     */
    class njMovementSystem final : public njSystem {
        public:
        njMovementSystem();
        void update(const ecs::njEntityManager& entity_manager) override;

        private:
        /**
         * Calculate a new matrix representing a given translation
         * @param old Old local transform matrix
         * @param translation Local space translation to apply
         * @return Updated transform matrix
         */
        math::njMat4f move(const math::njMat4f& old,
                           const math::njVec3f& translation) const;
    };
}  // namespace njin::ecs