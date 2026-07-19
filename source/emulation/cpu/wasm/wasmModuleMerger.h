/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef __WASM_MODULE_MERGER_H__
#define __WASM_MODULE_MERGER_H__

#ifdef BOXEDWINE_WASM_JIT
#include "boxedwine.h"

struct WasmJitMergeInput {
    const std::vector<U8>* bytes = nullptr;
};

bool wasmJitMergeModules(const std::vector<WasmJitMergeInput>& inputs, std::vector<U8>& output, BString& error);

#endif
#endif
