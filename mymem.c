#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mymem.h"
#include <time.h>


/*
GROUP: 4 MEMBERs
=========
ID, computingID, Name
301 335 629, rsg23, Rajan Grewal
301 333 284, bhow, Brian How
301 334 771, jta118, Johnson Tan
301 333 239, ela57, Eric Leung


The main structure for implementing memory allocation.
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

	struct memoryList* trav;
	if(head != NULL){
		trav = head->next;
		while(trav!=head){
			trav = trav->next;
			free(trav->prev);
		}
		free(head);
	}
	if (myMemory != NULL) free(myMemory); /* in case this is not the first time initmem is called */

	/* TODO: release any other memory you were using for bookkeeping when doing a re-initialization! */
	
	myMemory = malloc(sz);
	
	/* TODO: Initialize memory management structure. */
	head = malloc(sizeof(struct memoryList));

	head->ptr = myMemory;
	head->size = sz;	  // setting the member vars to appropriate values
	head->alloc = 0;
	head->prev = head;
	head->next = head; 

						// may need to add stuff for circular LL (nextfit)
	next = head;
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
	
	
	struct memoryList *trav, *temp; // we'll need these later prob
	size_t m;

	switch (myStrategy)
	{
	  	case NotSet:
	        return NULL;

	  	case First:
			trav = head;
			// iterate until we find the first valid block
			do{
				if (trav->size>=req && trav->alloc==0)
					break; // trav now holds the block to be used
				trav = trav->next;
			}while (trav!=head);

			if (trav==head){
				if (trav->alloc==1 || trav->size<req) // no block found
		        return NULL;
		    }
		    break;

	  	case Best:
	     	temp = NULL;
	     	m = mySize+1; //set min to the size of the allocated memory + 1 
	        trav = head;
			// iterate until we reach the end 
			do{
				if (trav->size>=req && trav->alloc==0)
					if(trav->size < m){//if a suitable block smaller than min is found set it as min 
						m = trav->size;
						temp = trav;
					}
				trav = trav->next;
			}while (trav!=head);
			
			if (temp==NULL) // no block found
		        return NULL;
		    trav = temp; //Set trav=temp for the rest of the function
		    break;

	  	case Worst:
	        temp = NULL;
	     	m = 0; //set max to 0
	        trav = head;
			// iterate until we reach the end 
			do{
				if (trav->size>=req && trav->alloc==0)
					if(trav->size > m){//if a suitable block larger than max is found set it as max 
						m = trav->size;
						temp = trav;
					}
				trav = trav->next;
			}while (trav!=head);
			
			if (temp==NULL) // no block found
		        return NULL;
		    trav = temp; //Set trav=temp for the rest of the function
		    break;

	  	case Next:
	  		trav = next;	//starting from next position
	  		do{
	  			if(trav->size >= req && trav -> alloc == 0)	// if it fits, end
	  				break;
	  			else
	  				trav = trav->next;
	  		}while (trav != next);

			if (trav==next){
				if (trav->alloc==1 || trav->size<req) // no block found
		        return NULL;
		    }
		    break;
    } 

	trav->alloc = 1; // allocate the blocks
	if (trav->size==req){ // perfect fit
    	next = trav->next;
    }
	else{ // block > requested size. split.
    	int excess = trav->size - requested; // holds the unused space in the block
		trav->size = requested; // shrink size to match requested size

		void* nextptr = trav->ptr + requested; // base address of next block
		struct memoryList *new = malloc(sizeof(struct memoryList)); // create new block

		new->size = excess; // update size
		new->ptr = nextptr; // new block now contains correct base address
		new->alloc = 0;	// unallocated

	    // conncting new to the block after trav
	    new->next = trav->next;
	    trav->next->prev = new;
	    // connecting new to trav
	    trav->next = new;
	    new->prev = trav;

		next = new; //updating next to point at the new free block

	}
	return trav->ptr;
}


/* Frees a block of memory previously allocated by mymalloc. */
void myfree(void* block)
{	
	// printf("free\n");
	struct memoryList *trav = head;

	do{ // iterate until block address is found
		if (trav->ptr == block)
			break;
		trav = trav->next;
	}while (trav!=head);

	trav->alloc = 0;
	if (trav!=head){
		if (trav->prev->alloc==0){ // merge with previous block, eliminating prev
			// printf("prev merge\n");
			struct memoryList* prevBlock = trav->prev;
			int newsize = trav->size + trav->prev->size; // combined size

			//update the linked list pointers of neighbor blocks
			prevBlock->next = trav->next;
			trav->next->prev = prevBlock;

			prevBlock->size = newsize;

			if(next == trav)	//if next is being freed, reset it
				next = prevBlock;

		free(trav);
		trav = prevBlock;	//free and reassign trav
		}
	} 

	// handle next block (this code is similar to previous if)
	
	if (trav->next!=head){
		if (trav->next->alloc==0){ // merge with next block, eliminating next
			// printf("next  merge\n");
			struct memoryList* temp = trav->next;
			int newsize = trav->size + trav->next->size;
			// dont need new ptr since base address is same
			trav->next= trav->next->next;
			trav->next->prev = trav;
			trav->size = newsize;

			if(next == temp)
				next = trav;
		}
	}
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
	do{
		if (trav->alloc==0) // hole has been found
			holes++;
		trav = trav->next;
	}while (trav!=head);
	return holes;
}

/* Get the number of bytes allocated */ // thess parts are pretty simple. 
int mem_allocated()
{
	struct memoryList* trav = head;
	int bytes = 0;
	do{
		if (trav->alloc==1) // allocated block, retrieve size
			bytes+= trav->size;
		trav = trav->next;
	}while (trav!=head);
	return bytes;
}

/* Number of non-allocated bytes */
int mem_free()
{
	struct memoryList* trav = head;
	int bytes = 0;
	do {
		if (trav->alloc==0) // unallocated block, retrieve size
			bytes+= trav->size;
		trav = trav->next;
	}while (trav!=head);
	return bytes;
}

/* Number of bytes in the largest contiguous area of unallocated memory */
int mem_largest_free()
{
	struct memoryList* trav = head;
	int max = 0;
	do {
		if (trav->size>max && trav->alloc == 0) // check if its bigger than prev max
			max = trav->size; 				   // and if unallocated
		trav = trav->next;
	} while (trav!=head);
	return max;
}

/* Number of free blocks smaller than "size" bytes. */
int mem_small_free(int size)
{
	struct memoryList* trav = head;
	int free = 0;
	do{
		if (trav->size<=size && trav->alloc==0) // this logically should be <, but it errors, so i made it <=
			free++; 
		trav = trav->next;
	} while (trav!=head);
	return free;
}

char mem_is_alloc(void *ptr)
{
	struct memoryList* trav = head;
	while (trav!=head){
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
	struct memoryList *test = head;
	int i = 1;
	do{
		printf("block: %d\n", i);
		printf("alloc: %d\n", test->alloc);
		printf("size: %d\n", test->size);
		printf("ptr: %p\n", test->ptr);
		test = test->next;
		i++;
	}while(test!=head);
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
