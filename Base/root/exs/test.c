#include "stdio.h"

int count;

int fibonacci (int i) {
	if (i <= 1) return 1;
	return fibonacci(i - 1) + fibonacci(i - 2);
}

int main () {
	int i;
	i = 0;
	count = 15;
	while (i <= count) {
		printf("fibonacci(%d) return %d\n", i, fibonacci(i));
		i++;
	}
	return 0;
}
