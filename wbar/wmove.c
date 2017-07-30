#include <stdio.h>
#include <string.h>
#include <proto/intuition.h>

int main(void)
{
  struct Screen *screen;

  // Lock workbench
  if(screen=LockPubScreen("Workbench"))
    {
      struct Window *window;
      ULONG ilock;
      int winposl = 0;

      ilock=LockIBase(0);
      
      printf("Screen width: %d\nScreen height: %d\n", screen->Width, screen->Height);

      // Go through windows on screen
      for (window=screen->FirstWindow;
           window;
           window=window->NextWindow)
           {
             // Not really interested in Workbench window information
             if (!stricmp(window->Title,"Workbench")==0)
               {
                 printf("Title: \"%s\"\nWidth: %d\nHeight: %d\nx: %d\ny: %d\n",
                         window->Title,
                         window->Width,
                         window->Height,
                         window->LeftEdge,
                         window->TopEdge);

                 // MoveWindow( window, 0, 20 );
                 ChangeWindowBox( window, 500, 200, 600, 300);
                 // Increment window left pos 
                 winposl = winposl+600;
               }
           }

      // Unlock IBase and screen
      UnlockIBase(ilock);
      UnlockPubScreen(0,screen);
    }

  return 0;
}
