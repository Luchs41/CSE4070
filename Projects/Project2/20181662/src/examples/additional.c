#include <stdio.h>
#include <syscall.h>
#include <stdlib.h>
int main (int argc, char **argv)
{
	argc++;
	argc--;
	int a = atoi(argv[1]);
	int b = atoi(argv[2]);
	int c = atoi(argv[3]);
	int d = atoi(argv[4]);
	printf("%d %d\n", fibonacci(a), max_of_four_int(a, b, c, d));
	return EXIT_SUCCESS;
}
