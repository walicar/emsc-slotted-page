#include "das.h"

DAS::DAS() {
    _data = nullptr;
    _size = 0;
}

DAS::DAS(void* data, uint32_t size) {
    _data = data;
    _size = size;
}

DAS::DAS(const DAS &other) {
    _data = other.get_data();
    _size = other.get_size();
}

DAS &DAS::operator=(const DAS &other) {
    if (this != &other) {
        _data = other.get_data();
        _size = other.get_size();
    }
    return *this;
}

void* DAS::get_data() const {
    return _data;
}

void DAS::set_data(void* data) {
    _data = data;
}

uint32_t DAS::get_size() const {
    return _size;
}

void DAS::set_size(uint32_t size) {
    _size = size;
}