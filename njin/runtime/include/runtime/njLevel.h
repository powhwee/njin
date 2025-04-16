#pragma once
#include <memory>

#include "runtime/njObject.h"

namespace njin::runtime {
    class njLevel : public njObject {
        public:
        njLevel(njBuffer<int>& inputs);

        void tick() override;

        private:
    };

}  // namespace njin::runtime