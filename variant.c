#include <stdio.h>
#include <stdlib.h>

void main(int argc, char* argv[])
	{
		int n = atoi(argv[1]);
		int c = atoi(argv[2]);
		srand(n);
		int v = (rand() % c) + 1;
		printf("Variant: %d \n", v);
	}
