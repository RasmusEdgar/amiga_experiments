#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

struct Wininfo {
    int winnr;
    char *wintitle;
    short width;
    short height;
    short posx;
    short posy;
    char flag[16];
};

struct Miscinfo {
    int winnr;
    int max_chars;
    int titlepad;
    int printpad;
};

// Function prototypes
struct Miscinfo *getmiscinfo(struct Screen *screen);
struct Wininfo *getwininfos(struct Screen *screen, struct Miscinfo *miscinfo);
int printwindows(const struct Wininfo *wininfos, struct Miscinfo *miscinfo);

// Config section. Modify the following to easily change output "table" 
static const char hseparator[] = "-";
static const char vseparator[] = "\xa6";
// Known number of hardcoded chars in table row output, spaces count
static const int numcharstable = 19;
// Known number of cells in table row output
static const int numcellstable = 7;
// Config section done

int main(void)
{
    struct Screen *screen;
    struct Wininfo *wininfos;
    struct Miscinfo *miscinfo;
    int printer;
    long unsigned ilock;

    // Lockdown to avoid deadlocks
    if ((ilock = LockIBase(0)) != 0) {
	printf("Failed to lock IntuitionBase! Exiting.\n");
	return 1;
    }
    if (!(screen = LockPubScreen(NULL))) {
	printf("Failed to lock default pubscreen! Exiting.\n");
	return 1;
    }
    // Get window count and other information needed to print on terminal
    miscinfo = getmiscinfo(screen);

    if (!miscinfo) {
	printf("Failed to fetch misc. info! Exiting.\n");
	return 1;
    }
    // Gather needed info from open windows
    wininfos = getwininfos(screen, miscinfo);
    if (!wininfos) {
	printf("Failed to create window array of structs! Exiting.\n");
	return 1;
    }
    // Unlock Pubscreen and Intuitionbase both return nothing
    UnlockPubScreen(0L, screen);
    UnlockIBase(ilock);

    // Print window list
    if ((printer = printwindows(wininfos, miscinfo)) != 0) {
	printf("Failed to print windows! Exiting.\n");
	return 1;
    }
    // Clear mem
    free(miscinfo);
    free(wininfos);

    // And we are done
    return 0;

}

struct Miscinfo *getmiscinfo(struct Screen *screen)
{
    int winnr = 0, max_chars, wswidth, awidth;
    char *text;
    struct Window *window;
    struct TextExtent te;
    struct RastPort *wrport;
    struct TextFont *wfont;
    struct Miscinfo *miscinfo = malloc(sizeof(struct Miscinfo));
    for (window = screen->FirstWindow; window; window = window->NextWindow) {
	// Get width of terminal window for printing table and count windows.
	if (window->Flags & WINDOWACTIVE) {
	    wfont = window->IFont;
	    wrport = window->RPort;
	    awidth = window->Width - window->BorderLeft - window->BorderRight;
	    wswidth = window->WScreen->Width;
	    text = malloc(wswidth * sizeof(char));
	    memset(text, '-', wswidth - 1);
	    max_chars =
		TextFit(wrport, text, strlen(text), &te, NULL, 1,
			awidth, wfont->tf_YSize + 1);
	    miscinfo->max_chars = max_chars;
	    miscinfo->printpad = (max_chars - numcharstable) / numcellstable;
	    free(text);
	}
	winnr++;
    }
    miscinfo->winnr = winnr;
    return miscinfo;
}

int printwindows(const struct Wininfo *wininfos, struct Miscinfo *miscinfo)
{
    int a, i;
    printf("\n Open windows on Public Screen:\n");
    for (a = 0; a < miscinfo->max_chars; a++) {
	fputs(hseparator, stdout);
    }
    printf(" Flag legend:\n %s\n %s\n %s\n %s\n %s\n %s\n",
          "bd = BACDROP", "ac = WINACTIVE",
          "bl = BORDERLESS", "wb = WBENCHWINDOW",
          "gz = GIMMEZEROZERO", "zo = ZOOMED");
    for (a = 0; a < miscinfo->max_chars; a++) {
	fputs(hseparator, stdout);
    }
    printf("\n %-*s %s %-*s %s %-*s %s %-*s %s %-*s %s %-*s %s %-*s\n",
	   miscinfo->printpad, "Number", vseparator,
	   miscinfo->printpad, "Title", vseparator,
	   miscinfo->printpad, "Width", vseparator,
	   miscinfo->printpad, "Height", vseparator,
	   miscinfo->printpad, "Pos X", vseparator,
	   miscinfo->printpad, "Pos Y", vseparator,
	   miscinfo->printpad, "Flag(s)");
    for (i = 0; i < miscinfo->winnr; i++) {
	for (a = 0; a < miscinfo->max_chars; a++) {
	    fputs(hseparator, stdout);
	}
	if (strlen(wininfos[i].wintitle) <= (unsigned int)miscinfo->printpad) {
	    int titlelen = 0, titlediff = 0, titlepad = 0;
	    titlelen = strlen(wininfos[i].wintitle);
	    titlediff = miscinfo->printpad - titlelen;
	    titlepad = titlediff + titlelen;
	    printf(" %-*d %s %-*s %s %-*d %s %-*d %s %-*d %s %-*d %s %-*s\n",
		   miscinfo->printpad, wininfos[i].winnr, vseparator,
		   titlepad, wininfos[i].wintitle, vseparator,
		   miscinfo->printpad, wininfos[i].width, vseparator,
		   miscinfo->printpad, wininfos[i].height, vseparator,
		   miscinfo->printpad, wininfos[i].posx, vseparator,
		   miscinfo->printpad, wininfos[i].posy, vseparator,
		   miscinfo->printpad, wininfos[i].flag);
	} else {
	    printf(" %-*d %s %-.*s %s %-*d %s %-*d %s %-*d %s %-*d %s %-*s\n",
		   miscinfo->printpad, wininfos[i].winnr, vseparator,
		   miscinfo->printpad, wininfos[i].wintitle, vseparator,
		   miscinfo->printpad, wininfos[i].width, vseparator,
		   miscinfo->printpad, wininfos[i].height, vseparator,
		   miscinfo->printpad, wininfos[i].posx, vseparator,
		   miscinfo->printpad, wininfos[i].posy, vseparator,
		   miscinfo->printpad, wininfos[i].flag);
	}
    }
    for (a = 0; a < miscinfo->max_chars; a++) {
	fputs(hseparator, stdout);
    }
    printf("\n\n");
    return 0;
}

struct Wininfo *getwininfos(struct Screen *screen, struct Miscinfo *miscinfo)
{

    struct Window *window;
    struct Wininfo *wininfo;
    char *wintitle, bd[3], ac[3], bl[3], wb[3], gz[3], zo[3];

    // Allocate memory for struct array based on window count from miscinfo function
    struct Wininfo *wininfos = malloc(miscinfo->winnr * sizeof(*wininfo));

    // Reset winnr
    int winnr = 0;

    // Loop through windows on screen
    for (window = screen->FirstWindow; window; window = window->NextWindow) {

        // Check window flags for window
	// First reset flags char array
	char flags[16] = { 0 };

	if (window->Flags & BACKDROP) {
	    if (strlen(flags) == 0) {
		strcpy(bd, "bd");
		strcpy(flags, bd);
	    } else {
		strcpy(bd, "/bd");
		strcat(flags, bd);
	    }
	}

	if (window->Flags & WINDOWACTIVE) {
	    if (strlen(flags) == 0) {
		strcpy(ac, "ac");
		strcpy(flags, ac);
	    } else {
		strcpy(ac, "/ac");
		strcat(flags, ac);
	    }
	}

	if (window->Flags & BORDERLESS) {
	    if (strlen(flags) == 0) {
		strcpy(bl, "bl");
		strcpy(flags, bl);
	    } else {
		strcpy(bl, "/bl");
		strcat(flags, bl);
	    }
	}

	if (window->Flags & WBENCHWINDOW) {
	    if (strlen(flags) == 0) {
		strcpy(wb, "wb");
		strcpy(flags, wb);
	    } else {
		strcpy(wb, "/wb");
		strcat(flags, wb);
	    }
	}

	if (window->Flags & GIMMEZEROZERO) {
	    if (strlen(flags) == 0) {
		strcpy(gz, "gz");
		strcpy(flags, gz);
	    } else {
		strcpy(gz, "/gz");
		strcat(flags, gz);
	    }
	}

	if (window->Flags & ZOOMED) {
	    if (strlen(flags) == 0) {
		strcpy(zo, "zo");
		strcpy(flags, zo);
	    } else {
		strcpy(zo, "/zo");
		strcat(flags, zo);
	    }
	}

	// copy contents of flags array to wininfos[winnr].flag
	memcpy(wininfos[winnr].flag, flags, 14);

	// pad last char with zero
	wininfos[winnr].flag[16] = 0;

	if (!window->Title) {
	    wintitle = "Unnamed Winwdow";
	} else {
	    wintitle = window->Title;
	}

	// Store window information in wininfos array
	wininfos[winnr].winnr = winnr;
	wininfos[winnr].wintitle = wintitle;
	wininfos[winnr].width = window->Width;
	wininfos[winnr].height = window->Height;
	wininfos[winnr].posx = window->LeftEdge;
	wininfos[winnr].posy = window->TopEdge;
	winnr++;
    }
    return wininfos;
}
