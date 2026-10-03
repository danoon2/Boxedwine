/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __MAC_CURSOR_H__
#define __MAC_CURSOR_H__

// Main-thread only. Supplement SDL's transparent cursor with native hiding,
// owning exactly one hide/unhide pair while the guest needs an invisible cursor.
void macCursorUpdate();
void macCursorReset();

#endif
