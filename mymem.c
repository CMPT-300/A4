#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mymem.h"
#include <time.h>


/* The main structure for implementing memory allocation.
 * You may change this to fit your implementation.
 */

struct memoryList
{
  // doubly-linked list
  struct memoryList *prev;	// i changed this from last to prev cuz its less confusing
  struct memoryList *next;

  int size;            // How many bytes in this block?
  char alloc;          // 1 if this block is allocated,
                       // 0 if this block is free.
  void *ptr;           // location of block in memory pool.
};

strategies myStrategy = NotSet;    // Current strategy


size_t mySize;
void *myMemory = NULL;

static struct memoryList *head;
static struct memoryList *next;


/* initmem must be called prior to mymalloc and myfree.

   initmem may be called more than once in a given exeuction;
   when this occurs, all memory you previously malloc'ed  *must* be freed,
   including any existing bookkeeping data.

   strategy must be one of the following:
		- "best" (best-fit)
		- "worst" (worst-fit)
		- "first" (first-fit)
		- "next" (next-fit)
   sz specifies the number of bytes that will be available, in total, for all mymalloc requests.
*/

void initmem(strategies strategy, size_t sz)
{
	// printf("init\n");
	myStrategy = strategy;

	/* all implementations will need an actual block of memory to use */
	mySize = sz;

	if (myMemory != NULL) free(myMemory); /* in case this is not the first time initmem2 is called */

	/* TODO: release any other memory you were using for bookkeeping when doing a re-initialization! */
	
	struct memoryList *temp;	

	while(head!=NULL){			// frees everything as instructed
		temp = head->next;		
		free(head);				
		head = temp;
	}							
	myMemory = malloc(sz);
	
	/* TODO: Initialize memory management structure. */
	head = malloc(sizeof(struct memoryList));

	head->ptr = myMemory;
	head->size = sz;	  // setting the member vars to appropriate values
	head->alloc = 0;
	head->prev = NULL;
	head->next = NULL; 
						// may need to add stuff for circular LL (nextfit)
}

/* Allocate a block of memory with the requested size.
 *  If the requested block is not available, mymalloc returns NULL.
 *  Otherwise, it returns a pointer to the newly allocated block.
 *  Restriction: requested >= 1 
 */

void *mymalloc(size_t requested)
{
	// printf("malloc\n");
	assert((int)myStrategy > 0);

	size_t req = requested;
	if (head==NULL)
		return NULL;
	
	
	int best_size;
	struct memoryList *trav, *temp, *bestmode; // we'll need these later prob
	

	switch (myStrategy)
	{
	  	case NotSet:
	        return NULL;

	  	case First:
			trav = head;
			// iterate until we find the first valid block
			while (trav!=NULL){
				if (trav->size>=req && trav->alloc==0)
					break; // trav now holds the block to be used
				trav = trav->next;
			}

			if (trav==NULL) // no block found
		        return NULL;
		    break;

	  	case Best:
	        return NULL;

	  	case Worst:
	        return NULL;

	  	case Next:
	        return NULL;
    } // the below code is designed for firstfit.
      // nextfit's circular behaviour probably won't comply with this,
      // so adjustments/additions might have to be made.
      // don't break it for the other strategies though.
    if(trav!=NULL){
	    if (trav->size==req) // perfect fit
	    	trav->alloc = 1; // allocate the blocks

	    else{ // block > requested size. split.
	    	int excess = trav->size - requested; // holds the unused space in the block
    		trav->size = requested; // shrink size to match requested size
    		trav->alloc = 1;  // mark as allocated

    		void* nextptr = trav->ptr + requested; // base address of next block
    		temp = malloc(sizeof(struct memoryList)); // create new block
    		temp->size = excess; // update size
    		temp->ptr = nextptr; // new block now contains correct base address
    		temp->alloc = 0;	// unallocated

    		if(trav->next!=NULL){
	    		temp->next = trav->next;
	    		trav->next->prev = temp;
	    	}
	    	else
	    		temp->next = NULL;

    		trav->next = temp;
    		temp->prev = trav;
    	}
	}
		// for debugging
	// struct memoryList *test = head;
	// int i = 1;
	// // printf("malloc\n");
	// while(test!=NULL){
	// 	printf("block: %d\n", i);
	// 	printf("alloc: %d\n", test->alloc);
	// 	printf("size: %d\n", test->size);
	// 	printf("ptr: %p\n", test->ptr);
	// 	test = test->next;
	// 	i++;
	// }
	return trav->ptr;
    
}


/* Frees a block of memory previously allocated by mymalloc. */
void myfree(void* block)
{	
	// printf("free\n");
	struct memoryList *trav = head;

	while (trav!=NULL){ // iterate until block address is found
		if (trav->ptr == block)
			break;
		trav = trav->next;
	}
	int adj_full_blocks = 0; // counts number of adjacent full blocks
	if (trav->prev!=NULL){
		if (trav->prev->alloc==0){ // merge with previous block, eliminating prev
			// printf("prev merge\n");
			int newsize = trav->size + trav->prev->size; // combined size
			void* newaddress = trav->prev->ptr;	// need new ptr since base address changing
			if (trav->prev->prev!=NULL){
				trav->prev->prev->next = trav; // update pointers to eliminate trav->prev
				trav->prev = trav->prev->prev;
			}
			else{ // remove head
				head = head->next;
				head->prev = NULL;
			}
			trav->alloc = 0;  		// update member vars 
			trav->ptr = newaddress;
		 	trav->size = newsize;
		}
		else{ // prev block allocated, increment adj
			// printf("prev allocated\n");
			adj_full_blocks++;
		}
	}
	else{ // prev block NULL (no free neighbour), increment adj
		// printf("prev allocated\n");
		adj_full_blocks++;
	}  

	// handle next block (this code is similar to previous if)
	
	if (trav->next!=NULL){
		if (trav->next->alloc==0){ // merge with next block, eliminating next
			// printf("next  merge\n");
			int newsize = trav->size + trav->next->size;
			// dont need new ptr since base address is same
			if (trav->next->next!=NULL){
				trav->next->next->prev = trav;
				trav->next = trav->next->next;
			}
			else{ // remove last elem
				trav->next = NULL;
			}

			trav->alloc = 0;  
		 	trav->size = newsize;
		}
		else{
			// printf("next allocated\n");
			adj_full_blocks++;
		}
	}
	else{
		// printf("next allocated\n");
		adj_full_blocks++;
	}

	// printf("adj_free_blocks: %d\n", adj_full_blocks);
	if (adj_full_blocks==2){
		// printf("freeing\n");
		trav->alloc = 0;
	}
		// for debugging
	// struct memoryList *test = head;
	// int i = 1;
	// while(test!=NULL){
	// 	printf("block: %d\n", i);
	// 	printf("alloc: %d\n", test->alloc);
	// 	printf("size: %d\n", test->size);
	// 	printf("ptr: %p\n", test->ptr);
	// 	test = test->next;
	// 	i++;
	// }
	return;
}

/****** Memory status/property functions ******
 * Implement these functions.
 * Note that when we refer to "memory" here, we mean the 
 * memory pool this module manages via initmem/mymalloc/myfree. 
 */

/* Get the number of contiguous areas of free space in memory. */
int mem_holes()
{
	struct memoryList* trav = head;
	int holes = 0;
	while (trav!=NULL){
		if (trav->alloc==0) // hole has been found
			holes++;
		trav = trav->next;
	}
	return holes;
}

/* Get the number of bytes allocated */ // thess parts are pretty simple. 
int mem_allocated()
{
	struct memoryList* trav = head;
	int bytes = 0;
	while (trav!=NULL){
		if (trav->alloc==1) // allocated block, retrieve size
			bytes+= trav->size;
		trav = trav->next;
	}
	return bytes;
}

/* Number of non-allocated bytes */
int mem_free()
{
	struct memoryList* trav = head;
	int bytes = 0;
	while (trav!=NULL){
		if (trav->alloc==0) // unallocated block, retrieve size
			bytes+= trav->size;
		trav = trav->next;
	}
	return bytes;
}

/* Number of bytes in the largest contiguous area of unallocated memory */
int mem_largest_free()
{
	struct memoryList* trav = head;
	int max = 0;
	while (trav!=NULL){
		if (trav->size>max && trav->alloc == 0) // check if its bigger than prev max
			max = trav->size; 				   // and if unallocated
		trav = trav->next;
	}
	return max;
}

/* Number of free blocks smaller than "size" bytes. */
int mem_small_free(int size)
{
	struct memoryList* trav = head;
	int free = 0;
	while (trav!=NULL){
		if (trav->size<=size && trav->alloc==0) // this logically should be <, but it errors, so i made it <=
			free++; 
		trav = trav->next;
	}
	return free;
}

char mem_is_alloc(void *ptr)
{
	struct memoryList* trav = head;
	while (trav!=NULL){
		if (trav->ptr==ptr){
			if(trav->alloc==0)
				return 0;
			else
				return 1;
		}
		trav = trav->next;
	}
	return -1;
}

/* 
 * Feel free to use these functions, but do not modify them.  
 * The test code uses them, but you may ind them useful.
 */


//Returns a pointer to the memory pool.
void *mem_pool()
{
	return myMemory;
}

// Returns the total number of bytes in the memory pool. */
int mem_total()
{
	return mySize;
}


// Get string name for a strategy. 
char *strategy_name(strategies strategy)
{
	switch (strategy)
	{
		case Best:
			return "best";
		case Worst:
			return "worst";
		case First:
			return "first";
		case Next:
			return "next";
		default:
			return "unknown";
	}
}

// Get strategy from name.
strategies strategyFromString(char * strategy)
{
	if (!strcmp(strategy,"best"))
	{
		return Best;
	}
	else if (!strcmp(strategy,"worst"))
	{
		return Worst;
	}
	else if (!strcmp(strategy,"first"))
	{
		return First;
	}
	else if (!strcmp(strategy,"next"))
	{
		return Next;
	}
	else
	{
		return 0;
	}
}


/* 
 * These functions are for you to modify however you see fit.  These will not
 * be used in tests, but you may find them useful for debugging.
 */

/* Use this function to print out the current contents of memory. */
void print_memory()
{
		// for debugging
	// struct memoryList *test = head;
	// int i = 1;
	// while(test!=NULL){
	// 	printf("block: %d\n", i);
	// 	printf("alloc: %d\n", test->alloc);
	// 	printf("size: %d\n", test->size);
	// 	printf("ptr: %p\n", test->ptr);
	// 	test = test->next;
	// 	i++;
	// }
}

/* Use this function to track memory allocation performance.  
 * This function does not depend on your implementation, 
 * but on the functions you wrote above.
 */ 
void print_memory_status()
{
	printf("%d out of %d bytes allocated.\n",mem_allocated(),mem_total());
	printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n",mem_free(),mem_holes(),mem_largest_free());
	printf("Average hole size is %f.\n\n",((float)mem_free())/mem_holes());
}

/* Use this function to see what happens when your malloc and free
 * implementations are called.  Run "mem -try <args>" to call this function.
 * We have given you a simple example to start.
 */
void try_mymem(int argc, char **argv) {
        strategies strat;
	void *a, *b, *c, *d, *e;
	if(argc > 1)
	  strat = strategyFromString(argv[1]);
	else
	  strat = First;
	
	
	/* A simple example.  
	   Each algorithm should produce a different layout. */
	
	initmem(strat,4);
	
	a = mymalloc(1);
	b = mymalloc(1);
	c = mymalloc(1);
	d = mymalloc(1);
	myfree(b);
	
	print_memory();
	print_memory_status();
	
}
