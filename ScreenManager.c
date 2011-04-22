/*
htop
(C) 2004-2010 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "ScreenManager.h"
#include "Panel.h"
#include "Object.h"
#include "Vector.h"
#include "Header.h"
#include "FunctionBar.h"

#include "debug.h"
#include <assert.h>
#include <time.h>

#include <stdbool.h>

/*{

typedef enum Orientation_ {
   VERTICAL,
   HORIZONTAL
} Orientation;

typedef struct ScreenManager_ {
   int x1;
   int y1;
   int x2;
   int y2;
   Orientation orientation;
   Vector* items;
   Vector* fuBars;
   int itemCount;
   const FunctionBar* fuBar;
   const Header* header;
   time_t lastScan;
   bool owner;
} ScreenManager;

}*/

ScreenManager* ScreenManager_new(int x1, int y1, int x2, int y2, Orientation orientation, const Header* header, bool owner) {
   ScreenManager* this;
   this = malloc(sizeof(ScreenManager));
   this->x1 = x1;
   this->y1 = y1;
   this->x2 = x2;
   this->y2 = y2;
   this->fuBar = NULL;
   this->orientation = orientation;
   this->items = Vector_new(PANEL_CLASS, owner, DEFAULT_SIZE, NULL);
   this->fuBars = Vector_new(FUNCTIONBAR_CLASS, true, DEFAULT_SIZE, NULL);
   this->itemCount = 0;
   this->header = header;
   this->owner = owner;
   return this;
}

void ScreenManager_delete(ScreenManager* this) {
   Vector_delete(this->items);
   Vector_delete(this->fuBars);
   free(this);
}

inline int ScreenManager_size(ScreenManager* this) {
   return this->itemCount;
}

void ScreenManager_add(ScreenManager* this, Panel* item, FunctionBar* fuBar, int size) {
   if (this->orientation == HORIZONTAL) {
      int lastX = 0;
      if (this->itemCount > 0) {
         Panel* last = (Panel*) Vector_get(this->items, this->itemCount - 1);
         lastX = last->x + last->w + 1;
      }
      if (size > 0) {
         Panel_resize(item, size, LINES-this->y1+this->y2);
      } else {
         Panel_resize(item, COLS-this->x1+this->x2-lastX, LINES-this->y1+this->y2);
      }
      Panel_move(item, lastX, this->y1);
   }
   // TODO: VERTICAL
   Vector_add(this->items, item);
   if (fuBar)
      Vector_add(this->fuBars, fuBar);
   else
      Vector_add(this->fuBars, FunctionBar_new(NULL, NULL, NULL));
   if (!this->fuBar && fuBar) this->fuBar = fuBar;
   item->needsRedraw = true;
   this->itemCount++;
}

Panel* ScreenManager_remove(ScreenManager* this, int idx) {
   assert(this->itemCount > idx);
   Panel* panel = (Panel*) Vector_remove(this->items, idx);
   Vector_remove(this->fuBars, idx);
   this->fuBar = NULL;
   this->itemCount--;
   return panel;
}

void ScreenManager_resize(ScreenManager* this, int x1, int y1, int x2, int y2) {
   this->x1 = x1;
   this->y1 = y1;
   this->x2 = x2;
   this->y2 = y2;
   int items = this->itemCount;
   int lastX = 0;
   for (int i = 0; i < items - 1; i++) {
      Panel* panel = (Panel*) Vector_get(this->items, i);
      Panel_resize(panel, panel->w, LINES-y1+y2);
      Panel_move(panel, lastX, y1);
      lastX = panel->x + panel->w + 1;
   }
   Panel* panel = (Panel*) Vector_get(this->items, items-1);
   Panel_resize(panel, COLS-x1+x2-lastX, LINES-y1+y2);
   Panel_move(panel, lastX, y1);
}

void ScreenManager_run(ScreenManager* this, Panel** lastFocus, int* lastKey) {
   bool quit = false;
   int focus = 0;
   
   Panel* panelFocus = (Panel*) Vector_get(this->items, focus);
   if (this->fuBar)
      FunctionBar_draw(this->fuBar, NULL);
   
   this->lastScan = 0;

   int ch = 0;
   while (!quit) {
      int items = this->itemCount;
      if (this->header) {
         time_t now = time(NULL);
         if (now > this->lastScan) {
            ProcessList_scan(this->header->pl);
            this->lastScan = now;
         }
         Header_draw(this->header);
      }
      for (int i = 0; i < items; i++) {
         Panel* panel = (Panel*) Vector_get(this->items, i);
         Panel_draw(panel, i == focus);
         if (i < items) {
            if (this->orientation == HORIZONTAL) {
               mvvline(panel->y, panel->x+panel->w, ' ', panel->h+1);
            }
         }
      }
      FunctionBar* bar = (FunctionBar*) Vector_get(this->fuBars, focus);
      if (bar)
         this->fuBar = bar;
      if (this->fuBar)
         FunctionBar_draw(this->fuBar, NULL);

      ch = getch();
      
      if (ch == KEY_MOUSE) {
         MEVENT mevent;
         int ok = getmouse(&mevent);
         if (ok == OK) {
            if (mevent.y == LINES - 1) {
               ch = FunctionBar_synthesizeEvent(this->fuBar, mevent.x);
            } else {
               for (int i = 0; i < this->itemCount; i++) {
                  Panel* panel = (Panel*) Vector_get(this->items, i);
                  if (mevent.x > panel->x && mevent.x <= panel->x+panel->w &&
                     mevent.y > panel->y && mevent.y <= panel->y+panel->h) {
                     focus = i;
                     panelFocus = panel;
                     Panel_setSelected(panel, mevent.y - panel->y + panel->scrollV - 1);
                     break;
                  }
               }
            }
         }
      }
      
      if (panelFocus->eventHandler) {
         HandlerResult result = panelFocus->eventHandler(panelFocus, ch);
         if (result == HANDLED) {
            continue;
         } else if (result == BREAK_LOOP) {
            quit = true;
            continue;
         }
      }
      
      switch (ch) {
      case ERR:
         continue;
      case KEY_RESIZE:
      {
         ScreenManager_resize(this, this->x1, this->y1, this->x2, this->y2);
         continue;
      }
      case 'h':         /* vi */
      case KEY_LEFT:
         tryLeft:
         if (focus > 0)
            focus--;
         panelFocus = (Panel*) Vector_get(this->items, focus);
         if (Panel_size(panelFocus) == 0 && focus > 0)
            goto tryLeft;
         break;
      case 'l':         /* vi */
      case KEY_RIGHT:
         tryRight:
         if (focus < this->itemCount - 1)
            focus++;
         panelFocus = (Panel*) Vector_get(this->items, focus);
         if (Panel_size(panelFocus) == 0 && focus < this->itemCount - 1)
            goto tryRight;
         break;
      case 'Q':         /* vi: often remapped to :q! */
      case 'q':
      case 27:          /* Esc */
         quit = true;
         continue;
      default:
         Panel_onKey(panelFocus, ch);
         break;
      }
   }

   *lastFocus = panelFocus;
   *lastKey = ch;
}
