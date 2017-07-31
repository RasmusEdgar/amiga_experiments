#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <dos/dosextens.h>
#include <ctype.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/dos.h>

int main(void)
{
	struct TagItem tagitem[3];
	unsigned long ilock = LockIBase(0L);
	struct Screen *screen = LockPubScreen(NULL);
	struct Window *window;
        int swidth = screen->Width;

        // printf("swidth: %d\n", swidth);
        tagitem[0].ti_Tag = WA_Width;
        tagitem[0].ti_Data = swidth;
        tagitem[1].ti_Tag = WA_Height;
        tagitem[1].ti_Data = 20;
        tagitem[2].ti_Tag = TAG_DONE;

        if (window = OpenWindowTagList(NULL, tagitem)) { 
		Delay(500);
	}
        CloseWindow(window);

	// Unlock Pubscreen and Intuitionbase both return nothing
	UnlockPubScreen(NULL, screen);
	UnlockIBase(ilock);
	return 0;

}
