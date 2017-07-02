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
};

struct Miscinfo {
  int winnr;
  int max_chars;
  int titlepad;
  int printpad;
};

// Config section. Modify the following to easily change output "table" 
static const char hseparator[] = "-";
static const char vseparator[] = "\xa6";
// Known number of hardcoded chars in table row output, spaces count
static const int numcharstable = 15;
// Known number of cells in table row output
static const int numcellstable = 6;
// Config section done

int main(void)
{
  struct Screen *screen;
  struct Wininfo *wininfos;
  struct Miscinfo *miscinfo;
  int printer;
  struct Miscinfo *getmiscinfo(struct Screen *screen);
  struct Wininfo *getwininfos(struct Screen *screen, struct Miscinfo *miscinfo);
  int printwindows(const struct Wininfo *wininfos, struct Miscinfo *miscinfo);
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
    // Ignore Workbench window, and any backdropped windows.
    if (!(window->Flags & BACKDROP)
	&& (!stricmp(window->Title, "Workbench") == 0)) {
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
  }
  miscinfo->winnr = winnr;
  return miscinfo;
}

int printwindows(const struct Wininfo *wininfos, struct Miscinfo *miscinfo)
{
  int a, i;
  printf("\nOpen windows on Public Screen:\n");
  for (i = 0; i < miscinfo->winnr; i++) {
    for (a = 0; a < miscinfo->max_chars; a++) {
      fputs(hseparator, stdout);
    }
    printf("\n%-*s %s %-*s %s %-*s %s %-*s %s %-*s %s %-*s\n",
        miscinfo->printpad,"Number",vseparator,
        miscinfo->printpad,"Title",vseparator,
        miscinfo->printpad,"Width",vseparator,
        miscinfo->printpad,"Height",vseparator,
        miscinfo->printpad,"Pos X",vseparator,
        miscinfo->printpad,"Pos Y",vseparator);
    for (a = 0; a < miscinfo->max_chars; a++) {
      fputs(hseparator, stdout);
    }
    if (strlen(wininfos[i].wintitle) <= (unsigned int) miscinfo->printpad) {
       int titlelen = 0, titlediff = 0, titlepad = 0;
       titlelen = strlen(wininfos[i].wintitle);
       titlediff = miscinfo->printpad - titlelen;
       titlepad = titlediff + titlelen; 
       printf("%-*d %s %-*s %s %-*d %s %-*d %s %-*d %s %-*d\n",
	   miscinfo->printpad,wininfos[i].winnr,vseparator,
	   titlepad,wininfos[i].wintitle,vseparator,
	   miscinfo->printpad,wininfos[i].width,vseparator,
	   miscinfo->printpad,wininfos[i].height,vseparator,
           miscinfo->printpad,wininfos[i].posx,vseparator,
           miscinfo->printpad,wininfos[i].posy);
    } else { 
       printf("%-*d %s %-.*s %s %-*d %s %-*d %s %-*d %s %-*d\n",
	   miscinfo->printpad,wininfos[i].winnr,vseparator,
	   miscinfo->printpad,wininfos[i].wintitle,vseparator,
	   miscinfo->printpad,wininfos[i].width,vseparator,
	   miscinfo->printpad,wininfos[i].height,vseparator,
           miscinfo->printpad,wininfos[i].posx,vseparator,
           miscinfo->printpad,wininfos[i].posy);
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
  char *wintitle;

  // Allocate memory for struct array based on window count from miscinfo function
  struct Wininfo *wininfos = malloc(miscinfo->winnr * sizeof(*wininfo));
  // Reset winnr
  int winnr = 0;

  // Loop through windows on screen
  for (window = screen->FirstWindow; window; window = window->NextWindow) {
    // Ignore Workbench window and any backdropped ones.
    if (!(window->Flags & BACKDROP)
	&& (!stricmp(window->Title, "Workbench") == 0)) {
      // Set default window title if  NULL
      if (!window->Title) {
	wintitle = "Unnamed Window";
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
  }

  return wininfos;
}
