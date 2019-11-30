#include <stdio.h>
#include <stdlib.h>
					// i do some dumb stuff here cuz i still dont know pointers
int main(void){

	struct memoryList{
			int* x;
	};
	int x = 2;
	int a = 5;
	int* b;
	printf("b: %p\n", b);
	b = b + 1;
	// *b = &x;
	printf("x: %p\n", &x);
	printf("a: %p\n", &a);
	printf("b: %p\n", b);

}
