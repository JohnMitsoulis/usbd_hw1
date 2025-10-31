#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file_structs.h"
#include "record.h"

#define CALL_BF(call)         \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK)        \
    {                         \
      BF_PrintError(code);    \
      return 0;        \
    }                         \
  }

int HeapFile_Create(const char* fileName)
{

  CALL_BF(BF_CreateFile(fileName));                                   //initialize and open file
  int file_handle;
  CALL_BF(BF_OpenFile(fileName, &file_handle));

  BF_Block* block;                                                    //initialization and creation of first block with metadata
  BF_Block_Init(&block);                                              
  CALL_BF(BF_AllocateBlock(file_handle, block));
  
  
  char* data = BF_Block_GetData(block);                                 //write metadata in the header block
  HeapFileHeader header;
  memcpy(header.file_type, "HP_FILE", strlen("HP_FILE") + 1);           //type of file
  header.recordCount = 0;                                                //number of records in the file                                 
  memcpy(data, &header, sizeof(HeapFileHeader));                        //copy the structure in the file

  BF_Block_SetDirty(block);                                             //dirty and unpin
  CALL_BF(BF_UnpinBlock(block));

  BF_Block_Destroy(&block);
  CALL_BF(BF_CloseFile(file_handle));
  
  return 1;                                                             //CALL_BF returns 0 for us
}

int HeapFile_Open(const char *fileName, int *file_handle, HeapFileHeader** header_info)
{
  CALL_BF(BF_OpenFile(fileName, file_handle)); //open the file

  BF_Block* block;
  BF_Block_Init(&block);
  CALL_BF(BF_GetBlock(*file_handle, 0, block)); //take first block with metadata
  
  char* data = BF_Block_GetData(block);

  HeapFileHeader temp_header;
  memcpy(&temp_header, data, sizeof(HeapFileHeader));

  *header_info = malloc(sizeof(HeapFileHeader));  //allocate memory for header
  if(*header_info == NULL) {return 0;}
  memcpy(*header_info, &temp_header, sizeof(HeapFileHeader)); //copy data to header
  
  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);
  
  return 1;
}

int HeapFile_Close(int file_handle, HeapFileHeader *hp_info)
{
  BF_Block* block;
  BF_Block_Init(&block);
  CALL_BF(BF_GetBlock(file_handle, 0, block)); //take the first block with metadata

  char* data = BF_Block_GetData(block);

  memcpy(data, hp_info, sizeof(HeapFileHeader)); //update header

  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));

  BF_Block_Destroy(&block);
  CALL_BF(BF_CloseFile(file_handle));

  free(hp_info); //free header that was dynamically allocated in open
  return 1;
}


int HeapFile_InsertRecord(int file_handle, HeapFileHeader *hp_info, const Record record)
{
  int block_count;
  CALL_BF(BF_GetBlockCounter(file_handle, &block_count));

  BF_Block* block;
  BF_Block_Init(&block);
  if(block_count == 1){                             //if only header allocate new block else get the last block
    CALL_BF(BF_AllocateBlock(file_handle, block));
  }
  else{
    CALL_BF(BF_GetBlock(file_handle, block_count - 1, block));
  }
  
  char * data = BF_Block_GetData(block);

  int recordsPerBlock = BF_BLOCK_SIZE / sizeof(Record); //records in a block
  int recordsInLastBlock = hp_info->recordCount % recordsPerBlock; //records in last block

  if(recordsInLastBlock == 0){ //if last block full
    CALL_BF(BF_UnpinBlock(block)); //unpin last block
    BF_Block_Destroy(&block);
    
    BF_Block_Init(&block);
    CALL_BF(BF_AllocateBlock(file_handle, block)); //allocate a new one
    data = BF_Block_GetData(block);
  }
  
  memcpy(data + recordsInLastBlock * sizeof(Record), &record, sizeof(Record)); //correctly add the new record

  hp_info->recordCount++;

  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);

  return 1;
}

HeapFileIterator HeapFile_CreateIterator(int file_handle, HeapFileHeader* header_info, int id)
{
  HeapFileIterator out;
  out.file_handle = file_handle;
  out.header = header_info;
  out.currentBlock = 1; //0 is header, so initialize as 1
  out.currentRecord = 0;
  out.recordCount = header_info->recordCount;
  out.searchID = id;
  return out;
}


int HeapFile_GetNextRecord(HeapFileIterator* heap_iterator, Record** record)
{
  int block_count;
  CALL_BF(BF_GetBlockCounter(heap_iterator->file_handle, &block_count));

  int recordsPerBlock = BF_BLOCK_SIZE / sizeof(Record);

  BF_Block* block;
  BF_Block_Init(&block);
  
  while(heap_iterator->currentBlock < block_count){
    CALL_BF(BF_GetBlock(heap_iterator->file_handle, heap_iterator->currentBlock, block));
    char* data = BF_Block_GetData(block);
    
    while(heap_iterator->currentRecord < recordsPerBlock){
      Record* curent_record = (Record*)(data + heap_iterator->currentRecord * sizeof(Record)); //address of current record
      
      if(curent_record->id == heap_iterator->searchID){
        *record = curent_record;
        heap_iterator->currentRecord++; //since we found it, we search from the next record moving forward
        CALL_BF(BF_UnpinBlock(block));
        BF_Block_Destroy(&block);
        return 1;
      }
      heap_iterator->currentRecord++;
    }
    
    CALL_BF(BF_UnpinBlock(block));
    heap_iterator->currentBlock++;
    heap_iterator->currentRecord = 0;
  }
  
  BF_Block_Destroy(&block); 
  *record = NULL;
  return 0;
}

