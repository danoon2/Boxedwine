/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"

#ifdef __TEST
#include "../cpu/testCPU.h"
#include "../../x11/x11.h"
#include "../../../tools/x11/X11_def.h"
#include "knativesystem.h"

static BOXEDWINE_MUTEX paletteTestMutex;

static void initPaletteTestServer() {
    // The CPU test runner has no native screen. Allocate the headless backing
    // screen before XServer queries its default depth; no SDL window is made.
    if (!KNativeSystem::getScreen()) KNativeSystem::initWindow(640, 480, 8, 100, 100, B("0"), 0, 0);
}

extern void x11_init();

void testCapturedMouseSensitivity() {
    CapturedMouse mouse;
    auto position = [&](int rawX, int rawY, int expectedX, int expectedY) {
        int x = rawX, y = rawY;
        mouse.map(x, y);
        if (x != expectedX || y != expectedY) testFail("Captured mouse %d,%d -> %d,%d, expected %d,%d", rawX, rawY, x, y, expectedX, expectedY);
    };
    mouse.configure(true, false, true, 50);
    position(321, 238, 321, 238); // A grab alone is not relative input.
    mouse.warp(319, 239); // Alice's recenter point.
    position(329, 229, 324, 234);
    position(329, 229, 324, 234); // Queries and events must not count twice.
    mouse.warp(319, 239);
    for (int i = 0; i < 100; ++i) {
        position(320, 238, 319 + i % 2, 239 - i % 2);
        position(320, 238, 319 + i % 2, 239 - i % 2);
        mouse.warp(319, 239);
        position(319, 239, 319, 239); // Warp event contributes no movement.
    }
    // Both signs, direction changes, and changing the warp target.
    position(318, 240, 319, 239);
    mouse.warp(500, 400);
    position(501, 399, 500, 400);
    mouse.configure(true, false, true, 200);
    mouse.warp(319, 239);
    position(324, 236, 329, 233);
    for (int i = 0; i < 100; ++i) {
        mouse.warp(319, 239);
        position(319, 239, 319, 239);
    }
    // Visible menus, release, focus loss, and 100% all bypass the transform.
    for (int mode = 0; mode < 4; ++mode) {
        mouse.configure(mode != 0, mode == 1, mode != 2, mode == 3 ? 100 : 50);
        mouse.warp(319, 239);
        position(329, 229, 329, 229);
        mouse.configure(true, false, true, 50);
        position(329, 229, 329, 229); // No old anchor on reentry.
        mouse.warp(319, 239);
        position(320, 238, 319, 239); // No old remainder either.
    }
    mouse.reset(); // Resolution/presentation change.
    position(329, 229, 329, 229);
    mouse.configure(true, false, true, 0); // Legacy default.
    mouse.warp(319, 239);
    position(329, 229, 329, 229);
    mouse.configure(true, false, true, 1000);
    mouse.warp(0, 0);
    position(INT32_MAX, INT32_MIN, INT32_MAX, INT32_MIN); // Saturate, never wrap.
}

void testX11ImageIncludeInferiors() {
	// No global X server or native window is needed for these backing-store
	// checks. The test windows retain dirty state locally.
	class ImageWindow : public XWindow {
	public:
		using XWindow::XWindow;
		void setDirty() override { isDirty = true; }
	};
	testNewInstruction(0);
	VisualPtr visual = std::make_shared<Visual>();
	visual->bits_per_rgb = 32;
	auto parent = std::make_shared<ImageWindow>(0, nullptr, 8, 6, 32, 50, 60, InputOutput, 0, visual);
	auto child = std::make_shared<ImageWindow>(0, parent, 4, 3, 32, 2, 1, InputOutput, 0, visual);
	auto nested = std::make_shared<ImageWindow>(0, child, 2, 2, 32, 1, 1, InputOutput, 0, visual);
	auto partial = std::make_shared<ImageWindow>(0, parent, 4, 3, 32, 5, 4, InputOutput, 0, visual);
	auto unmapped = std::make_shared<ImageWindow>(0, parent, 2, 2, 32, 0, 0, InputOutput, 0, visual);
	auto otherDepth = std::make_shared<ImageWindow>(0, parent, 2, 2, 24, 0, 0, InputOutput, 0, visual);
	std::vector<XWindowPtr> children = {child, nested, partial, unmapped, otherDepth};
	for (const auto& window : children) {
		window->isMapped = window != unmapped;
		window->addToParent();
	}
	XGCPtr gc = std::make_shared<XGC>(parent);
	for (U32 y = 0; y < 8; ++y) {
		for (U32 x = 0; x < 8; ++x) {
			testContext().memory->writed(TEST_HEAP_ADDRESS + (y * 8 + x) * 4, 0x100000 + y * 100 + x);
		}
	}
	auto upload = [&]() {
		if (parent->copyImageData(testContext().thread, gc, TEST_HEAP_ADDRESS, 32, 32, 1, 1, 1, 1, 5, 4) != Success) {
			testFail("X11 image upload failed");
		}
	};
	upload(); // ClipByChildren must not overwrite the child backing stores.
	for (const auto& window : children) {
		for (U32 i = 0; i < window->getDataSize(); ++i) {
			if (window->getData()[i]) testFail("ClipByChildren changed a child image");
		}
	}
	gc->values.subwindow_mode = IncludeInferiors;
	upload();
	for (const auto& window : children) {
		S32 ox = 0, oy = 0;
		window->windowToScreen(ox, oy);
		ox -= 50; oy -= 60;
		for (U32 y = 0; y < window->height(); ++y) {
			for (U32 x = 0; x < window->width(); ++x) {
				S32 px = ox + x, py = oy + y;
				U32 expected = window != unmapped && window != otherDepth && px >= 1 && px < 6 && py >= 1 && py < 5
					? 0x100000 + py * 100 + px : 0;
				U32 actual;
				memcpy(&actual, window->getData() + y * window->getBytesPerLine() + x * 4, 4);
				if (actual != expected) testFail("IncludeInferiors child pixel mismatch at %u,%u: %x != %x", x, y, actual, expected);
			}
		}
	}
	// Break the parent/child shared-pointer ownership in this isolated tree.
	for (auto it = children.rbegin(); it != children.rend(); ++it) (*it)->removeFromParent();
}

void testX11ImageScanlinePadding() {
    x11_init();
    testNewInstruction(0);
    CPU* cpu = testContext().cpu;
    KMemory* memory = cpu->memory;
    const struct { U32 width, depth, pad, suppliedPitch, expectedPitch; } cases[] = {
        {153, 32, 32, 0, 612}, // Wine's partial GDI text upload.
        {154, 24, 32, 0, 616}, {1, 32, 32, 0, 4},
        {5, 16, 32, 0, 12}, {3, 8, 32, 0, 4},
        {3, 8, 16, 0, 4}, {3, 8, 8, 0, 3},
        {1, 1, 8, 0, 1}, {9, 1, 8, 0, 2},
        {17, 1, 16, 0, 4}, {33, 1, 32, 0, 8},
        {153, 32, 32, 1024, 1024} // Preserve an explicit client stride.
    };
    for (const auto& row : cases) {
        U32 args[] = {0, 0, row.depth, ZPixmap, 0, TEST_HEAP_ADDRESS,
            row.width, 3, row.pad, row.suppliedPitch};
        for (U32 i = 0; i < 10; ++i) {
            memory->writed(cpu->seg[SS].address + cpu->reg[4].u32 + (i + 1) * 4, args[i]);
        }
        callX11(cpu, X11_CREATE_IMAGE);
        U32 address = EAX;
        if (!address) { testFail("XCreateImage failed"); continue; }
        XImage image;
        XImage::read(memory, address, &image);
        if ((U32)image.bytes_per_line != row.expectedPitch) {
            testFail("XCreateImage width=%u depth=%u pad=%u stride=%d expected=%u",
                row.width, row.depth, row.pad, image.bytes_per_line, row.expectedPitch);
        }
        if (row.width == 153 && !row.suppliedPitch) {
            // A row-dependent pattern catches the observed shifted scanlines
            // through the actual image-upload operation, beyond header checks.
            for (U32 y = 0; y < 3; ++y) {
                for (U32 x = 0; x < 153; ++x) {
                    memory->writed(TEST_HEAP_ADDRESS + (y * 153 + x) * 4, 0x11000000 + y * 0x10000 + x);
                }
            }
            VisualPtr visual = std::make_shared<Visual>();
            visual->bits_per_rgb = 32;
            auto drawable = std::make_shared<XDrawable>(153, 3, 32, visual, false, false);
            auto gc = std::make_shared<XGC>(drawable);
            if (drawable->putImage(cpu->thread, gc, &image, 0, 0, 0, 0, 153, 3) != Success) {
                testFail("XCreateImage test upload failed");
            }
            U32 incorrect = 0;
            for (U32 y = 0; y < 3; ++y) {
                for (U32 x = 0; x < 153; ++x) {
                    U32 pixel;
                    memcpy(&pixel, drawable->getData() + y * drawable->getBytesPerLine() + x * 4, 4);
                    incorrect += pixel != 0x11000000 + y * 0x10000 + x;
                }
            }
            if (incorrect) testFail("XCreateImage shifted %u uploaded pixels", incorrect);
        }
        cpu->thread->process->free(address);
    }
}

void testXInputValuatorLayout() {
	testNewInstruction(0);
	KMemory* memory = testContext().memory;
	const U32 address = TEST_HEAP_ADDRESS;
	for (U32 offset = 0; offset < 52; offset += 4) memory->writed(address + offset, 0xcccccccc);
	XIValuatorClassInfo::write(memory, address, XIValuatorClass, 7, 1, 42, -10.5, 123.25, 17.75, 300, XIModeRelative);
	long2Double value;
	value.l = memory->readq(address + 32);
	if (value.d != 17.75 || memory->readd(address + 40) != 300 || memory->readd(address + 44) != XIModeRelative) {
		testFail("XInput valuator value, resolution, or relative mode has wrong guest ABI offset");
	}
	if (memory->readd(address + 48) != 0xcccccccc) testFail("XInput valuator write overran guest structure");
}

void testX11GetImageBounds() {
    testNewInstruction(0);
    auto visual = std::make_shared<Visual>();
    visual->bits_per_rgb = 8;
    XDrawable drawable(4, 2, 8, visual, false);
    for (U32 i = 0; i < 8; ++i) drawable.getData()[i] = (U8)(i + 1);
    auto capture = [&](S32 x, S32 y, U32 w, U32 h) {
        return drawable.getImage(testContext().thread, x, y, w, h, AllPlanes, ZPixmap, 0, 0, 0);
    };
    if (capture(0, 0, 0, 1) || capture(0, 0, 1, 0) || capture(0, 0, 0xffffffff, 2))
        testFail("XGetImage accepted an empty or overflowing image");
    // Last pixel of the last row: copying the padded stride would overread.
    U32 address = capture(3, 1, 1, 1);
    if (!address) { testFail("XGetImage rejected a valid edge pixel"); return; }
    XImage image;
    auto memory = testContext().memory;
    XImage::read(memory, address, &image);
    if (image.bytes_per_line != 4 || memory->readd(image.data) != 8)
        testFail("XGetImage edge pixel/padding incorrect");
    testContext().thread->process->free(image.data);
    testContext().thread->process->free(address);
    // A software cursor can save pixels across all four edges. Keep the
    // requested dimensions, fill missing pixels/padding, and copy only bounds.
    address = capture(-1, -1, 6, 4);
    if (!address) { testFail("XGetImage rejected a rectangle crossing drawable edges"); return; }
    XImage::read(memory, address, &image);
    if (image.width != 6 || image.height != 4 || image.bytes_per_line != 8)
        testFail("XGetImage changed the requested edge rectangle");
    for (U32 row = 0; row < 4; ++row) {
        for (U32 column = 0; column < 8; ++column) {
            U8 expected = row >= 1 && row <= 2 && column >= 1 && column <= 4 ?
                (U8)((row - 1) * 4 + column) : 0;
            if (memory->readb(image.data + row * 8 + column) != expected)
                testFail("XGetImage edge clipping/padding incorrect");
        }
    }
    testContext().thread->process->free(image.data);
    testContext().thread->process->free(address);
    address = capture(4, 2, 1, 1);
    if (!address) { testFail("XGetImage rejected an offscreen cursor rectangle"); return; }
    XImage::read(memory, address, &image);
    if (memory->readd(image.data)) testFail("XGetImage offscreen pixels were not cleared");
    testContext().thread->process->free(image.data);
    testContext().thread->process->free(address);
    drawable.setSize(0, 0);
    if (capture(0, 0, 1, 1)) testFail("XGetImage accepted an empty backing store");
    auto monoVisual = std::make_shared<Visual>();
    monoVisual->bits_per_rgb = 1;
    XDrawable mono(33, 1, 1, monoVisual, false);
    if (mono.getBytesPerLine() != 8) testFail("monochrome drawable lost a partial byte");
    memset(mono.getData(), 0xaa, mono.getDataSize());
    address = mono.getImage(testContext().thread, 1, 0, 32, 1, AllPlanes, ZPixmap, 0, 0, 0);
    if (!address) { testFail("XGetImage rejected a monochrome rectangle"); return; }
    XImage::read(memory, address, &image);
    if (memory->readd(image.data) != 0x55555555) testFail("XGetImage lost the monochrome bit offset");
    testContext().thread->process->free(image.data);
    testContext().thread->process->free(address);
}

void testX11ImageReadback() {
    class ImageWindow : public XWindow {
    public:
        using XWindow::XWindow;
        void setDirty() override { isDirty = true; }
    };
    testNewInstruction(0);
    auto thread = testContext().thread;
    auto memory = thread->memory;
    for (U32 depth : {8u, 32u}) {
        auto visual = std::make_shared<Visual>();
        visual->bits_per_rgb = depth;
        auto root = std::make_shared<ImageWindow>(0, nullptr, 8, 6, depth, 50, 60, InputOutput, 0, visual);
        auto game = std::make_shared<ImageWindow>(0, root, 5, 4, depth, 1, 1, InputOutput, 0, visual);
        auto nested = std::make_shared<ImageWindow>(0, game, 4, 4, depth, 3, 2, InputOutput, 0, visual);
        auto overlap = std::make_shared<ImageWindow>(0, root, 2, 2, depth, 5, 3, InputOutput, 0, visual);
        auto partial = std::make_shared<ImageWindow>(0, root, 4, 3, depth, 6, 5, InputOutput, 0, visual);
        auto hidden = std::make_shared<ImageWindow>(0, root, 8, 6, depth, 0, 0, InputOutput, 0, visual);
        auto input = std::make_shared<ImageWindow>(0, root, 8, 6, depth, 0, 0, InputOnly, 0, visual);
        auto otherDepth = std::make_shared<ImageWindow>(0, root, 8, 6, depth + 1, 0, 0, InputOutput, 0, visual);
        std::vector<XWindowPtr> windows = {root, game, nested, overlap, partial, hidden, input, otherDepth};
        for (U32 i = 0; i < windows.size(); ++i) {
            auto window = windows[i];
            window->isMapped = window != hidden;
            if (window != root) window->addToParent();
            for (U32 y = 0; y < window->height(); ++y) for (U32 x = 0; x < window->width(); ++x) {
                U32 color = 11 + i * 11;
                memcpy(window->getData() + y * window->getBytesPerLine() + x * (depth / 8), &color, depth / 8);
            }
        }
        auto expected = [&](S32 x, S32 y) -> U32 {
            if (x < 0 || y < 0 || x >= 8 || y >= 6) return 0;
            U32 value = 11;
            if (x >= 1 && x < 6 && y >= 1 && y < 5) {
                value = 22;
                if (x >= 4 && y >= 3) value = 33;
            }
            if (x >= 5 && x < 7 && y >= 3 && y < 5) value = 44;
            if (x >= 6 && y == 5) value = 55;
            return value;
        };
        auto verify = [&]() {
            U32 address = root->getImage(thread, -1, -1, 10, 8, AllPlanes, ZPixmap, 0, 0, 0);
            XImage image;
            XImage::read(memory, address, &image);
            for (U32 y = 0; y < 8; ++y) for (U32 x = 0; x < 10; ++x) {
                U32 pixel = image.data + y * image.bytes_per_line + x * (depth / 8);
                U32 actual = depth == 8 ? memory->readb(pixel) : memory->readd(pixel);
                if (actual != expected(x - 1, y - 1)) testFail("Window readback depth=%u at %u,%u: %u", depth, x, y, actual);
            }
            thread->process->free(image.data);
            thread->process->free(address);
        };
        verify(); // Includes nested children, stacking, parent clipping and screen edges.
        auto gc = std::make_shared<XGC>(root);
        gc->values.subwindow_mode = IncludeInferiors;
        for (U32 i = 0; i < 6; ++i) memory->writed(TEST_HEAP_ADDRESS + i * 4, 0xcccccccc);
        for (S32 x : {0, 3, 5}) {
            U32 address = root->getImage(thread, x, 3, 3, 2, AllPlanes, ZPixmap, 0, 0, 0);
            XImage saved;
            XImage::read(memory, address, &saved);
            root->copyImageData(thread, gc, TEST_HEAP_ADDRESS, 3 * (depth / 8), depth, 0, 0, x, 3, 3, 2);
            root->putImage(thread, gc, &saved, 0, 0, x, 3, 3, 2);
            thread->process->free(saved.data);
            thread->process->free(address);
            verify(); // Repeated cursor movement must leave the visible background intact.
        }
        for (auto it = windows.rbegin(); it != windows.rend() - 1; ++it) (*it)->removeFromParent();
    }
}

void testX11PalettedDrawing() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(paletteTestMutex);
    x11_init();
    testNewInstruction(0);
    initPaletteTestServer();
    auto visual = std::make_shared<Visual>();
    visual->bits_per_rgb = 8;
    auto server = XServer::getServer();
    auto drawable = server->createNewPixmap(5, 4, 8, visual);
    auto gc = server->createGC(drawable);
    auto thread = testContext().thread;
    auto memory = thread->memory;
    CPU* cpu = testContext().cpu;
    auto call = [&](U32 function, std::initializer_list<U32> args, U32 expected = Success) {
        U32 i = 1;
        for (U32 arg : args) memory->writed(cpu->seg[SS].address + cpu->reg[4].u32 + i++ * 4, arg);
        callX11(cpu, function);
        if (EAX != expected) testFail("Paletted drawing X11 call failed: %u", function);
    };
    auto pixel = [&](U32 x, U32 y) { return drawable->getData()[y * drawable->getBytesPerLine() + x]; };
    memset(drawable->getData(), 0xcc, drawable->getDataSize());
    gc->values.foreground = 73;
    drawable->fillRectangle(thread, gc, -2, -2, 4, 4);
    for (U32 y = 0; y < 4; y++) for (U32 x = 0; x < drawable->getBytesPerLine(); x++) {
        if (pixel(x, y) != (x < 2 && y < 2 ? 73 : 0xcc)) testFail("8-bit fill bounds/padding");
    }
    drawable->fillRectangle(thread, gc, 7, 7, 2, 2);
    gc->values.foreground = 42;
    drawable->drawLine(thread, gc, 4, 3, 4, -3);
    drawable->drawLine(thread, gc, 8, 3, -3, 3);
    for (U32 y = 0; y < 4; y++) if (pixel(4, y) != 42) testFail("8-bit reversed vertical line");
    for (U32 x = 0; x < 5; x++) if (pixel(x, 3) != 42) testFail("8-bit reversed horizontal line");
    gc->values.foreground = 99;
    gc->values.cap_style = CapNotLast;
    drawable->drawLine(thread, gc, 0, 3, 4, 3);
    if (pixel(3, 3) != 99 || pixel(4, 3) != 42) testFail("8-bit CapNotLast endpoint");
    gc->values.cap_style = CapButt;

    // The XFillRectangles guest ABI uses signed 16-bit origins and 8-byte records.
    U32 rects = TEST_HEAP_ADDRESS;
    memory->writew(rects, (U16)-1); memory->writew(rects + 2, 0);
    memory->writew(rects + 4, 2); memory->writew(rects + 6, 1);
    memory->writew(rects + 8, 3); memory->writew(rects + 10, 2);
    memory->writew(rects + 12, 2); memory->writew(rects + 14, 1);
    call(X11_FILL_RECTANGLES, {0, drawable->id, gc->id, rects, 2});
    if (pixel(0, 0) != 99 || pixel(3, 2) != 99 || pixel(4, 2) != 99) testFail("XFillRectangles guest layout");

    memory->writew(rects, 0); memory->writew(rects + 2, 0);
    memory->writew(rects + 4, 2); memory->writew(rects + 6, 1);
    call(X11_SET_CLIP_RECTANGLES, {0, gc->id, 1, 1, rects, 1, 0});
    gc->values.foreground = 17;
    drawable->fillRectangle(thread, gc, 0, 0, 5, 4);
    if (pixel(1, 1) != 17 || pixel(2, 1) != 17 || pixel(0, 0) != 99 || pixel(4, 3) != 42)
        testFail("8-bit GC clip rectangle/origin");
    call(X11_SET_CLIP_RECTANGLES, {0, gc->id, 0, 0, rects, 0, 0});
    gc->values.foreground = 18;
    drawable->fillRectangle(thread, gc, 0, 0, 5, 4);
    if (pixel(1, 1) != 17) testFail("Empty GC clip must suppress drawing");
    call(X11_SET_CLIP_MASK, {0, gc->id, None});

    // XSetDashes retains a complete pattern, expands odd lists, and rejects
    // invalid replacements without changing the existing GC.
    memory->writeb(rects, 3); memory->writeb(rects + 1, 2); memory->writeb(rects + 2, 1);
    call(X11_SET_DASHES, {0, gc->id, 5, rects, 3});
    if (gc->dashList != std::vector<U8>({3, 2, 1, 3, 2, 1}) || gc->values.dash_offset != 5)
        testFail("XSetDashes odd pattern/offset");
    memory->writeb(rects, 0);
    call(X11_SET_DASHES, {0, gc->id, 0, rects, 1}, BadValue);
    if (EAX != BadValue || gc->dashList[0] != 3 || gc->values.dash_offset != 5)
        testFail("XSetDashes accepted a zero length");
    call(X11_SET_DASHES, {0, gc->id, 0, rects, 0}, BadValue);
    if (EAX != BadValue) testFail("XSetDashes accepted an empty pattern");

    const U8 rops[] = {0, 0x12, 0x84, 0x96, 0x48, 0x5a, 0xcc, 0xde,
        0x21, 0x33, 0xa5, 0xb7, 0x69, 0x7b, 0xed, 0xff};
    for (U32 op = 0; op < 16; op++) {
        drawable->getData()[0] = 0x5a;
        gc->values.foreground = 0x96;
        gc->values.function = op;
        gc->values.plane_mask = 0x3c;
        drawable->fillRectangle(thread, gc, 0, 0, 1, 1);
        if (pixel(0, 0) != ((0x5a & ~0x3c) | (rops[op] & 0x3c))) testFail("8-bit raster operation %u", op);
    }
    server->removeGC(gc->id);
    server->removePixmap(drawable->id);
}

void testX11PatternFills() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(paletteTestMutex);
    initPaletteTestServer();
    auto server = XServer::getServer();
    for (U32 bits : {1u, 8u, 16u, 32u}) {
        auto visual = std::make_shared<Visual>(); visual->bits_per_rgb = bits;
        auto bitmapVisual = std::make_shared<Visual>(); bitmapVisual->bits_per_rgb = 1;
        auto target = server->createNewPixmap(7, 5, bits, visual);
        auto tile = server->createNewPixmap(3, 2, bits, visual);
        auto stipple = server->createNewPixmap(3, 2, 1, bitmapVisual);
        auto gc = server->createGC(target);
        auto pixel = [&](const XDrawablePtr& image, U32 x, U32 y) {
            U8* row = image->getData() + y * image->getBytesPerLine();
            U32 value = 0;
            if (image->getBitsPerPixel() == 1) value = (row[x / 8] >> (x & 7)) & 1;
            else memcpy(&value, row + x * (image->getBitsPerPixel() / 8), image->getBitsPerPixel() / 8);
            return value;
        };
        for (U32 y = 0; y < 2; y++) for (U32 x = 0; x < 3; x++) {
            U32 value = 1 + x + y * 3;
            if (bits == 1) tile->getData()[y * tile->getBytesPerLine()] |= (value & 1) << x;
            else memcpy(tile->getData() + y * tile->getBytesPerLine() + x * (bits / 8), &value, bits / 8);
        }
        stipple->getData()[0] = 5; stipple->getData()[stipple->getBytesPerLine()] = 2;
        XGCValues values = gc->values;
        values.tile = tile->id; values.stipple = stipple->id;
        gc->updateValues(GCTile | GCStipple, &values);
        server->removePixmap(tile->id); server->removePixmap(stipple->id);
        gc->values.ts_x_origin = 4; gc->values.ts_y_origin = -3;
        gc->values.fill_style = FillTiled;
        memset(target->getData(), 0xcc, target->getDataSize());
        if (target->fillRectangle(nullptr, gc, -2, -1, 12, 8) != Success) testFail("%u-bit tile fill failed", bits);
        for (U32 y = 0; y < 5; y++) for (U32 x = 0; x < 7; x++) {
            if (pixel(target, x, y) != pixel(tile, (x + 2) % 3, (y + 3) % 2)) testFail("%u-bit tile origin/repetition/lifetime", bits);
        }
        // Empty and offscreen rectangles leave both pixels and row padding intact.
        std::vector<U8> before(target->getData(), target->getData() + target->getDataSize());
        target->fillRectangle(nullptr, gc, 20, 20, 2, 2);
        target->fillRectangle(nullptr, gc, 0, 0, 0, 5);
        if (memcmp(before.data(), target->getData(), before.size())) testFail("%u-bit empty fill changed pixels", bits);
        for (U32 y = 0; y < 5; y++) {
            U32 used = (7 * bits + 7) / 8;
            for (U32 x = used; x < target->getBytesPerLine(); x++)
                if (target->getData()[y * target->getBytesPerLine() + x] != 0xcc) testFail("%u-bit tile changed row padding", bits);
        }
        // A retained tile must be ignored for solid brushes.
        gc->values.fill_style = FillSolid; gc->values.foreground = 0x96;
        const U32 rops[] = {0, 0x12, 0x84, 0x96, 0x48, 0x5a, 0xcc, 0xde,
            ~0xdeu, ~0xccu, ~0x5au, 0x96u | ~0x5au, ~0x96u, ~0x96u | 0x5au, ~0x12u, 0xffffffff};
        for (U32 op = 0; op < 16; op++) {
            memset(target->getData(), 0, target->getDataSize());
            target->getData()[0] = 0x5a;
            gc->values.function = op; gc->values.plane_mask = bits == 1 ? 1 : 0x3c;
            target->fillRectangle(nullptr, gc, 0, 0, 1, 1);
            U32 expected = bits == 1 ? (rops[op] & 1) : ((0x5a & ~0x3c) | (rops[op] & 0x3c));
            if (pixel(target, 0, 0) != expected) testFail("%u-bit patterned GC solid raster operation %u", bits, op);
        }
        gc->values.function = GXcopy; gc->values.plane_mask = 0xffffffff;
        gc->values.foreground = 3; gc->values.background = 4;
        XRectangle clip; clip.x = 0; clip.y = 0; clip.width = 4; clip.height = 2;
        gc->hasClipRects = true; gc->clip_rects = {clip};
        gc->values.clip_x_origin = 1; gc->values.clip_y_origin = 2;
        for (S32 style : {FillTiled, FillStippled, FillOpaqueStippled}) {
            gc->values.fill_style = style;
            memset(target->getData(), 0, target->getDataSize());
            target->fillRectangle(nullptr, gc, 0, 0, 7, 5);
            for (U32 y = 0; y < 5; y++) for (U32 x = 0; x < 7; x++) {
                U32 expected = 0;
                if (x >= 1 && x < 5 && y >= 2 && y < 4) {
                    U32 tx = (x + 2) % 3, ty = (y + 3) % 2;
                    expected = style == FillTiled ? pixel(tile, tx, ty) : pixel(stipple, tx, ty) ? 3 : style == FillOpaqueStippled ? 4 : 0;
                }
                if (bits == 1) expected &= 1;
                if (pixel(target, x, y) != expected) testFail("%u-bit fill style %d clip/origin", bits, style);
            }
        }
        gc->clip_rects.clear();
        memset(target->getData(), 0, target->getDataSize());
        target->fillRectangle(nullptr, gc, 0, 0, 7, 5);
        if (pixel(target, 1, 2)) testFail("Empty pattern clip should suppress painting");
        server->removeGC(gc->id); server->removePixmap(target->id);
    }
}

void testX11PaletteInvalidation() {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(paletteTestMutex);
    x11_init();
    testNewInstruction(0);
    initPaletteTestServer();
    auto server = XServer::getServer();
    auto visual = std::make_shared<Visual>();
    visual->bits_per_rgb = 8;
    auto map = server->getColorMap(server->createColorMap(visual.get(), AllocNone));
    for (U32 pixel : map->nativePixels) if (pixel) testFail("New colormap contains undefined RGB values");
    auto first = server->createNewWindow(0, nullptr, 2, 2, 8, 0, 0, InputOutput, 0, visual);
    auto second = server->createNewWindow(0, nullptr, 2, 2, 8, 0, 0, InputOutput, 0, visual);
    first->colorMap = second->colorMap = map;
    first->isDirty = second->isDirty = false;
    map->colors[42].flags = COLOR_ALLOCATED | COLOR_WRITE;
    CPU* cpu = testContext().cpu;
    auto memory = cpu->memory;
    U32 color = TEST_HEAP_ADDRESS;
    memory->writed(color, 42);
    memory->writew(color + 4, 0x1212); memory->writew(color + 6, 0x3434); memory->writew(color + 8, 0x5656);
    memory->writeb(color + 10, 7);
    U32 args[] = {0, map->id, color};
    for (U32 i = 0; i < 3; i++) memory->writed(cpu->seg[SS].address + cpu->reg[4].u32 + (i + 1) * 4, args[i]);
    server->isDisplayDirty = false;
    callX11(cpu, X11_STORE_COLOR);
    if (EAX != Success || !first->isDirty || !second->isDirty || !server->isDisplayDirty)
        testFail("Palette-only change did not invalidate every sharing window");
    map->buildCache();
    if (map->nativePixels[42] != 0x123456) testFail("Palette RGB cache did not update");
    // Replacing a window's map must also invalidate its converted pixels.
    first->isDirty = false;
    XSetWindowAttributes attributes = {};
    attributes.colormap = server->createColorMap(visual.get(), AllocNone);
    first->setAttributes(nullptr, &attributes, 1L << 13);
    if (!first->isDirty || first->colorMap == map) testFail("Colormap replacement did not invalidate the window");
    // AllocAll must make every palette cell writable, not just cell zero.
    visual->c_class = PseudoColor;
    visual->map_entries = MAX_COLORMAP_SIZE;
    U32 visualAddress = TEST_HEAP_ADDRESS + 64;
    visual->write(memory, visualAddress);
    U32 createArgs[] = {77, 0, visualAddress, AllocAll};
    for (U32 i = 0; i < 4; i++) memory->writed(cpu->seg[SS].address + cpu->reg[4].u32 + (i + 1) * 4, createArgs[i]);
    callX11(cpu, X11_CREATE_COLORMAP);
    auto privateMap = server->getColorMap(EAX);
    if (!privateMap) testFail("AllocAll did not create a colormap");
    for (U32 i = 0; i < MAX_COLORMAP_SIZE; i++) {
        U32 uses = 0;
        if (privateMap->colors[i].flags != (COLOR_ALLOCATED | COLOR_WRITE) ||
            !privateMap->colors[i].uses.get(77, uses) || uses != 1)
            testFail("AllocAll did not allocate writable cell %u", i);
    }
    server->destroyWindow(first->id);
    server->destroyWindow(second->id);
}

#endif
