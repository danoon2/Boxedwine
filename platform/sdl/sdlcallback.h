/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
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

#ifndef __SDL_CALLBACK_H__
#define __SDL_CALLBACK_H__

#ifdef BOXEDWINE_MULTI_THREADED

class SdlCallback {
public:
    SdlCallback() : cond(std::make_shared<BoxedWineCondition>(B("SdlCallback"))) {}

    SDL_Event sdlEvent = { 0 };
    BOXEDWINE_CONDITION cond;
    std::function<U32()> pfn;
    U32 result = 0;
    SdlCallback* next = nullptr;
};

U32 sdlDispatch(std::function<U32()> p);
#define COMMA ,
#define DISPATCH_MAIN_THREAD_BLOCK_BEGIN sdlDispatch([=]() -> U32 { 
#define DISPATCH_MAIN_THREAD_BLOCK_THIS_BEGIN sdlDispatch([=, this]() -> U32 {
#define DISPATCH_MAIN_THREAD_BLOCK_BEGIN_WITH_ARG(x) sdlDispatch([x]() -> U32 {
#define DISPATCH_MAIN_THREAD_BLOCK_THIS_BEGIN_WITH_ARG2(x, y) sdlDispatch([x, y, this]() -> U32 {
#define DISPATCH_MAIN_THREAD_BLOCK_BEGIN_RETURN return sdlDispatch([=, this]() -> U32 {
#define DISPATCH_MAIN_THREAD_BLOCK_THIS_BEGIN_RETURN return sdlDispatch([=, this]() -> U32 {
#define DISPATCH_MAIN_THREAD_BLOCK_END return 0;});
#else
#define DISPATCH_MAIN_THREAD_BLOCK_BEGIN
#define DISPATCH_MAIN_THREAD_BLOCK_THIS_BEGIN
#define DISPATCH_MAIN_THREAD_BLOCK_BEGIN_WITH_ARG(x)
#define DISPATCH_MAIN_THREAD_BLOCK_BEGIN_RETURN
#define DISPATCH_MAIN_THREAD_BLOCK_THIS_BEGIN_RETURN
#define DISPATCH_MAIN_THREAD_BLOCK_END
#endif

#endif
