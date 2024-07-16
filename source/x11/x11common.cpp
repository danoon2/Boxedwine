/*
 *  Copyright (C) 2024  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "boxedwine.h"
#include "../../../tools/x11/X11_def.h"
#include "x11.h"

Int99Callback int9BCallback[X11_COUNT];
U32 int9BCallbackSize = X11_COUNT;

#define Success 0

// Display* XOpenDisplay(const char* displayName) 
static void x11_OpenDisplay(CPU* cpu) {
    // winex11 always pass null for displayName
}

static void x11_CloseDisplay(CPU* cpu) {
}

static void x11_GrabServer(CPU* cpu) {
}

static void x11_UnGrabServer(CPU* cpu) {
}

// Status XInitThreads()
static void x11_InitThread(CPU* cpu) {
    EAX = Success;
}

static void x11_ClearArea(CPU* cpu) {
}

static void x11_Sync(CPU* cpu) {
}

static void x11_CreateWindow(CPU* cpu) {
}

static void x11_TranslateCoordinates(CPU* cpu) {
}

static void x11_DestroyWindow(CPU* cpu) {
}

static void x11_ReparentWindow(CPU* cpu) {
}

static void x11_QueryTree(CPU* cpu) {
}

static void x11_ChangeWindowAttributes(CPU* cpu) {
}

static void x11_ConfigureWindow(CPU* cpu) {
}

static void x11_SetInputFocus(CPU* cpu) {
}

static void x11_SelectInput(CPU* cpu) {
}

static void x11_FindContext(CPU* cpu) {
}

static void x11_SaveContext(CPU* cpu) {
}

static void x11_DeleteContext(CPU* cpu) {
}

static void x11_GetInputFocus(CPU* cpu) {
}

static void x11_FreeFont(CPU* cpu) {
}

static void x11_MoveResizeWindow(CPU* cpu) {
}

static void x11_MapWindow(CPU* cpu) {
}

static void x11_UnmapWindow(CPU* cpu) {
}

static void x11_GrabPointer(CPU* cpu) {
}

static void x11_UngrabPointer(CPU* cpu) {
}

static void x11_WarpPointer(CPU* cpu) {
}

static void x11_QueryPointer(CPU* cpu) {
}

static void x11_MbTextListToTextProperty(CPU* cpu) {
}

static void x11_SetTextProperty(CPU* cpu) {
}

static void x11_SetSelectionOwner(CPU* cpu) {
}

static void x11_GetSelectionOwner(CPU* cpu) {
}

static void x11_CheckIfEvent(CPU* cpu) {
}

static void x11_SendEvent(CPU* cpu) {
}

static void x11_FilterEvent(CPU* cpu) {
}

static void x11_LookupString(CPU* cpu) {
}

static void x11_MbLookupString(CPU* cpu) {
}

static void x11_KeysymToString(CPU* cpu) {
}

static void x11_KbTranslateKeysym(CPU* cpu) {
}

static void x11_LookupKeysym(CPU* cpu) {
}

static void x11_GetKeyboardMapping(CPU* cpu) {
}

static void x11_FreeModifierMap(CPU* cpu) {
}

static void x11_KbKeycodeToKeysym(CPU* cpu) {
}

static void x11_DisplayKeycodes(CPU* cpu) {
}

static void x11_GetModifierMapping(CPU* cpu) {
}

static void x11_RefreshKeyboardMapping(CPU* cpu) {
}

static void x11_Bell(CPU* cpu) {
}

static void x11_GetWindowProperty(CPU* cpu) {
}

static void x11_DeleteProperty(CPU* cpu) {
}

static void x11_ConvertSelection(CPU* cpu) {
}

static void x11_CheckTypedWindowEvent(CPU* cpu) {
}

static void x11_GetGeometry(CPU* cpu) {
}

static void x11_InternAtoms(CPU* cpu) {
}

static void x11_GetAtomNames(CPU* cpu) {
}

static void x11_CreateColorMap(CPU* cpu) {
}

static void x11_FreeColorMap(CPU* cpu) {
}

static void x11_FreeColors(CPU* cpu) {
}

static void x11_QueryColors(CPU* cpu) {
}

static void x11_AllocColor(CPU* cpu) {
}

static void x11_AllocColorCells(CPU* cpu) {
}

static void x11_GetVisualInfo(CPU* cpu) {
}

static void x11_ListPixelFormats(CPU* cpu) {
}

static void x11_LockDisplay(CPU* cpu) {
}

static void x11_UnlockDisplay(CPU* cpu) {
}

static void x11_CopyArea(CPU* cpu) {
}

static void x11_GetImage(CPU* cpu) {
}

static void x11_PutImage(CPU* cpu) {
}

static void x11_DestroyImage(CPU* cpu) {
}

static void x11_GetPixel(CPU* cpu) {
}

static void x11_PutPixel(CPU* cpu) {
}

static void x11_CreatePixmap(CPU* cpu) {
}

static void x11_CreateBitmapFromData(CPU* cpu) {
}

static void x11_FreePixmap(CPU* cpu) {
}

static void x11_CreatePixmapCursor(CPU* cpu) {
}

static void x11_CreateFontCursor(CPU* cpu) {
}

static void x11_DefineCursor(CPU* cpu) {
}

static void x11_FreeCursor(CPU* cpu) {
}

static void x11_SetFunction(CPU* cpu) {
}

static void x11_SetBackground(CPU* cpu) {
}

static void x11_SetForeground(CPU* cpu) {
}

static void x11_CopyPlane(CPU* cpu) {
}

static void x11_CreateGC(CPU* cpu) {
}

static void x11_SetDashes(CPU* cpu) {
}

static void x11_DrawLine(CPU* cpu) {
}

static void x11_DrawLines(CPU* cpu) {
}

static void x11_SetArcMode(CPU* cpu) {
}

static void x11_FillArc(CPU* cpu) {
}

static void x11_DrawArc(CPU* cpu) {
}

static void x11_DrawRectangle(CPU* cpu) {
}

static void x11_FillRectangle(CPU* cpu) {
}

static void x11_FillRectangles(CPU* cpu) {
}

static void x11_DrawPoint(CPU* cpu) {
}

static void x11_FillPolygon(CPU* cpu) {
}

static void x11_ChangeGC(CPU* cpu) {
}

static void x11_FreeGC(CPU* cpu) {
}

static void x11_SetSubwindowMode(CPU* cpu) {
}

static void x11_SetGraphicsExposures(CPU* cpu) {
}

static void x11_SetFillStyle(CPU* cpu) {
}

static void x11_Free(CPU* cpu) {
}

static void x11_SetClipMask(CPU* cpu) {
}

static void x11_SetClipRectangles(CPU* cpu) {
}

static void x11_SetTransientForHint(CPU* cpu) {
}

static void x11_AllocWMHints(CPU* cpu) {
}

static void x11_AllocClassHint(CPU* cpu) {
}

static void x11_SetClasHint(CPU* cpu) {
}

static void x11_SetWMName(CPU* cpu) {
}

static void x11_SetWMIconName(CPU* cpu) {
}

static void x11_SetWMNormalHints(CPU* cpu) {
}

static void x11_SetWMProperties(CPU* cpu) {
}

static void x11_ReconfigureWMWindow(CPU* cpu) {
}

static void x11_VaCreateNestedList(CPU* cpu) {
}

static void x11_UnsetICFocus(CPU* cpu) {
}

static void x11_SetICFocus(CPU* cpu) {
}

static void x11_DestroyIC(CPU* cpu) {
}

static void x11_SetICValues(CPU* cpu) {
}

static void x11_MbResetIC(CPU* cpu) {
}

static void x11_SetLocaleModifiers(CPU* cpu) {
}

static void x11_OpenIM(CPU* cpu) {
}

static void x11_CloseIM(CPU* cpu) {
}

static void x11_SetIMValues(CPU* cpu) {
}

static void x11_GetIMValues(CPU* cpu) {
}

static void x11_DisplayOfIM(CPU* cpu) {
}

static void x11_UnregisterIMInstantiateCallback(CPU* cpu) {
}

static void x11_RegisterIMInstantiateCallback(CPU* cpu) {
}

static void x11_FreeStringList(CPU* cpu) {
}

static void x11_AllocSizeHints(CPU* cpu) {
}

static void x11_ChangeProperty(CPU* cpu) {
}

static void x11_CreateFontSet(CPU* cpu) {
}

static void x11_FreeFontSet(CPU* cpu) {
}

static void x11_CreateIC(CPU* cpu) {
}

static void x11_CreateImage(CPU* cpu) {
}

static void x11_DisplayName(CPU* cpu) {
}

static void x11_GetAtomName(CPU* cpu) {
}

static void x11_GetDefault(CPU* cpu) {
}

static void x11_GetWindowAttributes(CPU* cpu) {
}

static void x11_IconifyWindow(CPU* cpu) {
}

static void x11_InternAtom(CPU* cpu) {
}

static void x11_KeysymToKeycode(CPU* cpu) {
}

static void x11_LocaleOfIM(CPU* cpu) {
}

static void x11_MatchVisualInfo(CPU* cpu) {
}

static void x11_QueryColor(CPU* cpu) {
}

static void x11_QueryExtension(CPU* cpu) {
}

static void x11_SetWMHints(CPU* cpu) {
}

static void x11_ShapeCombineMask(CPU* cpu) {
}

static void x11_ShapeCombineRectangles(CPU* cpu) {
}

static void x11_ShapeOffsetShape(CPU* cpu) {
}

static void x11_ShmAttach(CPU* cpu) {
}

static void x11_ShmCreateImage(CPU* cpu) {
}

static void x11_ShmDetach(CPU* cpu) {
}

static void x11_ShmPutImage(CPU* cpu) {
}

static void x11_StoreColor(CPU* cpu) {
}

static void x11_WindowEvent(CPU* cpu) {
}

static void x11_WithDrawWindow(CPU* cpu) {
}

static void x11_MbTextPropertyToTextList(CPU* cpu) {
}

static void x11_RmUniqueQuark(CPU* cpu) {
}

void x11_init() {
    int9BCallbackSize = X11_COUNT;   

    int9BCallback[X11_OPEN_DISPLAY] = x11_OpenDisplay;
    int9BCallback[X11_CLOSE_DISPLAY] = x11_CloseDisplay;
    int9BCallback[X11_GRAB_SERVER] = x11_GrabServer;
    int9BCallback[X11_UNGRAB_SERVER] = x11_UnGrabServer;
    int9BCallback[X11_INIT_THREADS] = x11_InitThread;
    int9BCallback[X11_CLEAR_AREA] = x11_ClearArea;
    int9BCallback[X11_SYNC] = x11_Sync;
    int9BCallback[X11_CREATE_WINDOW] = x11_CreateWindow;
    int9BCallback[X11_TRANSLATE_COORDINATES] = x11_TranslateCoordinates;
    int9BCallback[X11_DESTROY_WINDOW] = x11_DestroyWindow;
    int9BCallback[X11_REPARENT_WINDOW] = x11_ReparentWindow;
    int9BCallback[X11_QUERY_TREE] = x11_QueryTree;
    int9BCallback[X11_CHANGE_WINDOW_ATTRIBUTES] = x11_ChangeWindowAttributes;
    int9BCallback[X11_CONFIGURE_WINDOW] = x11_ConfigureWindow;
    int9BCallback[X11_SET_INPUT_FOCUS] = x11_SetInputFocus;
    int9BCallback[X11_SELECT_INPUT] = x11_SelectInput;
    int9BCallback[X11_FIND_CONTEXT] = x11_FindContext;
    int9BCallback[X11_SAVE_CONTEXT] = x11_SaveContext;
    int9BCallback[X11_DELETE_CONTEXT] = x11_DeleteContext;
    int9BCallback[X11_GET_INPUT_FOCUS] = x11_GetInputFocus;
    int9BCallback[X11_FREE_FONT] = x11_FreeFont;
    int9BCallback[X11_MOVE_RESIZE_WINDOW] = x11_MoveResizeWindow;
    int9BCallback[X11_MAP_WINDOW] = x11_MapWindow;
    int9BCallback[X11_UNMAP_WINDOW] = x11_UnmapWindow;
    int9BCallback[X11_GRAB_POINTER] = x11_GrabPointer;
    int9BCallback[X11_UNGRAB_POINTER] = x11_UngrabPointer;
    int9BCallback[X11_WARP_POINTER] = x11_WarpPointer;
    int9BCallback[X11_QUERY_POINTER] = x11_QueryPointer;
    int9BCallback[X11_MB_TEXT_LIST_TO_TEXT_PROPERTY] = x11_MbTextListToTextProperty;
    int9BCallback[X11_SET_TEXT_PROPERTY] = x11_SetTextProperty;
    int9BCallback[X11_SET_SELECTION_OWNER] = x11_SetSelectionOwner;
    int9BCallback[X11_GET_SELECTION_OWNER] = x11_GetSelectionOwner;
    int9BCallback[X11_CHECK_IF_EVENT] = x11_CheckIfEvent;
    int9BCallback[X11_SEND_EVENT] = x11_SendEvent;
    int9BCallback[X11_FILTER_EVENT] = x11_FilterEvent;
    int9BCallback[X11_LOOKUP_STRING] = x11_LookupString;
    int9BCallback[X11_MB_LOOKUP_STRING] = x11_MbLookupString;
    int9BCallback[X11_KEYSYM_TO_STRING] = x11_KeysymToString;
    int9BCallback[X11_KB_TRANSLATE_KEYSYM] = x11_KbTranslateKeysym;
    int9BCallback[X11_LOOKUP_KEYSYM] = x11_LookupKeysym;
    int9BCallback[X11_GET_KEYBOARD_MAPPING] = x11_GetKeyboardMapping;
    int9BCallback[X11_FREE_MODIFIER_MAP] = x11_FreeModifierMap;
    int9BCallback[X11_KB_KEYCODE_TO_KEYSYM] = x11_KbKeycodeToKeysym;
    int9BCallback[X11_DISPLAY_KEYCODES] = x11_DisplayKeycodes;
    int9BCallback[X11_GET_MODIFIER_MAPPING] = x11_GetModifierMapping;
    int9BCallback[X11_REFRESH_KEYBOARD_MAPPING] = x11_RefreshKeyboardMapping;
    int9BCallback[X11_BELL] = x11_Bell;
    int9BCallback[X11_GET_WINDOW_PROPERTY] = x11_GetWindowProperty;
    int9BCallback[X11_DELETE_PROPERTY] = x11_DeleteProperty;
    int9BCallback[X11_CONVERT_SELECTION] = x11_ConvertSelection;
    int9BCallback[X11_CHECK_TYPED_WINDOW_EVENT] = x11_CheckTypedWindowEvent;
    int9BCallback[X11_GET_GEOMETRY] = x11_GetGeometry;
    int9BCallback[X11_INTERN_ATOMS] = x11_InternAtoms;
    int9BCallback[X11_GET_ATOM_NAMES] = x11_GetAtomNames;
    int9BCallback[X11_CREATE_COLORMAP] = x11_CreateColorMap;
    int9BCallback[X11_FREE_COLORMAP] = x11_FreeColorMap;
    int9BCallback[X11_FREE_COLORS] = x11_FreeColors;
    int9BCallback[X11_QUERY_COLORS] = x11_QueryColors;
    int9BCallback[X11_ALLOC_COLOR] = x11_AllocColor;
    int9BCallback[X11_ALLOC_COLOR_CELLS] = x11_AllocColorCells;
    int9BCallback[X11_GET_VISUAL_INFO] = x11_GetVisualInfo;
    int9BCallback[X11_LIST_PIXEL_FORMATS] = x11_ListPixelFormats;
    int9BCallback[X11_LOCK_DISPLAY] = x11_LockDisplay;
    int9BCallback[X11_UNLOCK_DISPLAY] = x11_UnlockDisplay;
    int9BCallback[X11_COPY_AREA] = x11_CopyArea;
    int9BCallback[X11_GET_IMAGE] = x11_GetImage;
    int9BCallback[X11_PUT_IMAGE] = x11_PutImage;
    int9BCallback[X11_DESTROY_IMAGE] = x11_DestroyImage;
    int9BCallback[X11_GET_PIXEL] = x11_GetPixel;
    int9BCallback[X11_PUT_PIXEL] = x11_PutPixel;
    int9BCallback[X11_CREATE_PIXMAP] = x11_CreatePixmap;
    int9BCallback[X11_CREATE_BITMAP_FROM_DATA] = x11_CreateBitmapFromData;
    int9BCallback[X11_FREE_PIXMAP] = x11_FreePixmap;
    int9BCallback[X11_CREATE_PIXMAP_CURSOR] = x11_CreatePixmapCursor;
    int9BCallback[X11_CREATE_FONT_CURSOR] = x11_CreateFontCursor;
    int9BCallback[X11_DEFINE_CURSOR] = x11_DefineCursor;
    int9BCallback[X11_FREE_CURSOR] = x11_FreeCursor;
    int9BCallback[X11_SET_FUNCTION] = x11_SetFunction;
    int9BCallback[X11_SET_BACKGROUND] = x11_SetBackground;
    int9BCallback[X11_SET_FOREGROUND] = x11_SetForeground;
    int9BCallback[X11_COPY_PLANE] = x11_CopyPlane;
    int9BCallback[X11_CREATE_GC] = x11_CreateGC;
    int9BCallback[X11_SET_DASHES] = x11_SetDashes;
    int9BCallback[X11_DRAW_LINE] = x11_DrawLine;
    int9BCallback[X11_DRAW_LINES] = x11_DrawLines;
    int9BCallback[X11_SET_ARC_MODE] = x11_SetArcMode;
    int9BCallback[X11_FILL_ARC] = x11_FillArc;
    int9BCallback[X11_DRAW_ARC] = x11_DrawArc;
    int9BCallback[X11_DRAW_RECTANGLE] = x11_DrawRectangle;
    int9BCallback[X11_FILL_RECTANGLE] = x11_FillRectangle;
    int9BCallback[X11_FILL_RECTANGLES] = x11_FillRectangles;
    int9BCallback[X11_DRAW_POINT] = x11_DrawPoint;
    int9BCallback[X11_FILL_POLYGON] = x11_FillPolygon;
    int9BCallback[X11_CHANGE_GC] = x11_ChangeGC;
    int9BCallback[X11_FREE_GC] = x11_FreeGC;
    int9BCallback[X11_SET_SUBWINDOW_MODE] = x11_SetSubwindowMode;
    int9BCallback[X11_SET_GRAPHICS_EXPOSURES] = x11_SetGraphicsExposures;
    int9BCallback[X11_SET_FILL_STYLE] = x11_SetFillStyle;
    int9BCallback[X11_FREE] = x11_Free;
    int9BCallback[X11_SET_CLIP_MASK] = x11_SetClipMask;
    int9BCallback[X11_SET_CLIP_RECTANGLES] = x11_SetClipRectangles;
    int9BCallback[X11_SET_TRANSIENT_FOR_HINT] = x11_SetTransientForHint;
    int9BCallback[X11_ALLOC_WM_HINTS] = x11_AllocWMHints;
    int9BCallback[X11_ALLOC_CLASS_HINT] = x11_AllocClassHint;
    int9BCallback[X11_SET_CLASS_HINT] = x11_SetClasHint;
    int9BCallback[X11_SET_WM_NAME] = x11_SetWMName;
    int9BCallback[X11_SET_WM_ICON_NAME] = x11_SetWMIconName;
    int9BCallback[X11_SET_WM_NORMAL_HINTS] = x11_SetWMNormalHints;
    int9BCallback[X11_SET_WM_PROPERTIES] = x11_SetWMProperties;
    int9BCallback[X11_RECONFIGURE_WM_WINDOW] = x11_ReconfigureWMWindow;
    int9BCallback[X11_VA_CREATE_NESTED_LIST] = x11_VaCreateNestedList;
    int9BCallback[X11_UNSET_IC_FOCUS] = x11_UnsetICFocus;
    int9BCallback[X11_SET_IC_FOCUS] = x11_SetICFocus;
    int9BCallback[X11_DESTROY_IC] = x11_DestroyIC;
    int9BCallback[X11_SET_IC_VALUES] = x11_SetICValues;
    int9BCallback[X11_MB_RESET_IC] = x11_MbResetIC;
    int9BCallback[X11_SET_LOCALE_MODIFIERS] = x11_SetLocaleModifiers;
    int9BCallback[X11_OPEN_IM] = x11_OpenIM;
    int9BCallback[X11_CLOSE_IM] = x11_CloseIM;
    int9BCallback[X11_SET_IM_VALUES] = x11_SetIMValues;
    int9BCallback[X11_GET_IM_VALUES] = x11_GetIMValues;
    int9BCallback[X11_DISPLAY_OF_IM] = x11_DisplayOfIM;
    int9BCallback[X11_UNREGISTER_IM_INSTANTIATE_CALLBACK] = x11_UnregisterIMInstantiateCallback;
    int9BCallback[X11_REGISTER_IM_INSTANTIATE_CALLBACK] = x11_RegisterIMInstantiateCallback;
    int9BCallback[X11_FREE_STRING_LIST] = x11_FreeStringList;
    int9BCallback[X11_ALLOC_SIZE_HINTS] = x11_AllocSizeHints;
    int9BCallback[X11_CHANGE_PROPERTY] = x11_ChangeProperty;
    int9BCallback[X11_CREATE_FONT_SET] = x11_CreateFontSet;
    int9BCallback[X11_FREE_FONT_SET] = x11_FreeFontSet;
    int9BCallback[X11_CREATE_IC] = x11_CreateIC;
    int9BCallback[X11_CREATE_IMAGE] = x11_CreateImage;
    int9BCallback[X11_DISPLAY_NAME] = x11_DisplayName;
    int9BCallback[X11_GET_ATOM_NAME] = x11_GetAtomName;
    int9BCallback[X11_GET_DEFAULT] = x11_GetDefault;
    int9BCallback[X11_GET_WINDOW_ATTRIBUTES] = x11_GetWindowAttributes;
    int9BCallback[X11_ICONIFY_WINDOW] = x11_IconifyWindow;
    int9BCallback[X11_INTERN_ATOM] = x11_InternAtom;
    int9BCallback[X11_KEYSYM_TO_KEYCODE] = x11_KeysymToKeycode;
    int9BCallback[X11_LOCALE_OF_IM] = x11_LocaleOfIM;
    int9BCallback[X11_MATCH_VISUAL_INFO] = x11_MatchVisualInfo;
    int9BCallback[X11_QUERY_COLOR] = x11_QueryColor;
    int9BCallback[X11_QUERY_EXTENSION] = x11_QueryExtension;
    int9BCallback[X11_SET_WM_HINTS] = x11_SetWMHints;
    int9BCallback[X11_SHAPE_COMBINE_MASK] = x11_ShapeCombineMask;
    int9BCallback[X11_SHAPE_COMBINE_RECTANGLES] = x11_ShapeCombineRectangles;
    int9BCallback[X11_SHAPE_OFFSET_SHAPE] = x11_ShapeOffsetShape;
    int9BCallback[X11_SHM_ATTACH] = x11_ShmAttach;
    int9BCallback[X11_SHM_CREATE_IMAGE] = x11_ShmCreateImage;
    int9BCallback[X11_SHM_DETACH] = x11_ShmDetach;
    int9BCallback[X11_SHM_PUT_IMAGE] = x11_ShmPutImage;
    int9BCallback[X11_STORE_COLOR] = x11_StoreColor;
    int9BCallback[X11_WINDOW_EVENT] = x11_WindowEvent;
    int9BCallback[X11_WITHDRAW_WINDOW] = x11_WithDrawWindow;
    int9BCallback[X11_MB_TEXT_PROPERTY_TO_TEXT_LIST] = x11_MbTextListToTextProperty;
    int9BCallback[X11_RM_UNIQUE_QUARK] = x11_RmUniqueQuark;
}

void callX11(CPU* cpu, U32 index) {
    if (index < int9BCallbackSize) {
        if (int9BCallback[index]) {
            int9BCallback[index](cpu);
        } else {
            kpanic("x11 tried to call missing function: %d", index);
        }
    } else
    {
        kpanic("x11 not compiled into Boxedwine: %d", index);
    }
}