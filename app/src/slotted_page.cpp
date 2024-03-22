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

    uint16_t left = 0;
    uint16_t right = num_records;
    // get target key
    char *t_bytes = (char *)key->get_data();
    uint16_t t_size;
    memcpy(&t_size, t_bytes, sizeof(uint16_t));
    std::string t_key(t_bytes + sizeof(uint16_t), t_size);

    while (left < right) {
        uint16_t mid = left + (right - left)/2;
        uint16_t p_loc = (mid * PTR_SIZE) + HDR_SIZE; // calculate actual ptr cell location
        uint16_t i_loc = get_n(p_loc); // get info cell location
        uint16_t i_key_size = get_n(p_loc + 2);
        uint16_t i_val_size = get_n(p_loc + 4);
        // get current key
        char *c_bytes = (char *)address(i_loc);
        uint16_t c_size;
        memcpy(&c_size, c_bytes, sizeof(uint16_t));
        std::string c_key(c_bytes + sizeof(uint16_t), c_size);

        if (strcmp(t_key.c_str(), c_key.c_str()) == 0) { 
            // update the record 
            uint16_t value_loc = i_loc + i_key_size;
            if (value->get_size() > i_val_size) {
                uint16_t extra = value->get_size() - i_val_size;
                slide(value_loc, value_loc - extra);
                memcpy(address(value_loc - extra), value->get_data(), value->get_size());
            } else {
                memcpy(address(value_loc), value->get_data(), value->get_size());
                slide(value_loc + value->get_size(), value_loc + i_val_size);
            }
            return;
        }

        if (t_key < c_key) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    // append new record

    uint16_t i_loc = end_free - key->get_size() - value->get_size();
    set_n(END_FREE_LOC, i_loc - 1);

    // add pointer cell
    // left and right shouldve converged at this point
    uint16_t p_loc = (left * PTR_SIZE) + HDR_SIZE;
    memmove(address(p_loc + PTR_SIZE), address(p_loc), PTR_SIZE); // make space
    set_n(p_loc, i_loc);
    set_n(p_loc + 2, key->get_size());
    set_n(p_loc + 4, value->get_size());

    // add value cell
    memcpy(address(i_loc), key->get_data(), key->get_size());
    memcpy(address(i_loc + key->get_size()), value->get_data(), value->get_size());

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
    uint16_t left = 0;
    uint16_t right = num_records;
    // get target key
    char *t_bytes = (char *)key->get_data();
    uint16_t t_size;
    memcpy(&t_size, t_bytes, sizeof(uint16_t));
    std::string t_key(t_bytes + sizeof(uint16_t), t_size);

    while (left <= right) {
        uint16_t mid = left + (right - left)/2;
        uint16_t p_loc = (mid * PTR_SIZE) + HDR_SIZE; // calculate actual ptr cell location
        uint16_t i_loc = get_n(p_loc); // get info cell location
        uint16_t key_size = get_n(p_loc + 2);
        uint16_t val_size = get_n(p_loc + 4);
        // get current key
        char *c_bytes = (char *)address(i_loc);
        uint16_t c_size;
        memcpy(&c_size, c_bytes, sizeof(uint16_t));
        std::string c_key(c_bytes + sizeof(uint16_t), c_size);

        if (strcmp(t_key.c_str(), c_key.c_str()) == 0) { // can't use == operator
            return new DAS(address(i_loc + key_size), val_size);
        }

        if (t_key < c_key) {
            left = mid + 1;
        } else {
            right = mid - 1;
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
