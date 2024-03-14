#include <stdint.h>

class DAS {
    public:
        DAS();
        DAS(void*, uint16_t);
        DAS(const DAS&);
	    DAS &operator = (const DAS&);

        void* get_data() const;
        void set_data(void*);

        uint16_t get_size() const;
        void set_size(uint16_t);
    protected:
        void *_data;
        uint16_t _size;
};