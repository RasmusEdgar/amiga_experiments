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
	char flagname[54];
};

static struct Flagarray flagarray[] = {
	{0x00000100, "BACKDROP"},
	{0x00000400, "GIMMEZEROZERO"},
	{0x00000800, "BORDERLESS"},
	{0x00002000, "WINDOWACTIVE"},
	{0x02000000, "WBENCHWINDOW"},
	{0x10000000, "ZOOMED"}
};

// Function prototypes
struct Miscinfo *getmiscinfo(struct Screen *screen);
struct Wininfo *getwininfos(struct Screen *screen, struct Miscinfo *miscinfo);
int printwindows(const struct Wininfo *wininfos, struct Miscinfo *miscinfo);
void getflags(char *flags, struct Window *window);

int main(void)
{
	struct Screen *screen;
	struct Wininfo *wininfos;
	struct Miscinfo *miscinfo;
	int printer = 0;
	unsigned long ilock = 0;

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
		free(miscinfo);
		printf("Failed to fetch misc. info! Exiting.\n");
		return 1;
	}
	// Gather needed info from open windows
	wininfos = getwininfos(screen, miscinfo);
	if (!wininfos) {
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

struct Miscinfo *getmiscinfo(struct Screen *screen)
{
	int mwinnr = 0;
	int max_chars = 0;
	int wswidth = 0;
	int awidth = 0;
	unsigned char *text = NULL;
	struct Window *window;
	struct TextExtent te;
	struct RastPort *wrport;
	struct TextFont *wfont;

	// Malloc 
	struct Miscinfo *miscinfo = malloc(sizeof(struct Miscinfo));

	if (miscinfo == NULL) {
		return NULL;
	}

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
		mwinnr++;
	}
	miscinfo->mwinnr = mwinnr;
	return miscinfo;
}

int printwindows(const struct Wininfo *wininfos, struct Miscinfo *miscinfo)
{
	int a = 0;
	int i = 0;
	int j = 0;
	int titlelen = 0;
	int titlediff = 0;
	int titlepad = 0;
	char trflagname[4] = { 0 };
	char trlflagname[4] = { 0 };
	printf("\n Open windows on Public Screen:\n");
	for (a = 0; a < miscinfo->max_chars; a++) {
		fputs(hseparator, stdout);
	}
	printf(" %s:\n", "Flaglegend");
	for (i = 0; i < sizeof flagarray / sizeof(struct Flagarray); ++i) {
		strncpy(trflagname, flagarray[i].flagname, 2);
		for (j = 0; j < strlen(trflagname); j++) {
			trlflagname[j] = tolower(trflagname[j]);
		}
		printf(" %s: %s\n", trlflagname, flagarray[i].flagname);
	}
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
	for (i = 0; i < miscinfo->mwinnr; i++) {
		for (a = 0; a < miscinfo->max_chars; a++) {
			fputs(hseparator, stdout);
		}
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
	for (a = 0; a < miscinfo->max_chars; a++) {
		fputs(hseparator, stdout);
	}
	printf("\n\n");
	return 0;
}

struct Wininfo *getwininfos(struct Screen *screen, struct Miscinfo *miscinfo)
{
	int winnr = 0;
	char flags[flag_size] = { 0 };

	struct Window *window;
	struct Wininfo *wininfo;

	// Allocate memory for struct array based on window count from miscinfo function
	struct Wininfo *wininfos = malloc(miscinfo->mwinnr * sizeof(*wininfo));

	if (wininfos == NULL) {
		return NULL;
	}
	// Loop through windows on screen
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
	return wininfos;
}

void getflags(char *flags, struct Window *window)
{
	int i = 0;
	int j = 0;
	char trflagname[4] = { 0 };
	char trlflagname[4] = { 0 };
	char tmpflag[19] = { 0 };
	char sep[3] = "/";

	for (i = 0; i < sizeof flagarray / sizeof(struct Flagarray); ++i) {
		if (window->Flags & flagarray[i].hexflag) {
			// clear truncate flagname variables
			memset(trflagname, 0, 4);
			memset(trlflagname, 0, 4);
			// copy first to chars to trflagname
			strncpy(trflagname, flagarray[i].flagname, 2);
			// ensure NUL terminator
			trflagname[3] = '\0';
			// uncapitalize letters
			for (j = 0; j < strlen(trflagname); j++) {
				trlflagname[j] = tolower(trflagname[j]);
			}
			// Create flag strings
			if (flags[0] == 0) {
				strncat(trlflagname, sep,
					strlen(trlflagname) - 1);
				strcpy(flags, trlflagname);
			} else {
				strncat(trlflagname, sep,
					strlen(trlflagname) - 1);
				strncat(flags, trlflagname,
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
