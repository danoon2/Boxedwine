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

#include "boxedwine.h"
#include "llvm_helper.h"

// copied from LLVM
// http://llvm.org/doxygen/AArch64AddressingModes_8h_source.html
//
// LICENSE AT THE TOP OF THE FILE
//
//===- AArch64AddressingModes.h - AArch64 Addressing Modes ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

constexpr inline bool isMask_64(uint64_t Value) {
    return Value && ((Value + 1) & Value) == 0;
}

constexpr inline bool isShiftedMask_64(uint64_t Value) {
    return Value && isMask_64((Value - 1) | Value);
}

/// The behavior an operation has on an input of 0.
enum ZeroBehavior {
    /// The returned value is undefined.
    ZB_Undefined,
    /// The returned value is numeric_limits<T>::max()
    ZB_Max,
    /// The returned value is numeric_limits<T>::digits
    ZB_Width
};

template <typename T, std::size_t SizeOfT> struct TrailingZerosCounter {
    static unsigned count(T Val, ZeroBehavior) {
        if (!Val)
            return std::numeric_limits<T>::digits;
        if (Val & 0x1)
            return 0;

        // Bisection method.
        unsigned ZeroBits = 0;
        T Shift = std::numeric_limits<T>::digits >> 1;
        T Mask = std::numeric_limits<T>::max() >> Shift;
        while (Shift) {
            if ((Val & Mask) == 0) {
                Val >>= Shift;
                ZeroBits |= Shift;
            }
            Shift >>= 1;
            Mask >>= Shift;
        }
        return ZeroBits;
    }
};

#if defined(__GNUC__) || defined(_MSC_VER)
template <typename T> struct TrailingZerosCounter<T, 4> {
    static unsigned count(T Val, ZeroBehavior ZB) {
        if (ZB != ZB_Undefined && Val == 0)
            return 32;

#if defined(__GNUC__)
        return __builtin_ctz(Val);
#elif defined(_MSC_VER)
        unsigned long Index;
        _BitScanForward(&Index, Val);
        return Index;
#endif
    }
};

#if !defined(_MSC_VER) || defined(_M_X64)
template <typename T> struct TrailingZerosCounter<T, 8> {
    static unsigned count(T Val, ZeroBehavior ZB) {
        if (ZB != ZB_Undefined && Val == 0)
            return 64;

#if defined(__GNUC__)
        return __builtin_ctzll(Val);
#elif defined(_MSC_VER)
        unsigned long Index;
        _BitScanForward64(&Index, Val);
        return Index;
#endif
    }
};
#endif
#endif

template <typename T>
unsigned countTrailingZeros(T Val, ZeroBehavior ZB = ZB_Width) {
    static_assert(std::numeric_limits<T>::is_integer &&
        !std::numeric_limits<T>::is_signed,
        "Only unsigned integral types are allowed.");
    return TrailingZerosCounter<T, sizeof(T)>::count(Val, ZB);
}

template <typename T>
unsigned countTrailingOnes(T Value, ZeroBehavior ZB = ZB_Width) {
    static_assert(std::numeric_limits<T>::is_integer &&
        !std::numeric_limits<T>::is_signed,
        "Only unsigned integral types are allowed.");
    return countTrailingZeros<T>(~Value, ZB);
}

template <typename T, std::size_t SizeOfT> struct LeadingZerosCounter {
    static unsigned count(T Val, ZeroBehavior) {
        if (!Val)
            return std::numeric_limits<T>::digits;

        // Bisection method.
        unsigned ZeroBits = 0;
        for (T Shift = std::numeric_limits<T>::digits >> 1; Shift; Shift >>= 1) {
            T Tmp = Val >> Shift;
            if (Tmp)
                Val = Tmp;
            else
                ZeroBits |= Shift;
        }
        return ZeroBits;
    }
};

#if defined(__GNUC__) || defined(_MSC_VER)
template <typename T> struct LeadingZerosCounter<T, 4> {
    static unsigned count(T Val, ZeroBehavior ZB) {
        if (ZB != ZB_Undefined && Val == 0)
            return 32;

#if defined(__GNUC__)
        return __builtin_clz(Val);
#elif defined(_MSC_VER)
        unsigned long Index;
        _BitScanReverse(&Index, Val);
        return Index ^ 31;
#endif
    }
};

#if !defined(_MSC_VER) || defined(_M_X64)
template <typename T> struct LeadingZerosCounter<T, 8> {
    static unsigned count(T Val, ZeroBehavior ZB) {
        if (ZB != ZB_Undefined && Val == 0)
            return 64;

#if defined(__GNUC__)
        return __builtin_clzll(Val);
#elif defined(_MSC_VER)
        unsigned long Index;
        _BitScanReverse64(&Index, Val);
        return Index ^ 63;
#endif
    }
};
#endif
#endif

template <typename T>
unsigned countLeadingZeros(T Val, ZeroBehavior ZB = ZB_Width) {
    static_assert(std::numeric_limits<T>::is_integer &&
        !std::numeric_limits<T>::is_signed,
        "Only unsigned integral types are allowed.");
    return LeadingZerosCounter<T, sizeof(T)>::count(Val, ZB);
}

template <typename T>
unsigned countLeadingOnes(T Value, ZeroBehavior ZB = ZB_Width) {
    static_assert(std::numeric_limits<T>::is_integer &&
        !std::numeric_limits<T>::is_signed,
        "Only unsigned integral types are allowed.");
    return countLeadingZeros<T>(~Value, ZB);
}

bool processLogicalImmediate(U64 Imm, U32 RegSize, U64& Encoding) {
    if (Imm == 0ULL || Imm == ~0ULL ||
        (RegSize != 64 &&
            (Imm >> RegSize != 0 || Imm == (~0ULL >> (64 - RegSize)))))
        return false;

    // First, determine the element size.
    unsigned Size = RegSize;

    do {
        Size /= 2;
        uint64_t Mask = (1ULL << Size) - 1;

        if ((Imm & Mask) != ((Imm >> Size) & Mask)) {
            Size *= 2;
            break;
        }
    } while (Size > 2);

    // Second, determine the rotation to make the element be: 0^m 1^n.
    uint32_t CTO, I;
    uint64_t Mask = ((uint64_t)-1LL) >> (64 - Size);
    Imm &= Mask;

    if (isShiftedMask_64(Imm)) {
        I = countTrailingZeros(Imm);
        //assert(I < 64 && "undefined behavior");
        CTO = countTrailingOnes(Imm >> I);
    } else {
        Imm |= ~Mask;
        if (!isShiftedMask_64(~Imm))
            return false;

        unsigned CLO = countLeadingOnes(Imm);
        I = 64 - CLO;
        CTO = CLO + countTrailingOnes(Imm) - (64 - Size);
    }

    // Encode in Immr the number of RORs it would take to get *from* 0^m 1^n
    // to our target value, where I is the number of RORs to go the opposite
    // direction.
    //assert(Size > I && "I should be smaller than element size");
    unsigned Immr = (Size - I) & (Size - 1);

    // If size has a 1 in the n'th bit, create a value that has zeroes in
    // bits [0, n] and ones above that.
    uint64_t NImms = ~(Size - 1) << 1;

    // Or the CTO value into the low bits, which must be below the Nth bit
    // bit mentioned above.
    NImms |= (CTO - 1);

    // Extract the seventh bit and toggle it to create the N field.
    unsigned N = ((NImms >> 6) & 1) ^ 1;

    Encoding = (N << 12) | (Immr << 6) | (NImms & 0x3f);
    return true;
}