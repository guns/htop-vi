
#include "DisplayOptionsPanel.h"

#include "Panel.h"
#include "CheckItem.h"
#include "Settings.h"
#include "ScreenManager.h"

#include "debug.h"
#include <assert.h>

/*{

typedef struct DisplayOptionsPanel_ {
   Panel super;

   Settings* settings;
   ScreenManager* scr;
} DisplayOptionsPanel;

}*/

static void DisplayOptionsPanel_delete(Object* object) {
   Panel* super = (Panel*) object;
   DisplayOptionsPanel* this = (DisplayOptionsPanel*) object;
   Panel_done(super);
   free(this);
}

static HandlerResult DisplayOptionsPanel_eventHandler(Panel* super, int ch) {
   DisplayOptionsPanel* this = (DisplayOptionsPanel*) super;
   
   HandlerResult result = IGNORED;
   CheckItem* selected = (CheckItem*) Panel_getSelected(super);

   switch(ch) {
   case 0x0a:       /* \n */
   case 0x0d:       /* \r */
   case KEY_ENTER:
   case KEY_MOUSE:
   case ' ':
      CheckItem_set(selected, ! (CheckItem_get(selected)) );
      result = HANDLED;
   }

   if (result == HANDLED) {
      this->settings->changed = true;
      Header* header = this->settings->header;
      Header_calculateHeight(header);
      Header_reinit(header);
      Header_draw(header);
      ScreenManager_resize(this->scr, this->scr->x1, header->height, this->scr->x2, this->scr->y2);
   }
   return result;
}

DisplayOptionsPanel* DisplayOptionsPanel_new(Settings* settings, ScreenManager* scr) {
   DisplayOptionsPanel* this = (DisplayOptionsPanel*) malloc(sizeof(DisplayOptionsPanel));
   Panel* super = (Panel*) this;
   Panel_init(super, 1, 1, 1, 1, CHECKITEM_CLASS, true);
   ((Object*)this)->delete = DisplayOptionsPanel_delete;

   this->settings = settings;
   this->scr = scr;
   super->eventHandler = DisplayOptionsPanel_eventHandler;

   Panel_setHeader(super, "Display options");
   Panel_add(super, (Object*) CheckItem_new(String_copy("Tree view"), &(settings->pl->treeView), false));
   Panel_add(super, (Object*) CheckItem_new(String_copy("Shadow other users' processes"), &(settings->pl->shadowOtherUsers), false));
   Panel_add(super, (Object*) CheckItem_new(String_copy("Hide kernel threads"), &(settings->pl->hideKernelThreads), false));
   Panel_add(super, (Object*) CheckItem_new(String_copy("Hide userland threads"), &(settings->pl->hideUserlandThreads), false));
   Panel_add(super, (Object*) CheckItem_new(String_copy("Display threads in a different color"), &(settings->pl->highlightThreads), false));
   Panel_add(super, (Object*) CheckItem_new(String_copy("Show custom thread names"), &(settings->pl->showThreadNames), false));
   Panel_add(super, (Object*) CheckItem_new(String_copy("Highlight program \"basename\""), &(settings->pl->highlightBaseName), false));
   Panel_add(super, (Object*) CheckItem_new(String_copy("Highlight large numbers in memory counters"), &(settings->pl->highlightMegabytes), false));
   Panel_add(super, (Object*) CheckItem_new(String_copy("Leave a margin around header"), &(settings->header->margin), false));
   Panel_add(super, (Object*) CheckItem_new(String_copy("Detailed CPU time (System/IO-Wait/Hard-IRQ/Soft-IRQ/Steal/Guest)"), &(settings->pl->detailedCPUTime), false));
   Panel_add(super, (Object*) CheckItem_new(String_copy("Count CPUs from 0 instead of 1"), &(settings->pl->countCPUsFromZero), false));
   return this;
}
