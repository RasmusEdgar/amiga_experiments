#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <proto/intuition.h>

struct Wininfo {
  int winnr;
  char *wintitle;
  short width;
  short height;
  short posx;
  short posy;
};

int main(void)
{
  struct Screen *screen;
  struct Wininfo *wininfos;
  int winnr = 0;
  int printer;
  int wincount(struct Screen *screen);
  struct Wininfo *getwininfos(struct Screen *screen, int winnr);
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
  if (!(winnr = wincount(screen))) {
    printf("Failed to count windows! Exiting.\n");
    return 1;
  }
  wininfos = getwininfos(screen, winnr);
  if (!wininfos) {
    printf("Failed to create window array of structs! Exiting.\n");
    return 1;
  }
  // Unlock Pubscreen and Intuitionbase both return nothing
  UnlockPubScreen(0L, screen);
  UnlockIBase(ilock);

  // Print window list
  if ((printer = printwindows(wininfos, winnr)) != 0) {
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

int wincount(struct Screen *screen)
{
  int winnr = 0;
  struct Window *window;
  for (window = screen->FirstWindow; window; window = window->NextWindow) {
    // Ignore Workbench window, and any backdropped windows.
    if (!(window->Flags & BACKDROP)
	&& (!stricmp(window->Title, "Workbench") == 0)) {
      winnr++;
    }
  }
  return winnr;
}

int printwindows(const struct Wininfo *wininfos, int winnr)
{
  int i;
  for (i = 0; i < winnr; i++) {
    printf("Nr: %d | Title: %s | Width: %d | Height: %d | X: %d | Y: %d \n",
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

struct Wininfo *getwininfos(struct Screen *screen, int winnr)
{

  struct Window *window;
  struct Wininfo *wininfo;
  char *wintitle;

  // Allocate memory for struct array based on window count from wincount function
  struct Wininfo *wininfos = malloc(winnr * sizeof(*wininfo));
  // Reset winnr
  winnr = 0;

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
