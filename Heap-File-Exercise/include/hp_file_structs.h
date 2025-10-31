#ifndef HP_FILE_STRUCTS_H
#define HP_FILE_STRUCTS_H

#include <record.h>

/**
 * @file hp_file_structs.h
 * @brief Data structures for heap file management
 */

/* -------------------------------------------------------------------------- */
/*                              Data Structures                               */
/* -------------------------------------------------------------------------- */

/**
 * @brief Heap file header containing metadata about the file organization
 */
typedef struct HeapFileHeader {
    char file_type[8];                  //type of file
    int recordCount;                    //total number of records in the heapfile
} HeapFileHeader;

/**
 * @brief Iterator for scanning through records in a heap file
 */
typedef struct HeapFileIterator{
    int file_handle;
    HeapFileHeader* header;
    int currentBlock;   
    int currentRecord;
    int recordCount;    //total number of records in the heapfile
    int searchID;
} HeapFileIterator;

#endif /* HP_FILE_STRUCTS_H */

