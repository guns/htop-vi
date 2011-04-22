
#include "AvailableColumnsPanel.h"
#include "Settings.h"
#include "Header.h"
#include "ScreenManager.h"
#include "ColumnsPanel.h"

#include "Panel.h"

#include "debug.h"
#include <assert.h>

/*{

typedef struct AvailableColumnsPanel_ {
   Panel super;
   Panel* columns;

   Settings* settings;
   ScreenManager* scr;
} AvailableColumnsPanel;

}*/

static void AvailableColumnsPanel_delete(Object* object) {
   Panel* super = (Panel*) object;
   AvailableColumnsPanel* this = (AvailableColumnsPanel*) object;
   Panel_done(super);
   free(this);
}

static HandlerResult AvailableColumnsPanel_eventHandler(Panel* super, int ch) {
   AvailableColumnsPanel* this = (AvailableColumnsPanel*) super;
   char* text = ((ListItem*) Panel_getSelected(super))->value;
   HandlerResult result = IGNORED;

   switch(ch) {
      case 'H':         /* vi: match AvailableMeters binding */
      case ' ':         /* Selection */
      case 0x0a:        /* \n */
      case 0x0d:        /* \r */
      case KEY_ENTER:
      {
         int at = Panel_getSelectedIndex(this->columns);
         Panel_insert(this->columns, at, (Object*) ListItem_new(text, 0));
         Panel_setSelected(this->columns, at+1);
         ColumnsPanel_update(this->columns);
         result = HANDLED;
         break;
      }
   }
   return result;
}

AvailableColumnsPanel* AvailableColumnsPanel_new(Settings* settings, Panel* columns, ScreenManager* scr) {
   AvailableColumnsPanel* this = (AvailableColumnsPanel*) malloc(sizeof(AvailableColumnsPanel));
   Panel* super = (Panel*) this;
   Panel_init(super, 1, 1, 1, 1, LISTITEM_CLASS, true);
   ((Object*)this)->delete = AvailableColumnsPanel_delete;
   
   this->settings = settings;
   this->scr = scr;
   super->eventHandler = AvailableColumnsPanel_eventHandler;

   Panel_setHeader(super, "Available Columns");

   for (int i = 1; i < LAST_PROCESSFIELD; i++) {
      if (i != COMM)
         Panel_add(super, (Object*) ListItem_new(Process_fieldNames[i], 0));
   }
   this->columns = columns;
   return this;
}
