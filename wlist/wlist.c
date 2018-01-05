#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

// Config section. Modify the following to easily change output "table" 
static const char hseparator[] = "-";
static const char vseparator[] = "\xa6";
// Known number of hardcoded chars in table row output, spaces count
static const int numcharstable = 19;
// Known number of cells in table row output
static const int numcellstable = 7;
// Flag array size
enum { flag_size = 19 };
// Config section done

struct Wininfo {
	int winnr;
	const char *wintitle;
	short width;
	short height;
	short posx;
	short posy;
	char flag[flag_size];
};

struct Miscinfo {
	int mwinnr;
	int max_chars;
	int titlepad;
	int printpad;
};

struct Flagarray {
	unsigned long hexflag;
	char flagname[14];
};

// In order to check for bit flags, I opted for a struct contaning bitmasks from intuition.h
static struct Flagarray flagarray[] = {
	{0x00000100, "BACKDROP"},
	{0x00000400, "GIMMEZEROZERO"},
	{0x00000800, "BORDERLESS"},
	{0x00002000, "WINDOWACTIVE"},
	{0x02000000, "WBENCHWINDOW"},
	{0x10000000, "ZOOMED"}
};

// Function prototypes
int getmiscinfo(struct Screen *screen, struct Miscinfo *miscinfo);
void getwininfos(struct Screen *screen, struct Miscinfo *miscinfo,
		 struct Wininfo *wininfos);
int printwindows(const struct Wininfo *wininfos, struct Miscinfo *miscinfo);
void lowflagname(char *trflagname, int count);
void getflags(char *flags, struct Window *window);
void separate(int count);

int main(void)
{
	unsigned long ilock = LockIBase(0L);
	struct Screen *screen = LockPubScreen(NULL);
	struct Wininfo *wininfo;
	// Malloc miscinfo and wininfos
	struct Miscinfo *miscinfo = malloc(sizeof(struct Miscinfo));
	struct Wininfo *wininfos =
	    malloc(getmiscinfo(screen, miscinfo) * sizeof(*wininfo));
	int printer = 0;

	if (!miscinfo) {
		free(miscinfo);
		printf("Failed to fetch misc. info! Exiting.\n");

		return 1;
	}
	// Gather needed info from open windows
	getwininfos(screen, miscinfo, wininfos);
	if (!wininfos) {
		free(miscinfo);
		free(wininfos);
		printf("Failed to create window array of structs! Exiting.\n");

		return 1;
	}
	// Unlock Pubscreen and Intuitionbase both return nothing
	UnlockPubScreen(NULL, screen);
	UnlockIBase(ilock);

	// Print window list
	if ((printer = printwindows(wininfos, miscinfo)) != 0) {
		free(miscinfo);
		free(wininfos);
		printf("Failed to print windows! Exiting.\n");

		return 1;
	}
	// Clear mem
	free(miscinfo);
	free(wininfos);

	// And we are done
	return 0;
}

int getmiscinfo(struct Screen *screen, struct Miscinfo *miscinfo)
{
	int max_chars = 0;
	int wswidth = 0;
	int awidth = 0;
	unsigned char *text = NULL;
	struct Window *window;
	struct TextExtent te;
	struct RastPort *wrport;
	struct TextFont *wfont;

	// Set window counter 
	miscinfo->mwinnr = 0;

	for (window = screen->FirstWindow; window; window = window->NextWindow) {
		// Get width of terminal window for printing table and count windows.
		if (window->Flags & WINDOWACTIVE) {
			wfont = window->IFont;
			wrport = window->RPort;
			awidth =
			    window->Width - window->BorderLeft -
			    window->BorderRight;
			wswidth = window->WScreen->Width;
			text = malloc(wswidth * sizeof(unsigned char));
			memset(text, '-', wswidth - 1);
			max_chars =
			    TextFit(wrport, text, strlen((const char *)text),
				    &te, NULL, 1, awidth, wfont->tf_YSize + 1);
			miscinfo->max_chars = max_chars;
			miscinfo->printpad =
			    (max_chars - numcharstable) / numcellstable;
			free(text);
		}
		miscinfo->mwinnr++;
	}

	return miscinfo->mwinnr;
}

void separate(int count)
{
	int i = 0;

	for (i = 0; i < count; i++) {
		fputs(hseparator, stdout);
	}
}

int printwindows(const struct Wininfo *wininfos, struct Miscinfo *miscinfo)
{
	int i = 0;
	int titlelen = 0;
	int titlediff = 0;
	int titlepad = 0;
	char trflagname[4] = { 0 };

	printf("\n Open windows on Public Screen:\n");
	separate(miscinfo->max_chars);
	printf(" %s:\n", "Flaglegend");

	for (i = 0; i < sizeof flagarray / sizeof(struct Flagarray); ++i) {
		strncpy(trflagname, flagarray[i].flagname, 2);
		// uncapitalize letters
		lowflagname(trflagname, strlen(trflagname));
		printf(" %s: %s\n", trflagname, flagarray[i].flagname);
	}

	separate(miscinfo->max_chars);

	printf("\n %-*s %s %-*s %s %-*s %s %-*s %s %-*s %s %-*s %s %-*s\n",
	       miscinfo->printpad, "Number", vseparator,
	       miscinfo->printpad, "Title", vseparator,
	       miscinfo->printpad, "Width", vseparator,
	       miscinfo->printpad, "Height", vseparator,
	       miscinfo->printpad, "Pos X", vseparator,
	       miscinfo->printpad, "Pos Y", vseparator,
	       miscinfo->printpad, "Flag(s)");

	for (i = 0; i < miscinfo->mwinnr; i++) {

		separate(miscinfo->max_chars);

		if (strlen(wininfos[i].wintitle) <= miscinfo->printpad) {
			titlelen = strlen(wininfos[i].wintitle);
			titlediff = miscinfo->printpad - titlelen;
			titlepad = titlediff + titlelen;
			printf
			    (" %-*d %s %-*s %s %-*d %s %-*d %s %-*d %s %-*d %s %-*s\n",
			     miscinfo->printpad, wininfos[i].winnr, vseparator,
			     titlepad, wininfos[i].wintitle, vseparator,
			     miscinfo->printpad, wininfos[i].width, vseparator,
			     miscinfo->printpad, wininfos[i].height, vseparator,
			     miscinfo->printpad, wininfos[i].posx, vseparator,
			     miscinfo->printpad, wininfos[i].posy, vseparator,
			     miscinfo->printpad, wininfos[i].flag);
		} else {
			printf
			    (" %-*d %s %-.*s %s %-*d %s %-*d %s %-*d %s %-*d %s %-*s\n",
			     miscinfo->printpad, wininfos[i].winnr, vseparator,
			     miscinfo->printpad, wininfos[i].wintitle,
			     vseparator, miscinfo->printpad, wininfos[i].width,
			     vseparator, miscinfo->printpad, wininfos[i].height,
			     vseparator, miscinfo->printpad, wininfos[i].posx,
			     vseparator, miscinfo->printpad, wininfos[i].posy,
			     vseparator, miscinfo->printpad, wininfos[i].flag);
		}
	}

	separate(miscinfo->max_chars);

	printf("\n\n");

	return 0;
}

void getwininfos(struct Screen *screen, struct Miscinfo *miscinfo,
		 struct Wininfo *wininfos)
{
	int winnr = 0;
	char flags[flag_size] = { 0 };
	struct Window *window;

	for (window = screen->FirstWindow; window; window = window->NextWindow) {
		getflags(flags, window);
		// copy contents of flags array to wininfos[winnr].flag
		memcpy(wininfos[winnr].flag, flags, flag_size);
		// Clear flags array
		memset(&flags[0], 0, sizeof(flags));

		if (!window->Title) {
			wininfos[winnr].wintitle = "Unnamed Window";
		} else {
			wininfos[winnr].wintitle = (const char *)window->Title;
		}

		// Store window information in wininfos array
		wininfos[winnr].winnr = winnr;
		wininfos[winnr].width = window->Width;
		wininfos[winnr].height = window->Height;
		wininfos[winnr].posx = window->LeftEdge;
		wininfos[winnr].posy = window->TopEdge;
		winnr++;
	}

	return;
}

void lowflagname(char *trflagname, int count)
{
	int j = 0;

	for (j = 0; j < count; j++) {
		trflagname[j] = tolower(trflagname[j]);
	}
}

void getflags(char *flags, struct Window *window)
{
	int i = 0;
	//int j = 0;
	char trflagname[4] = { 0 };
	//char trlflagname[4] = { 0 };
	char tmpflag[19] = { 0 };
	char sep[3] = "/";

	for (i = 0; i < sizeof flagarray / sizeof(struct Flagarray); ++i) {
		if (window->Flags & flagarray[i].hexflag) {
			// clear truncate flagname variables
			memset(trflagname, 0, 4);
			// copy first to chars to trflagname
			strncpy(trflagname, flagarray[i].flagname, 2);
			// ensure NUL terminator
			trflagname[3] = '\0';
			// uncapitalize letters
			lowflagname(trflagname, strlen(trflagname));
			// Create flag strings
			if (flags[0] == 0) {
				strncat(trflagname, sep,
					strlen(trflagname) - 1);
				strcpy(flags, trflagname);
			} else {
				strncat(trflagname, sep,
					strlen(trflagname) - 1);
				strncat(flags, trflagname,
					flag_size - strlen(flags) - 1);
			}
		}
	}

	// Remove trailing "/"
	if (flags[strlen(flags) - 1] == '/') {
		strncpy(tmpflag, flags, strlen(flags));
		tmpflag[strlen(flags) - 1] = '\0';
		strncpy(flags, tmpflag, strlen(flags));
	}

	return;
}
