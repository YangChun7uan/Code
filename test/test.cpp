
#include "Headdefine.h"
extern int a;

extern "C" int Print(int i)
{
	printf("----i %d\n", i);
	return 0;
}
