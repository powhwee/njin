#pragma once
#include <vector>

namespace njin::runtime {
    template<typename T>
    class njBuffer {
        public:
        void insert(const T& data);

        private:
        std::vector<T> contents_;
    };
}  // namespace njin::runtime

#include "njBuffer.tpp"