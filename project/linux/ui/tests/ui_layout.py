# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Bounded layout waits for the generator-driven GTK smoke tests."""
import time


def settle_layout(window, reason, ready=lambda: True, timeout=5):
    """Require fresh frames and completed scroll restoration before measuring.

    Wayland may keep a window mapped while withholding frames when its display
    is disconnected. Sleeping alone then measures the previous allocation (or
    zero-sized replacement widgets), which can either fail or pass incorrectly.
    """
    assert hasattr(window, 'scroll'), 'The test library window did not open'
    display = window.get_display()
    if display.get_monitors().get_n_items() == 0:
        raise RuntimeError(f'{reason}: no active display outputs. Connect a monitor or run the test in a compositor with a virtual output.')
    clock = window.get_frame_clock()
    assert clock is not None, 'The test window has no frame clock'
    initial_frame = clock.get_frame_counter()
    started = time.monotonic()
    # Request frames even when the previous screen was idle; do not bypass the
    # compositor by manually allocating widgets or changing scroll adjustments.
    tick = window.add_tick_callback(lambda *_: True)
    try:
        while True:
            yield 25
            elapsed = time.monotonic() - started
            frames = clock.get_frame_counter() - initial_frame
            if elapsed >= .2 and frames >= 2 and window.scroll_restore is None and ready():
                return
            if elapsed >= timeout:
                if frames < 2:
                    raise RuntimeError(f'{reason}: the display is not delivering frames (advanced {frames}). Check the monitor, screen lock, and compositor before running UI tests.')
                raise AssertionError(f'{reason}: layout did not settle; scroll={window.scroll_restore}, ready={ready()}')
    finally:
        window.remove_tick_callback(tick)
