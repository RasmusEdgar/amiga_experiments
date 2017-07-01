#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <proto/intuition.h>
#include <clib/diskfont_protos.h>
#include <graphics/text.h>
#include <graphics/rastport.h>
#include <clib/graphics_protos.h>

struct Wininfo {
  int winnr;
  char *wintitle;
  short width;
  short height;
  short posx;
  short posy;
  // int active;
};

struct Miscinfo {
  int winnr;
  int max_chars;
  int indent;
};

int main(void)
{
  struct Screen *screen;
  struct Wininfo *wininfos;
  int winnr = 0, printer;
  struct Miscinfo *miscinfo(struct Screen *screen);
  struct Wininfo *getwininfos(struct Screen *screen, struct Miscinfo *miscinfo);
  int printwindows(const struct Wininfo *wininfos, int winnr);
  long unsigned ilock;

  if ((ilock = LockIBase(0)) != 0) {
    printf("Failed to lock IntuitionBase! Exiting.\n");
    return 1;
  }
  if (!(screen = LockPubScreen(NULL))) {
    printf("Failed to lock default pubscreen! Exiting.\n");
    return 1;
  }
  if (!(miscinfo = miscinfo(screen))) {
    printf("Failed to count windows! Exiting.\n");
    return 1;
  }
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
  free(wininfos);

  // Nilling wininfos
  wininfos = NULL;
  if (wininfos) {
    printf("Failed free mem from array. Exiting.\n");
    return 1;
  }
  // And we are done
  return 0;

}

struct Miscinfo *miscinfo(struct Screen *screen)
{
  int winnr = 0;
  struct Window *window;
  int a, i, max_chars, n;
  char *text;
  struct TextExtent te;
  int awidth;
  struct RastPort *wrport;
  struct TextFont *wfont;
  struct Screen *wscreen;
  for (window = screen->FirstWindow; window; window = window->NextWindow) {
    // Ignore Workbench window, and any backdropped windows.
    if (!(window->Flags & BACKDROP)
	&& (!stricmp(window->Title, "Workbench") == 0)) {
      if (window->Flags & WINDOWACTIVE) {
        wfont = window->IFont;
        wrport = window->RPort;
        awidth =
            window->Width - window->BorderLeft - window->BorderRight;
        wscreen = window->WScreen;
        n=wscreen->Width;
        printf("wscrenw= %d\n", n);
        text=malloc(n*sizeof(char));
        memset(text,'-',n-1);
        max_chars =
        TextFit(wrport, text, strlen(text), &te, NULL, 1,
                 awidth, wfont->tf_YSize + 1)
        miscinfo.indent = max_chars / wininfos[i].wfont->tf_YSize; 
        miscinfo.max_chars = max_chars; 
        free(text);
      }
    }
    winnr++;
  }
  miscinfo.winnr = winnr;
  return miscinfo;
}

int printwindows(const struct Wininfo *wininfos, struct Miscinfo *miscinfo)
{

  for (i = 0; i < miscinfo.winnr; i++) {
    printf("Nr: %d | Title: %.*s | Width: %d | Height: %d | X: %d | Y: %d \n",
	   wininfos[i].winnr,
	   wininfos[i].wintitle,
	   wininfos[i].width,
	   wininfos[i].height, wininfos[i].posx, wininfos[i].posy);
  }
  if (!i) {
    return 1;
  } else {
    return 0;
  }
}

struct Wininfo *getwininfos(struct Screen *screen, struct Miscinfo *miscinfo)
{

  struct Window *window;
  struct Wininfo *wininfo;
  char *wintitle;
  int winnr = 0;

  // Allocate memory for struct array based on window count from miscinfo function
  struct Wininfo *wininfos = malloc(miscinfo.winnr * sizeof(*wininfo));
  // Reset winnr

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
      if (window->Flags & WINDOWACTIVE) {
	wininfos[winnr].active = 1;
	wininfos[winnr].wfont = window->IFont;
	wininfos[winnr].wrport = window->RPort;
	wininfos[winnr].awidth =
	    window->Width - window->BorderLeft - window->BorderRight;
        wininfos[winnr].wscreen = window->WScreen;
      } else {
        wininfos[winnr].active = 0;
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
