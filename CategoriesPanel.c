
#include "CategoriesPanel.h"
#include "AvailableMetersPanel.h"
#include "MetersPanel.h"
#include "DisplayOptionsPanel.h"
#include "ColumnsPanel.h"
#include "ColorsPanel.h"
#include "AvailableColumnsPanel.h"

#include "Panel.h"

#include "debug.h"
#include <assert.h>

/*{

typedef struct CategoriesPanel_ {
   Panel super;

   Settings* settings;
   ScreenManager* scr;
} CategoriesPanel;

}*/

static char* MetersFunctions[] = {"Type " , "MoveDn ", "MoveUp ", "Remove ", "Done ", NULL};
static char* MetersKeys[]      = {"Space ", "J "     , "K "     , "x "     , "q "   , NULL};
static int   MetersEvents[]    = {' '     , 'J'      , 'K'      , 'x'      , 'q'};

static char* AvailableMetersFunctions[] = {"Add to Left ", "Add to Right ", "Done ", NULL};
static char* AvailableMetersKeys[]      = {"H "          , "L "           , "q "   , NULL};
static int   AvailableMetersEvents[]    = {'H'           , 'L'            , 'q'};

static char* DisplayOptionsFunctions[] = {"Toggle ", "Done ", NULL};
static char* DisplayOptionsKeys[]      = {"Space " , "q "   , NULL};
static int   DisplayOptionsEvents[]    = {' '      , 'q'};

static char* ColumnsFunctions[] = {"MoveDn ", "MoveUp ", "Remove ", "Done ", NULL};
static char* ColumnsKeys[]      = {"J "     , "K "     , "x "     , "q "   , NULL};
static int   ColumnsEvents[]    = {'J'      , 'K'      , 'x'      , 'q'};

static char* ColorsFunctions[] = {"Select ", "Done ", NULL};
static char* ColorsKeys[]      = {"Space " , "q "   , NULL};
static int   ColorsEvents[]    = {' '      , 'q'};

static char* AvailableColumnsFunctions[] = {"Add "  , "Done ", NULL};
static char* AvailableColumnsKeys[]      = {"Space ", "q "   , NULL};
static int   AvailableColumnsEvents[]    = {' '     , 'q'};

static void CategoriesPanel_delete(Object* object) {
   Panel* super = (Panel*) object;
   CategoriesPanel* this = (CategoriesPanel*) object;
   Panel_done(super);
   free(this);
}

void CategoriesPanel_makeMetersPage(CategoriesPanel* this) {
   Panel* leftMeters = (Panel*) MetersPanel_new(this->settings, "Left column", this->settings->header->leftMeters, this->scr);
   Panel* rightMeters = (Panel*) MetersPanel_new(this->settings, "Right column", this->settings->header->rightMeters, this->scr);
   Panel* availableMeters = (Panel*) AvailableMetersPanel_new(this->settings, leftMeters, rightMeters, this->scr);
   ScreenManager_add(this->scr, leftMeters, FunctionBar_new(MetersFunctions, MetersKeys, MetersEvents), 20);
   ScreenManager_add(this->scr, rightMeters, FunctionBar_new(MetersFunctions, MetersKeys, MetersEvents), 20);
   ScreenManager_add(this->scr, availableMeters, FunctionBar_new(AvailableMetersFunctions, AvailableMetersKeys, AvailableMetersEvents), -1);
}

static void CategoriesPanel_makeDisplayOptionsPage(CategoriesPanel* this) {
   Panel* displayOptions = (Panel*) DisplayOptionsPanel_new(this->settings, this->scr);
   ScreenManager_add(this->scr, displayOptions, FunctionBar_new(DisplayOptionsFunctions, DisplayOptionsKeys, DisplayOptionsEvents), -1);
}

static void CategoriesPanel_makeColorsPage(CategoriesPanel* this) {
   Panel* colors = (Panel*) ColorsPanel_new(this->settings, this->scr);
   ScreenManager_add(this->scr, colors, FunctionBar_new(ColorsFunctions, ColorsKeys, ColorsEvents), -1);
}

static void CategoriesPanel_makeColumnsPage(CategoriesPanel* this) {
   Panel* columns = (Panel*) ColumnsPanel_new(this->settings, this->scr);
   Panel* availableColumns = (Panel*) AvailableColumnsPanel_new(this->settings, columns, this->scr);
   ScreenManager_add(this->scr, columns, FunctionBar_new(ColumnsFunctions, ColumnsKeys, ColumnsEvents), 20);
   ScreenManager_add(this->scr, availableColumns, FunctionBar_new(AvailableColumnsFunctions, AvailableColumnsKeys, AvailableColumnsEvents), -1);
}

static HandlerResult CategoriesPanel_eventHandler(Panel* super, int ch) {
   CategoriesPanel* this = (CategoriesPanel*) super;

   HandlerResult result = IGNORED;

   int selected = Panel_getSelectedIndex(super);
   switch (ch) {
      case EVENT_SETSELECTED:
         result = HANDLED;
         break;
      case 'k':         /* vi */
      case KEY_UP:
      case 'j':         /* vi */
      case KEY_DOWN:
      case KEY_CTRLF:   /* vi */
      case KEY_NPAGE:
      case KEY_CTRLB:   /* vi */
      case KEY_PPAGE:
      case 'g':         /* vi */
      case KEY_HOME:
      case 'G':         /* vi */
      case KEY_END: {
         int previous = selected;
         Panel_onKey(super, ch);
         selected = Panel_getSelectedIndex(super);
         if (previous != selected)
            result = HANDLED;
         break;
      }
   }

   if (result == HANDLED) {
      int size = ScreenManager_size(this->scr);
      for (int i = 1; i < size; i++)
         ScreenManager_remove(this->scr, 1);
      switch (selected) {
         case 0:
            CategoriesPanel_makeMetersPage(this);
            break;
         case 1:
            CategoriesPanel_makeDisplayOptionsPage(this);
            break;
         case 2:
            CategoriesPanel_makeColorsPage(this);
            break;
         case 3:
            CategoriesPanel_makeColumnsPage(this);
            break;
      }
   }

   return result;
}

CategoriesPanel* CategoriesPanel_new(Settings* settings, ScreenManager* scr) {
   CategoriesPanel* this = (CategoriesPanel*) malloc(sizeof(CategoriesPanel));
   Panel* super = (Panel*) this;
   Panel_init(super, 1, 1, 1, 1, LISTITEM_CLASS, true);
   ((Object*)this)->delete = CategoriesPanel_delete;

   this->settings = settings;
   this->scr = scr;
   super->eventHandler = CategoriesPanel_eventHandler;
   Panel_setHeader(super, "Setup");
   Panel_add(super, (Object*) ListItem_new("Meters", 0));
   Panel_add(super, (Object*) ListItem_new("Display options", 0));
   Panel_add(super, (Object*) ListItem_new("Colors", 0));
   Panel_add(super, (Object*) ListItem_new("Columns", 0));
   return this;
}
