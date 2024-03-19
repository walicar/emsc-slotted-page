#include "slotted_page.h"

SlottedPage::SlottedPage() {
    // init memory
    char* data = new char[BLOCK_SZ]; // don't statically allocate this
    memset(data, 0, BLOCK_SZ);
    block.set_data(data);
    block.set_size(BLOCK_SZ);

    // set end_free
    set_n(END_FREE_LOC, BLOCK_SZ - 1);
}

SlottedPage::~SlottedPage() {
    char *data = (char*) block.get_data();
    delete[] data;
}

int SlottedPage::size() {
    return get_n(NUM_REC_LOC);
}

void SlottedPage::put(const DAS *key, const DAS *value) {
    // have to add a check if we can actually add a record
    uint16_t end_free = get_n(END_FREE_LOC);
    uint16_t num_records = get_n(NUM_REC_LOC);

    uint16_t space_left = end_free - (HDR_SIZE + (PTR_SIZE * (num_records))); // FIXME: not a thorough check
    if (key->get_size() + value->get_size() > space_left) {
        return;
    }

    uint16_t left = HDR_SIZE;
    uint16_t right = left + ((num_records - 1) * PTR_SIZE);
    std::string t_key((char*) key->get_data(), key->get_size());

    while (left <= right) {
        uint16_t mid = left + (right - left)/2;
        uint16_t c_loc = get_n(mid);
        uint16_t c_key_size = get_n(mid + 2);
        uint16_t c_value_size = get_n(mid + 4);
        std::string c_key((char*) address(c_loc), c_key_size);

        if (c_key == t_key) { 
            // update the record 
            uint16_t value_loc = c_loc + c_key_size;
            if (value->get_size() > c_value_size) {
                uint16_t extra = value->get_size() - c_value_size;
                slide(value_loc, value_loc - extra);
                memcpy(address(value_loc - extra), value->get_data(), value->get_size());
            } else {
                memcpy(address(value_loc), value->get_data(), value->get_size());
                slide(value_loc + value->get_size(), value_loc + c_value_size);
            }
            return;
        }

        if (c_key > t_key) {
            left += PTR_SIZE;
        } else {
            right -= PTR_SIZE;
        }
    }
    // append new record

    uint16_t loc = end_free - key->get_size() - value->get_size();
    set_n(END_FREE_LOC, loc - 1);

    // add pointer cell
    // left and right shouldve converged at this point
    memmove(address(left), address(left + PTR_SIZE), PTR_SIZE); // make space
    set_n(left, loc);
    set_n(left + 2, key->get_size());
    set_n(left + 4, value->get_size());

    // add value cell
    memcpy(address(loc), key->get_data(), key->get_size());
    memcpy(address(loc + key->get_size()), value->get_data(), value->get_size());

    // update header
    set_n(NUM_REC_LOC, num_records + 1);
    return;
}

void SlottedPage::slide(uint16_t start, uint16_t end) {
    int shift = end - start;
    if (shift == 0) return;

    // slide data
    uint16_t end_free = get_n(END_FREE_LOC);
    void *to = address(end_free + shift + 1);
    void *from = address(end_free + 1);
    int bytes = start - (end_free + 1);
    memmove(to, from, bytes);
    
    // adjust pointer cells
    uint16_t num_records = get_n(NUM_REC_LOC);
    uint16_t amt = ((num_records - 1) * PTR_SIZE);
    for (int i = 0; i < amt; i++) { // O(n)
        uint16_t ptr_offset = HDR_SIZE + (i * PTR_SIZE);
        uint16_t c_loc = get_n(ptr_offset);
        if (c_loc <= start) {
            c_loc += shift;
            set_n(ptr_offset, c_loc);
        }
    }
    end_free += shift;
    set_n(END_FREE_LOC, end_free);
}

DAS* SlottedPage::get(const DAS *key) {
    uint16_t num_records = get_n(NUM_REC_LOC);
    // locations of leftmost pointer entry, and rightmost pointer entry
    uint16_t left = HDR_SIZE;
    uint16_t right = left + ((num_records - 1) * PTR_SIZE);
    std::string t_key((char*) key->get_data() + sizeof(uint16_t), key->get_size());

    while (left <= right) {
        uint16_t mid = left + (right - left)/2;
        uint16_t c_loc = get_n(mid);
        uint16_t key_size = get_n(mid + 2);
        uint16_t value_size = get_n(mid + 4);
        std::string c_key((char*) address(c_loc) + sizeof(uint16_t), key_size);

        if (strcmp(t_key.c_str(), c_key.c_str())) { // can't use == operator
            return new DAS(address(c_loc + key_size), value_size);
        }

        if (c_key > t_key) {
            left += PTR_SIZE;
        } else {
            right -= PTR_SIZE;
        }
    }
    return new DAS(nullptr, 0); // could not find key-value pair
}

std::vector<DAS*> SlottedPage::list() { // list keys
    std::vector<DAS*> res;
    uint16_t num_records = get_n(NUM_REC_LOC);
    uint16_t amt = ((num_records - 1) * PTR_SIZE);
    for (int i = 0; i < amt; i++) {
        uint16_t ptr_offset = HDR_SIZE + (i * PTR_SIZE);
        uint16_t c_loc = get_n(ptr_offset);
        uint16_t key_size = get_n(ptr_offset + 2);
        res.push_back(new DAS(address(c_loc), key_size));
    }
    return res;
}

void SlottedPage::set_n(uint16_t offset, uint16_t value) {
    *(uint16_t *) address(offset) = value;
}

uint16_t SlottedPage::get_n(uint16_t offset) {
    return *(uint16_t*) address(offset);
}

void *SlottedPage::address(uint16_t offset) const {
  return (void *) ((char *)block.get_data() + offset);
}
