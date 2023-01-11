
     COMP 304 FALL 2022 – PROJECT 3

		   	DEADLINE: JANUARY 11, 2023


ATAKAN ÖZKAN 76277 | DEPARTMENT OF COMPUTER ENGINEERING, KOC UNIVERSITY

BUĞRAHAN YAMAN 76070 | DEPARTMENT OF COMPUTER ENGINEERING, KOC UNIVERSITY



1)	PART 1

We implemented the page fault section and implemented the add_to_tlb and search_tlb functions. Every time there is a page fault, we get the page address by multiplying the logical page and page size. After we do that, we get the data from bin file by memcpy function and write it into main memory array then we put it the physical page number to page table related to its logical page. Then we increment memory index with page size which is 1024. After all these operations we call the add_to_tlb function where we do FIFO policy. For the search table we just search the TLB with logical page number then we return the physical page number. If there is no entry in TLB, we return -1.




 


To run the first part, we simply used these commands:

gcc part1.c -o part1.o

./part1.o BACKING_STORE.bin addresses.txt









2)	PART 2

In part2, we used the same implementation where we used in part1. Additionally, we implemented a function which is called replace_page and we used different page replacement policies in that function. There are two-page replacement policies. They are LRU and FIFO. To determine the option, we just get the option number where 0 is FIFO and LRU is 1. We get the option from command line arguments. We also changed the structure of page table array. We changed the page table array to struct pageentry array. The pageentry struct consists of three entry which are page number, frame number and timestamp. We also used additional if statements in page fault section to prevent the errors which are occurred due to decreasing the amount of pages form 1024 to 256.


To run: 
	
	Compile:
	
	gcc part2.c -o part2.o

	For option 0 (FIFO):
	
		./part2.o BACKING_STORE.bin addresses.txt -p 0

	For option 1 (LRU):

		./part2.o BACKING_STORE.bin addresses.txt -p 1
		

 
