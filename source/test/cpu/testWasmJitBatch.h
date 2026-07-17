/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __TEST_WASM_JIT_BATCH_H__
#define __TEST_WASM_JIT_BATCH_H__

#ifdef BOXEDWINE_WASM_JIT
void testWasmJitModuleMerger();
void testWasmJitBatchPolicy();
void testWasmJitMappedFileRange();
#ifdef BOXEDWINE_MULTI_THREADED
void testWasmJitMtRuntimeGrouping();
void testWasmJitMtPendingLifecycle();
void testWasmJitMtGroupedOomBlock();
#else
void testWasmJitRuntimeGrouping();
void testWasmJitPendingLifecycle();
void testWasmJitTinyAnonymousPromotion();
void testWasmJitGroupedOomRecovery();
#endif
#endif

#endif
