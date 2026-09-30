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

extern void x11_init();

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

#endif
