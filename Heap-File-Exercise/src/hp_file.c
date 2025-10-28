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
  //o diaxiristis exei faei init stin hp_main

  CALL_BF(BF_CreateFile(fileName));                                   //arxikopoiisi kai anigma arxiou
  int file_handle;
  CALL_BF(BF_OpenFile(fileName, &file_handle));

  BF_Block* block;                                                    //arxikopoiisi kai desmeusi protou block (gia ta metadata tou arxiou)
  BF_Block_Init(&block);                                              //den epistrefei timi ara den mpainei se CALL_BF
  CALL_BF(BF_AllocateBlock(file_handle, block));
  
  //grafoume metadata stin epikefalida
  char* data = BF_Block_GetData(block);
  HeapFileHeader header;
  memcpy(header.file_type, "HP_FILE", strlen("HP_FILE") + 1);           //ti tipos arxiou einai | memcpy(*to, *from, numBytes); 8a mporousame na xrisimopoiisoume kai strcpy edo
  header.recordCount = 0;                                                //ari8mos ton eggrafon sto arxio
  //vazoume meta ama 8eloume kai alles plirofories                                     
  memcpy(data, &header, sizeof(HeapFileHeader));                        //adigrafoume oli ti domi mesa sto block

  BF_Block_SetDirty(block);                                             //to kanoume dirty kai unpin
  CALL_BF(BF_UnpinBlock(block));

  BF_Block_Destroy(&block);
  CALL_BF(BF_CloseFile(file_handle));
  
  return 1;                                                             //ean paei kati la8os i CALL_bf 8a epistrepsei 0
}

int HeapFile_Open(const char *fileName, int *file_handle, HeapFileHeader** header_info)
{
  CALL_BF(BF_OpenFile(fileName, file_handle)); //anigoume to arxio

  BF_Block* block;
  BF_Block_Init(&block);
  CALL_BF(BF_GetBlock(*file_handle, 0, block)); //pairnoume to proto block (ekei einai i kefalida)
  
  char* data = BF_Block_GetData(block);

  HeapFileHeader temp_header;
  memcpy(&temp_header, data, sizeof(HeapFileHeader));

  *header_info = malloc(sizeof(HeapFileHeader));
  if(*header_info == NULL) {return 0;}
  memcpy(*header_info, &temp_header, sizeof(HeapFileHeader));
  
  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);
  
  return 1;
}

int HeapFile_Close(int file_handle, HeapFileHeader *hp_info)
{
  BF_Block* block;
  BF_Block_Init(&block);
  int blockNum;
  CALL_BF(BF_GetBlockCounter(file_handle, &blockNum))
  int i = (blockNum - 1);
  for(i; i >= 0; i--) {  //gia kathe block tou Heap File
    CALL_BF(BF_GetBlock(file_handle, i, block));    //pare to block kai sbhsto
    BF_Block_Destroy(&block);
  }
  free(hp_info);  //free to header???
  return 1;
}

int HeapFile_InsertRecord(int file_handle, HeapFileHeader *hp_info, const Record record)
{
  BF_Block* block;
  BF_Block_Init(&block);
  int blockNum;
  CALL_BF(BF_GetBlockCounter(file_handle, &blockNum));   //pare arithmo blocks
  int BlockRecCount = BF_BLOCK_SIZE / sizeof(Record);   //posa records xorane ana block (edo 8)
  int lastBlockRec = hp_info->recordCount % BlockRecCount;  //posa records xorane sto teleutaio block
  int has_space = lastBlockRec < BlockRecCount ? 1 : 0;   //an teleutaio exei ligotera apo 8 records exei xoro
  if (has_space){
    CALL_BF(BF_GetBlock(file_handle, (blockNum - 1), block));  //pare teleutaio block kai bale to record
    char* data = BF_Block_GetData(block);
    memcpy(data + lastBlockRec*sizeof(Record), &record, sizeof(Record));
    BF_Block_SetDirty(block);
  } else {
    CALL_BF(BF_AllocateBlock(file_handle, block));  //allocate kainourgio block
    char* data = BF_Block_GetData(block); //pare data kai bale to record
    memcpy(data, &record, sizeof(Record));
    BF_Block_SetDirty(block);
  }
  BF_Block_Destroy(&block);
  return 1;
}


HeapFileIterator HeapFile_CreateIterator(    int file_handle, HeapFileHeader* header_info, int id)
{
  HeapFileIterator out;
  return out;
}


int HeapFile_GetNextRecord(    HeapFileIterator* heap_iterator, Record** record)
{
    * record=NULL;
    return 1;
}

