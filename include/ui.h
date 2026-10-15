#pragma once
#include "reader.h"
typedef enum {UI_HOME,UI_DISPLAY,UI_FONT,UI_SETTINGS,UI_LIBRARY,UI_JUMP,UI_FOLDER} UiView;
typedef enum {
 ACT_NONE,ACT_BACK,ACT_SLEEP,ACT_DISPLAY,ACT_FONT,ACT_SETTINGS,ACT_LIBRARY,
 ACT_BRIGHTNESS,ACT_THEME,ACT_FONT_CHANGE,ACT_SIZE,ACT_LEADING,ACT_TRACKING,
 ACT_BIONIC,ACT_FOCUS,ACT_STRENGTH,ACT_FOCUS_MOVE,ACT_SLEEP_TIME,
 ACT_JUMP,ACT_BOOKMARK,ACT_JUMP_MODE,ACT_JUMP_STEP,ACT_JUMP_CONFIRM,
 ACT_BOOKMARK_GO,ACT_FILTER,ACT_SCAN,ACT_ENTRY,ACT_LIST_PAGE,ACT_PARENT,ACT_PAGE_INPUT,
 ACT_LANGUAGE,ACT_CHOOSE_FOLDER,ACT_USE_FOLDER,ACT_RESUME
} UiAction;
typedef struct {
 int x,y,w,h;
 UiAction action;
 int value,style;
 bool active,enabled;
 char label[112],sub[112];
} UiControl;
typedef struct {UiControl controls[40];int count;} UiControls;
typedef struct {
 UiView view;
 UiView settingsReturn;
 int language,selected;
 ReaderOptions options;
 int focusLine,chapterCount,currentChapter,currentPage,pageCount;
 int jumpMode,jumpValue,filter,listPage,listCount;
 bool hasBook,hasBookmark,bookmarkHere,libraryReady,sleeping;
} UiState;
void ui_build(const UiState *state,UiControls *out);
int ui_hit(const UiControls *controls,int x,int y);
/* Returns true only when typography changes and layout must be rebuilt. */
bool ui_change(UiState *state,UiAction action,int value);
/* Returns true when this input was consumed solely to wake the screen. */
bool ui_wake(UiState *state,bool input);
void ui_move_entry(UiState *state,int delta);
