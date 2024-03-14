# SlottedPage

## DAS
- short for "data and size"
- able to store any type of data
- used for storing and retrieving data from slotted page

## Slotted Page
- 4096 block size
- key-value pairs are put in order
- key retrieval takes `O(log(n))` time
- deleting a key does not remove the its associated value
    - the key-value pair is only marked for deletion
