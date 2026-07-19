/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __TEST_WASM_JIT_MODULE_BROKER_H__
#define __TEST_WASM_JIT_MODULE_BROKER_H__

void testWasmJitMtModuleBrokerTransport();
void testWasmJitMtStandaloneModuleBroker();
void testWasmJitMtModuleBrokerLifecycle();
void testWasmJitMtModuleBrokerThreadStartOwner();
void testWasmJitMtGroupedModuleBroker();
void testWasmJitMtModuleBrokerPreloadSelection();
void testWasmJitMtModuleBrokerExecIncarnation();
void testWasmJitMtModuleBrokerPreloadDiagnostics();
void testWasmJitMtScheduleThreadPreload();

#endif
