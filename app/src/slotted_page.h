#include <vector>
#include <string>
#include "das.h"

const uint16_t BLOCK_SZ = 4096;

enum PageType {
    INT = 0,
    TEXT = 1,
};

class SlottedPage {
    public:
        SlottedPage();
        ~SlottedPage();

        static const int END_FREE_LOC = 4;
        static const int NUM_REC_LOC = 2;
        static const int PTR_SIZE = 6;
        static const int HDR_SIZE = 6;

        void put(const DAS *key, const DAS *value);

        DAS *get(const DAS *key);

        std::vector<DAS*> list(void);

        int size();

    private:

        void slide(uint16_t start, uint16_t end);

        void set_n(uint16_t offset, uint16_t value);
        uint16_t get_n(uint16_t offset);
        void *address(uint16_t offset) const;

        DAS block;

};