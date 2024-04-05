# Desgin

## DAS
- short for "data and size"
- able to store any type of data
- used for storing and retrieving data from slotted page

## Slotted Page
- 4096 block size
- key-value pairs are put in order
- key retrieval takes `O(log(n))` time
    - all keys are sorted in lexicographic order
- deleting a key does not remove its associated value
    - the key-value pair is only marked for deletion
- header record: (uint16_t: magic, uint16_t: num_records, uint16_t: end_free) 6 bytes
    - end_free always points to the leftmost empty space
- cell pointer record: (uint16_t: location, uint16_t: key_size, uint16_t: value_size) 6 bytes
- cell info record: (void*: key_data, void*: value_data)
