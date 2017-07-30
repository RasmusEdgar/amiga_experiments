#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

int main(void)
{
	unsigned long ilock = LockIBase(0L);
	struct Screen *screen = LockPubScreen(NULL);
        

	// Unlock Pubscreen and Intuitionbase both return nothing
	UnlockPubScreen(NULL, screen);
	UnlockIBase(ilock);
	return 0;

}
