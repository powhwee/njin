namespace njin::runtime {

        template <typename T>
        void njBuffer<T>::insert(const T& data) {
                contents_.push_back(data);

        }
}