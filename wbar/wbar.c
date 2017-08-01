#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/dos.h>

int main(void)
{
	struct TagItem tagitem[5];
        struct IntuiText wtext;
	unsigned long ilock = LockIBase(0L);
	struct Screen *screen = LockPubScreen(NULL);
	struct Window *window;
        int swidth = screen->Width;
        int sheight = screen->Height;
        int wbarheight = 20;

        // printf("swidth: %d\n", swidth);
        tagitem[0].ti_Tag = WA_Width;
        tagitem[0].ti_Data = swidth;
        tagitem[1].ti_Tag = WA_Height;
        tagitem[1].ti_Data = wbarheight;
        tagitem[2].ti_Tag = WA_Top;
        tagitem[2].ti_Data = sheight - wbarheight;
        tagitem[3].ti_Tag = WA_Borderless;
        tagitem[3].ti_Data = 1;
        tagitem[4].ti_Tag = TAG_DONE;

        if (window = OpenWindowTagList(NULL, tagitem)) { 
		wtext.LeftEdge = 10;
		wtext.TopEdge = 2;
		wtext.IText = "Hello Intuition!";
		wtext.ITextFont = NULL;
		wtext.DrawMode = JAM1;
		wtext.FrontPen = 1;
		wtext.NextText = NULL;

		PrintIText( window->RPort, &wtext, 0, 0 );
		Delay(200);
        	CloseWindow(window);
	}

	// Unlock Pubscreen and Intuitionbase both return nothing
	UnlockPubScreen(NULL, screen);
	UnlockIBase(ilock);
	return 0;

}

