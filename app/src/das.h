#include <stdint.h>

class DAS {
    public:
        DAS();
        DAS(void*, uint32_t);
        DAS(const DAS&);
	    DAS &operator = (const DAS&);

        void* get_data() const;
        void set_data(void*);

        uint32_t get_size() const;
        void set_size(uint32_t);
    protected:
        void *_data;
        uint32_t _size;
};