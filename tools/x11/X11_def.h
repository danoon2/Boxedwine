#define CALL_0_R(index) __asm__("push %0\n\tint $0x9b\n\taddl $4, %%esp"::"i"(index):"%eax"); 
#define CALL_1_R(index, arg1) __asm__("push %1\n\tpush %0\n\tint $0x9b\n\taddl $8, %%esp"::"i"(index), "g"(arg1):"%eax"); 
#define CALL_2_R(index, arg1, arg2) __asm__("push %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $12, %%esp"::"i"(index), "g"(arg1), "g"(arg2):"%eax"); 
#define CALL_3_R(index, arg1, arg2, arg3) __asm__("push %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $16, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3):"%eax"); 
#define CALL_4_R(index, arg1, arg2, arg3, arg4) __asm__("push %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $20, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4):"%eax"); 
#define CALL_5_R(index, arg1, arg2, arg3, arg4, arg5) __asm__("push %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $24, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5):"%eax");
#define CALL_6_R(index, arg1, arg2, arg3, arg4, arg5, arg6) __asm__("push %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $28, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6):"%eax");
#define CALL_7_R(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7) __asm__("push %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $32, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7):"%eax");
#define CALL_8_R(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) __asm__("push %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $36, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8):"%eax");
#define CALL_9_R(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) __asm__("push %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $40, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9):"%eax");
#define CALL_10_R(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) __asm__("push %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $44, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10):"%eax");
#define CALL_11_R(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11) __asm__("push %11\n\tpush %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $48, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10), "g"(arg11):"%eax");
#define CALL_12_R(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12) __asm__("push %12\n\tpush %11\n\tpush %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $52, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10), "g"(arg11), "g"(arg12):"%eax");

#define CALL_0(index) __asm__("push %0\n\tint $0x9b\n\taddl $4, %%esp"::"i"(index)); 
#define CALL_1(index, arg1) __asm__("push %1\n\tpush %0\n\tint $0x9b\n\taddl $8, %%esp"::"i"(index), "g"(arg1)); 
#define CALL_2(index, arg1, arg2) __asm__("push %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $12, %%esp"::"i"(index), "g"(arg1), "g"(arg2)); 
#define CALL_3(index, arg1, arg2, arg3) __asm__("push %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $16, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3)); 
#define CALL_4(index, arg1, arg2, arg3, arg4) __asm__("push %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $20, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4)); 
#define CALL_5(index, arg1, arg2, arg3, arg4, arg5) __asm__("push %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $24, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5));
#define CALL_6(index, arg1, arg2, arg3, arg4, arg5, arg6) __asm__("push %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $28, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6));
#define CALL_7(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7) __asm__("push %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $32, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7));
#define CALL_8(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) __asm__("push %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $36, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8));
#define CALL_9(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) __asm__("push %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $40, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9));
#define CALL_10(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) __asm__("push %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $44, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10));
#define CALL_11(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11) __asm__("push %11\n\tpush %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $48, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10), "g"(arg11));
#define CALL_12(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12) __asm__("push %12\n\tpush %11\n\tpush %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $52, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10), "g"(arg11), "g"(arg12));
#define CALL_13(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13) __asm__("push %13\n\tpush %12\n\tpush %11\n\tpush %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $56, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10), "g"(arg11), "g"(arg12), "g"(arg13));
#define CALL_14(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14) __asm__("push %14\n\tpush %13\n\tpush %12\n\tpush %11\n\tpush %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $60, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10), "g"(arg11), "g"(arg12), "g"(arg13), "g"(arg14));
#define CALL_15(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15) __asm__("push %15\n\tpush %14\n\tpush %13\n\tpush %12\n\tpush %11\n\tpush %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9b\n\taddl $64, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10), "g"(arg11), "g"(arg12), "g"(arg13), "g"(arg14), "g"(arg15));

#define X11_OPEN_DISPLAY 0
#define X11_CLOSE_DISPLAY 1
#define X11_GRAB_SERVER 2
#define X11_UNGRAB_SERVER 3
#define X11_INIT_THREADS 4
#define X11_CLEAR_AREA 5
#define X11_SYNC 6
#define X11_CREATE_WINDOW 7
#define X11_TRANSLATE_COORDINATES 8
#define X11_DESTROY_WINDOW 9
#define X11_REPARENT_WINDOW 10
#define X11_QUERY_TREE 11
#define X11_CHANGE_WINDOW_ATTRIBUTES 12
#define X11_CONFIGURE_WINDOW 13
#define X11_SET_INPUT_FOCUS 14
#define X11_SELECT_INPUT 15
#define X11_FIND_CONTEXT 16
#define X11_SAVE_CONTEXT 17
#define X11_DELETE_CONTEXT 18
#define X11_GET_INPUT_FOCUS 19
#define X11_FREE_FONT 20
#define X11_MOVE_RESIZE_WINDOW 21
#define X11_MAP_WINDOW 22
#define X11_UNMAP_WINDOW 23
#define X11_GRAB_POINTER 24
#define X11_UNGRAB_POINTER 25
#define X11_WARP_POINTER 26
#define X11_QUERY_POINTER 27
#define X11_MB_TEXT_LIST_TO_TEXT_PROPERTY 28
#define X11_SET_TEXT_PROPERTY 29
#define X11_SET_SELECTION_OWNER 30
#define X11_GET_SELECTION_OWNER 31
#define X11_CHECK_IF_EVENT 32
#define X11_SEND_EVENT 33
#define X11_FILTER_EVENT 34
#define X11_LOOKUP_STRING 35
#define X11_MB_LOOKUP_STRING 36
#define X11_KEYSYM_TO_STRING 37
#define X11_KB_TRANSLATE_KEYSYM 38
#define X11_LOOKUP_KEYSYM 39
#define X11_GET_KEYBOARD_MAPPING 40
#define X11_FREE_MODIFIER_MAP 41
#define X11_KB_KEYCODE_TO_KEYSYM 42
#define X11_DISPLAY_KEYCODES 43
#define X11_GET_MODIFIER_MAPPING 44
#define X11_REFRESH_KEYBOARD_MAPPING 45
#define X11_BELL 46
#define X11_GET_WINDOW_PROPERTY 47
#define X11_DELETE_PROPERTY 48
#define X11_CONVERT_SELECTION 49
#define X11_CHECK_TYPED_WINDOW_EVENT 50
#define X11_GET_GEOMETRY 51
#define X11_INTERN_ATOMS 52
#define X11_GET_ATOM_NAMES 53
#define X11_CREATE_COLORMAP 54
#define X11_FREE_COLORMAP 55
#define X11_FREE_COLORS 56
#define X11_QUERY_COLORS 57
#define X11_ALLOC_COLOR 58
#define X11_ALLOC_COLOR_CELLS 59
#define X11_GET_VISUAL_INFO 60
#define X11_LIST_PIXEL_FORMATS 61
#define X11_LOCK_DISPLAY 62
#define X11_UNLOCK_DISPLAY 63
#define X11_COPY_AREA 64
#define X11_GET_IMAGE 66
#define X11_PUT_IMAGE 67
#define X11_DESTROY_IMAGE 68
#define X11_GET_PIXEL 69
#define X11_PUT_PIXEL 70
#define X11_CREATE_PIXMAP 71
#define X11_CREATE_BITMAP_FROM_DATA 72
#define X11_FREE_PIXMAP 73
#define X11_CREATE_PIXMAP_CURSOR 74
#define X11_CREATE_FONT_CURSOR 75
#define X11_DEFINE_CURSOR 76
#define X11_FREE_CURSOR 77
#define X11_SET_FUNCTION 78
#define X11_SET_BACKGROUND 79
#define X11_SET_FOREGROUND 80
#define X11_COPY_PLANE 81
#define X11_CREATE_GC 82
#define X11_SET_DASHES 83
#define X11_DRAW_LINE 84
#define X11_DRAW_LINES 85
#define X11_SET_ARC_MODE 86
#define X11_FILL_ARC 87
#define X11_DRAW_ARC 88
#define X11_DRAW_RECTANGLE 89
#define X11_FILL_RECTANGLE 90
#define X11_FILL_RECTANGLES 91
#define X11_DRAW_POINT 92
#define X11_FILL_POLYGON 93
#define X11_CHANGE_GC 94
#define X11_FREE_GC 95
#define X11_SET_SUBWINDOW_MODE 96
#define X11_SET_GRAPHICS_EXPOSURES 97
#define X11_SET_FILL_STYLE 98
#define X11_FREE 99
#define X11_SET_CLIP_MASK 100
#define X11_SET_CLIP_RECTANGLES 101
#define X11_SET_TRANSIENT_FOR_HINT 102
#define X11_ALLOC_WM_HINTS 103
#define X11_ALLOC_CLASS_HINT 104
#define X11_SET_CLASS_HINT 105
#define X11_SET_WM_NAME 106
#define X11_SET_WM_ICON_NAME 107
#define X11_SET_WM_NORMAL_HINTS 108
#define X11_SET_WM_PROPERTIES 109
#define X11_RECONFIGURE_WM_WINDOW 110
#define X11_VA_CREATE_NESTED_LIST 111
#define X11_UNSET_IC_FOCUS 112
#define X11_SET_IC_FOCUS 113
#define X11_DESTROY_IC 114
#define X11_SET_IC_VALUES 115
#define X11_MB_RESET_IC 116
#define X11_SET_LOCALE_MODIFIERS 117
#define X11_OPEN_IM 118
#define X11_CLOSE_IM 119
#define X11_SET_IM_VALUES 120
#define X11_GET_IM_VALUES 121
#define X11_DISPLAY_OF_IM 122
#define X11_UNREGISTER_IM_INSTANTIATE_CALLBACK 123
#define X11_REGISTER_IM_INSTANTIATE_CALLBACK 124
#define X11_FREE_STRING_LIST 125
#define X11_ALLOC_SIZE_HINTS 126
#define X11_CHANGE_PROPERTY 127
#define X11_CREATE_FONT_SET 128
#define X11_FREE_FONT_SET 129
#define X11_CREATE_IC 130
#define X11_CREATE_IMAGE 131
#define X11_DISPLAY_NAME 132
#define X11_GET_ATOM_NAME 133
#define X11_GET_DEFAULT 134
#define X11_GET_WINDOW_ATTRIBUTES 135
#define X11_ICONIFY_WINDOW 136
#define X11_INTERN_ATOM 137
#define X11_KEYSYM_TO_KEYCODE 138
#define X11_LOCALE_OF_IM 139
#define X11_MATCH_VISUAL_INFO 140
#define X11_QUERY_COLOR 141
#define X11_QUERY_EXTENSION 142
#define X11_SET_WM_HINTS 143
#define X11_SHAPE_COMBINE_MASK 144
#define X11_SHAPE_COMBINE_RECTANGLES 145
#define X11_SHAPE_OFFSET_SHAPE 146
#define X11_SHM_ATTACH 147
#define X11_SHM_CREATE_IMAGE 148
#define X11_SHM_DETACH 149
#define X11_SHM_PUT_IMAGE 150
#define X11_STORE_COLOR 151
#define X11_WINDOW_EVENT 152
#define X11_WITHDRAW_WINDOW 153
#define X11_MB_TEXT_PROPERTY_TO_TEXT_LIST 154
#define X11_RM_UNIQUE_QUARK 155
#define X11_COUNT 156