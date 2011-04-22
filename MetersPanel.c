
#include "MetersPanel.h"

#include "Panel.h"
#include "Settings.h"
#include "ScreenManager.h"

#include "debug.h"
#include <assert.h>

/*{

typedef struct MetersPanel_ {
   Panel super;

   Settings* settings;
   Vector* meters;
   ScreenManager* scr;
} MetersPanel;

}*/

static void MetersPanel_delete(Object* object) {
   Panel* super = (Panel*) object;
   MetersPanel* this = (MetersPanel*) object;
   Panel_done(super);
   free(this);
}

static HandlerResult MetersPanel_EventHandler(Panel* super, int ch) {
   MetersPanel* this = (MetersPanel*) super;
   
   int selected = Panel_getSelectedIndex(super);
   HandlerResult result = IGNORED;

   switch(ch) {
      case 0x0a:        /* \n */
      case 0x0d:        /* \r */
      case KEY_ENTER:
      case ' ':
      {
         Meter* meter = (Meter*) Vector_get(this->meters, selected);
         int mode = meter->mode + 1;
         if (mode == LAST_METERMODE) mode = 1;
         Meter_setMode(meter, mode);
         Panel_set(super, selected, (Object*) Meter_toListItem(meter));
         result = HANDLED;
         break;
      }
      case 'K':         /* vi */
      case '[':
      case '-':
      {
         Vector_moveUp(this->meters, selected);
         Panel_moveSelectedUp(super);
         result = HANDLED;
         break;
      }
      case 'J':         /* vi */
      case ']':
      case '+':
      {
         Vector_moveDown(this->meters, selected);
         Panel_moveSelectedDown(super);
         result = HANDLED;
         break;
      }
      case 'x':         /* vi */
      case KEY_DC:      /* BS */
      {
         if (selected < Vector_size(this->meters)) {
            Vector_remove(this->meters, selected);
            Panel_remove(super, selected);
         }
         result = HANDLED;
         break;
      }
   }
   if (result == HANDLED) {
      Header* header = this->settings->header;
      this->settings->changed = true;
      Header_calculateHeight(header);
      Header_draw(header);
      ScreenManager_resize(this->scr, this->scr->x1, header->height, this->scr->x2, this->scr->y2);
   }
   return result;
}

MetersPanel* MetersPanel_new(Settings* settings, char* header, Vector* meters, ScreenManager* scr) {
   MetersPanel* this = (MetersPanel*) malloc(sizeof(MetersPanel));
   Panel* super = (Panel*) this;
   Panel_init(super, 1, 1, 1, 1, LISTITEM_CLASS, true);
   ((Object*)this)->delete = MetersPanel_delete;

   this->settings = settings;
   this->meters = meters;
   this->scr = scr;
   super->eventHandler = MetersPanel_EventHandler;
   Panel_setHeader(super, header);
   for (int i = 0; i < Vector_size(meters); i++) {
      Meter* meter = (Meter*) Vector_get(meters, i);
      Panel_add(super, (Object*) Meter_toListItem(meter));
   }
   return this;
}
