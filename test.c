#include <time.h>
#include <stdio.h>

int main() {

	clock_t start,end;
	int i;
	start = clock();
	printf("Clocks ticks at starting time: %ld\n", start);
	for (i=0;i<1000000000;i++){}
	end =clock();
	printf("Clock ticks at end time: %ld\n", end);
	printf("CLOCKS_PER_SECOND: %ld\n", CLOCKS_PER_SEC);
	printf("time elapsed %ld\n: ", (end-start)/CLOCKS_PER_SEC);
	printf("size of CLOCKS_PER_SEC: %ld\n", sizeof(CLOCKS_PER_SEC));
	printf("size of clock_t: %ld\n", sizeof(start));
	return 0;
}
