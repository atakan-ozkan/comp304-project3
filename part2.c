/**
 * virtmem.c
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include<stdbool.h>
#define TLB_SIZE 16
#define PAGES 256
#define PAGE_MASK 0x0FF

#define PAGE_SIZE 1024
#define OFFSET_BITS 10
#define OFFSET_MASK 1023

#define MEMORY_SIZE PAGES * PAGE_SIZE

// Max number of characters per line of input file to read.
#define BUFFER_SIZE 10

struct tlbentry {
  unsigned char logical;
  unsigned char physical;
};

struct pageentry{
    unsigned int page_number;
    unsigned int frame_number;
    unsigned int timestamp;
};

// TLB is kept track of as a circular array, with the oldest element being overwritten once the TLB is full.
struct tlbentry tlb[TLB_SIZE];
// number of inserts into TLB that have been completed. Use as tlbindex % TLB_SIZE for the index of the next TLB line to use.
int tlbindex = 0;
int option = 0;
int pagefilled = 0;
// pagetable[logical_page] is the physical page number for logical page. Value is -1 if that logical page isn't yet in the table.
struct pageentry pagetable[PAGES];

signed char main_memory[MEMORY_SIZE];

// Pointer to memory mapped backing file
signed char *backing;

int max(int a, int b)
{
  if (a > b)
    return a;
  return b;
}

void replace_page(unsigned int page_number,unsigned int frame_number){
        if(option == 1){ //LRU
            int pos = 0;
            int leastUsed = pagetable[0].timestamp;
            int i;
            
            for(i = 0;i < PAGE_SIZE; i++){
                leastUsed = max(leastUsed,pagetable[i].timestamp);
                if(pagefilled< PAGE_SIZE){
                    pos = pagefilled;
                    pagefilled = (pagefilled+1);
                    break;
                }
                if(pagetable[i].timestamp  == leastUsed){
                    leastUsed = pagetable[i].timestamp;
                    pos=i;
                }
            }
            
            for(i=0;i < PAGE_SIZE; i++){
                if(i == pos){
                    pagetable[pos].page_number = page_number;
                    pagetable[pos].frame_number = frame_number;
                    pagetable[pos].timestamp = 0;
                }
                else{
                    if(pagetable[i].frame_number != -1){
                        pagetable[i].timestamp++;
                    }
                }
            }
        }
        else{
                //FIFO
                pagetable[pagefilled].page_number = page_number;
                pagetable[pagefilled].frame_number = frame_number;
                pagefilled = (pagefilled+1) % PAGE_SIZE;
            
        }
}

int get_from_pagetable(unsigned int logical_page){
    
    for(int i=0;i < PAGES; i++){
        if(pagetable[i].page_number == logical_page){
            return pagetable[i].frame_number;
        }
    }
    return -1;
}




/* Returns the physical address from TLB or -1 if not present. */
int search_tlb(unsigned char logical_page) {
    
    for(int i=0; i < TLB_SIZE; i++){
        if(tlb[i].logical == logical_page){
            return tlb[i].physical; // There is a TLB HIT
        }
    }
    return -1; // there is a TLB MISS
}

/* Adds the specified mapping to the TLB, replacing the oldest mapping (FIFO replacement). */
void add_to_tlb(unsigned char logical, unsigned char physical) {
        tlb[tlbindex].logical = logical;
        tlb[tlbindex].physical = physical;
        tlbindex = (tlbindex+1)%TLB_SIZE;
    return;
}

int main(int argc, const char *argv[])
{
  if (argc !=3 && argc !=5) {
    fprintf(stderr, "Usage ./virtmem backingstore input or ./virtmem backingstore input -p (0 or 1)\n");
    exit(1);
  }
    char flag[]= "-p";
    if(argc > 4 && strcmp(argv[3],flag) == 0){
        option = atoi(argv[4]);
    }
    else{
        option = 0;
        
    }
    
  const char *backing_filename = argv[1];
  int backing_fd = open(backing_filename, O_RDONLY);
  backing = mmap(0, MEMORY_SIZE, PROT_READ, MAP_PRIVATE, backing_fd, 0);
  
  const char *input_filename = argv[2];
  FILE *input_fp = fopen(input_filename, "r");
  
  // Fill page table entries with -1 for initially empty table.
  int i;
  for (i = 0; i < PAGES; i++) {
    pagetable[i].page_number = -1;
    pagetable[i].frame_number = -1;
  }
  
  // Character buffer for reading lines of input file.
  char buffer[BUFFER_SIZE];
  
  // Data we need to keep track of to compute stats at end.
  int total_addresses = 0;
  int tlb_hits = 0;
  int page_faults = 0;
  int memory_index = 0;
  
  // Number of the next unallocated physical page in main memory
  unsigned char free_page = 0;
  
  while (fgets(buffer, BUFFER_SIZE, input_fp) != NULL) {
    total_addresses++;
    int logical_address = atoi(buffer);

    /* TODO
    / Calculate the page offset and logical page number from logical_address */
    int offset = logical_address & OFFSET_MASK;
    int logical_page = (logical_address >> OFFSET_BITS) ;
    int physical_page = search_tlb(logical_page);
    // TLB hit
    if (physical_page != -1) {
      tlb_hits++;
      // TLB miss
    } else {

        
        physical_page = get_from_pagetable(logical_page);
      // Page fault
      if (physical_page == -1) {
          /* TODO */
          char tempbuffer[BUFFER_SIZE];
          int pnum;
          int page_address = logical_page * PAGE_SIZE;
          if(PAGES == 1024 || ((pagefilled < (PAGES/4-1) && PAGES == 256))){
              
              memcpy(main_memory + memory_index, backing + page_address,PAGE_SIZE);
          }
 
          physical_page= (memory_index + offset) >> 10 ;
          
          replace_page(logical_page,physical_page);

          
          if(memory_index < MEMORY_SIZE - PAGE_SIZE){
              memory_index+= PAGE_SIZE;
          }
          page_faults++;
      }
            
      add_to_tlb(logical_page, physical_page);
    }
      
    int physical_address = (physical_page << OFFSET_BITS) | offset;
    signed char value = main_memory[physical_address];
    printf("Virtual address: %d Physical address: %d Value: %d\n",logical_address, physical_address, value);
  }
  
  printf("Number of Translated Addresses = %d\n", total_addresses);
  printf("Page Faults = %d\n", page_faults);
  printf("Page Fault Rate = %.3f\n", page_faults / (1. * total_addresses));
  printf("TLB Hits = %d\n", tlb_hits);
  printf("TLB Hit Rate = %.3f\n", tlb_hits / (1. * total_addresses));
  
  return 0;
}
