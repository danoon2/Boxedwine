#!/usr/bin/env python3
"""Build small, compiler-independent i386 instruction benchmarks using NASM."""
import argparse
import json
from pathlib import Path
import subprocess


def cases():
    result = {}
    for op in ('shufps', 'pshufd'):
        for mask in (0x00, 0x55, 0xaa, 0xff, 0xe4, 0x4e, 0xb1, 0x1b, 0x44, 0xee, 0x88, 0xdd,
                     0x39, 0x93, 0x1e, 0x27, 0x72, 0x50, 0xfa, 0xa0, 0xf5):
            for mode in ('latency', 'throughput'):
                regs = [0] if mode == 'latency' else range(8)
                body = [f'{op} xmm{r}, xmm{r}, {mask}' for _ in range(64 // len(regs)) for r in regs]
                result[f'{op}_{mask:02x}_{mode}'] = (body, len(body))
    for op in ('shufps', 'pshufd'):
        for mask in (0x00, 0x44, 0xee, 0xe4, 0x4e, 0x88, 0xdd, 0x39, 0x27):
            body = [f'{op} xmm{r}, xmm7, {mask}' for _ in range(16) for r in range(4)]
            result[f'{op}_{mask:02x}_two_inputs'] = (body, len(body))
    for mask in range(4):
        for mode in ('latency', 'throughput', 'two_inputs'):
            regs = [0] if mode == 'latency' else range(4)
            body = [f'shufpd xmm{r}, xmm{7 if mode == "two_inputs" else r}, {mask}'
                    for _ in range(64 // len(regs)) for r in regs]
            result[f'shufpd_{mask:02x}_{mode}'] = (body, len(body))
    for op in ('pshuflw', 'pshufhw'):
        for mask in (0x00, 0x55, 0xff, 0xe4, 0xb1, 0x1b, 0x4e, 0x27):
            for mode in ('latency', 'throughput', 'two_inputs'):
                regs = [0] if mode == 'latency' else range(4)
                body = [f'{op} xmm{r}, xmm{7 if mode == "two_inputs" else r}, {mask}'
                        for _ in range(64 // len(regs)) for r in regs]
                result[f'{op}_{mask:02x}_{mode}'] = (body, len(body))
    body = []
    for _ in range(16):
        body += ['movaps xmm0, xmm4', 'shufps xmm0, xmm0, 0x00', 'mulps xmm0, xmm5',
                 'movaps xmm1, xmm4', 'shufps xmm1, xmm1, 0x55', 'mulps xmm1, xmm6',
                 'addps xmm0, xmm1', 'movaps xmm2, xmm4', 'shufps xmm2, xmm2, 0xaa',
                 'mulps xmm2, xmm7', 'addps xmm0, xmm2', 'movups [edi], xmm0']
    result['vector_transform'] = (body, len(body))
    body = ['shufps xmm0, xmm1, 0x27', 'shufps xmm2, xmm3, 0x72',
            'shufps xmm4, xmm5, 0x1e', 'shufps xmm6, xmm7, 0x39'] * 16
    result['shuffle_mixed'] = (body, len(body))
    body = [f'movd xmm{r}, eax' for _ in range(16) for r in range(4)]
    result['movd_reg_throughput'] = (body, len(body))
    for mode in ('latency', 'throughput'):
        regs = [0] if mode == 'latency' else range(4)
        body = [instruction for _ in range(32 // len(regs)) for r in regs
                for reg in [('eax', 'ebx', 'ecx', 'edx')[r]]
                for instruction in (f'movd xmm{r}, {reg}', f'movd {reg}, xmm{r}')]
        result[f'movd_roundtrip_{mode}'] = (body, len(body))
    for form in ('reg', 'immediate', 'memory'):
        for mode in ('latency', 'throughput'):
            regs = ['eax'] if mode == 'latency' else ['eax', 'ebx', 'ecx']
            body = ['mov edx, 0x9e3779b1']
            for _ in range(60 // len(regs)):
                for reg in regs:
                    body.append(f'imul {reg}, ' + ('edx' if form == 'reg' else
                                f'{reg}, -1640531535' if form == 'immediate' else 'dword [esi]'))
            body += ['xor edx, edx']  # Kill all arithmetic flags after the final product.
            result[f'imul32_{form}_{mode}'] = (body, len(body))
    for width, regs in ((16, ('ax', 'bx', 'cx')), (32, ('eax', 'ebx', 'ecx'))):
        body = []
        for _ in range(20):
            for reg in regs:
                body += [f'imul {reg}, {reg}, -37', 'seto byte [edi]', 'setc byte [edi+1]']
        result[f'imul{width}_overflow_control'] = (body, len(body))
    body = []
    for _ in range(16):
        body += ['mov eax, [esi]', 'shr eax, 11', 'imul eax, dword [esi+4]',
                 'mov ebx, 2048', 'sub ebx, [esi+4]', 'shr ebx, 5',
                 'add ebx, [esi+4]', 'mov [edi], eax', 'mov [edi+4], ebx']
    result['lzma_probability_update'] = (body, len(body))
    for producer in ('cmp', 'test'):
        for cc in ('z', 'b', 'le', 'p'):
            for consumer in ('branch', 'set', 'cmov'):
                for gap in ('register', 'memory'):
                    body = ['mov edx, 3']
                    for n in range(32):
                        body += ['xor eax, 1', f'{producer} eax, edx']
                        body += (['mov ebx, esi', 'mov ecx, edi'] if gap == 'register' else
                                 ['mov ebx, [esi]', 'mov [edi+4], ebx'])
                        if consumer == 'branch':
                            body += [f'j{cc} .known_done_{n}', 'mov ecx, ebx', f'.known_done_{n}:']
                        elif consumer == 'set':
                            body += [f'set{cc} cl']
                        else:
                            body += [f'cmov{cc} ecx, ebx']
                        body += ['mov [edi], ecx']
                    result[f'known_flags_{producer}_{cc}_{consumer}_{gap}'] = (body, sum(not x.endswith(':') for x in body))
    for kind in ('adjacent', 'add32', 'cmp16', 'long_gap'):
        body = ['mov edx, 3']
        for n in range(32):
            body += ['xor eax, 1', 'add eax, edx' if kind == 'add32' else
                     'cmp ax, dx' if kind == 'cmp16' else 'cmp eax, edx']
            if kind == 'long_gap': body += ['mov ebx, [esi]'] * 17
            elif kind != 'adjacent': body += ['mov ebx, [esi]', 'mov [edi+4], ebx']
            body += ['setz cl', 'mov [edi], ecx']
        result[f'known_flags_{kind}_control'] = (body, len(body))
    # Compare a probability threshold, then preserve its flags across address
    # and memory operations before choosing the next range-coder value.
    body = []
    for n in range(16):
        body += ['mov eax, [esi]', 'shr eax, 11', 'imul eax, dword [esi+4]',
                 'cmp eax, [esi+8]', 'mov ebx, [esi+4]', 'mov [edi+4], eax',
                 'mov ecx, [esi+12]', f'jb .range_small_{n}', 'mov ecx, ebx', f'.range_small_{n}:',
                 'sub ecx, eax', 'mov [edi], ecx']
    result['known_flags_range_coder'] = (body, sum(not x.endswith(':') for x in body))
    for suffix in ('ss', 'sd'):
        for source in ('reg', 'memory'):
            body = [f'cvtsi2{suffix} xmm{r}, ' + ('eax' if source == 'reg' else 'dword [esi]')
                    for _ in range(16) for r in range(4)]
            result[f'cvtsi2{suffix}_{source}'] = (body, len(body))
        for mode in ('latency', 'throughput'):
            regs = [0] if mode == 'latency' else range(4)
            body = [instruction for _ in range(16 // len(regs)) for r in regs
                    for reg in [('eax', 'ebx', 'ecx', 'edx')[r]]
                    for instruction in (f'cvtsi2{suffix} xmm{r}, {reg}', f'cvtt{suffix}2si {reg}, xmm{r}')]
            result[f'convert_{suffix}_{mode}'] = (body, len(body))
    for op in ('cvttsd2si', 'cvtsd2si', 'cvttss2si', 'cvtss2si'):
        body = [f'{op} {reg}, xmm{r}' for _ in range(16) for r, reg in enumerate(('eax', 'ebx', 'ecx', 'edx'))]
        result[op] = (body, len(body))
    for suffix, width, table in (('ss', 4, 'lookup_float'), ('sd', 8, 'lookup_double')):
        for mode in ('latency', 'throughput'):
            regs = [0] if mode == 'latency' else range(4)
            body = [f'mov edi, {table}']
            for r in regs:
                reg = ('eax', 'ebx', 'ecx', 'edx')[r]
                body += [f'and {reg}, 63', f'mov{suffix} xmm{r}, [edi+{reg}*{width}]']
            for _ in range(32 // len(regs)):
                for r in regs:
                    reg = ('eax', 'ebx', 'ecx', 'edx')[r]
                    body += [f'cvtt{suffix}2si {reg}, xmm{r}', f'and {reg}, 63',
                             f'mov{suffix} xmm{r}, [edi+{reg}*{width}]']
            body += ['mov edi, scratch']
            result[f'convert_{suffix}_indexed_{mode}'] = (body, len(body))
        body = [f'mov{suffix} xmm6, [{table}_scale]', f'mov{suffix} xmm7, [{table}_bias]']
        for _ in range(16):
            for r, reg in enumerate(('eax', 'ebx', 'ecx', 'edx')):
                body += [f'mov{suffix} xmm{r}, [{table}+{r*width}]', f'mul{suffix} xmm{r}, xmm6',
                         f'add{suffix} xmm{r}, xmm7', f'cvtt{suffix}2si {reg}, xmm{r}', f'mov [edi+{r*4}], {reg}']
        result[f'convert_{suffix}_quantize'] = (body, len(body))
    for upper in ('zero', 'nonzero'):
        for op in ('addsd', 'mulsd'):
            for mode in ('latency', 'throughput'):
                regs = [0] if mode == 'latency' else range(4)
                load = 'movsd' if upper == 'zero' else 'movups'
                body = [f'{load} xmm{r}, [esi]' for r in regs]
                body += [f'{op} xmm{r}, xmm7' for _ in range(64 // len(regs)) for r in regs]
                result[f'scalar_{upper}_{op}_{mode}'] = (body, len(body))
    # Finite geometry arithmetic: loads establish lane state, then a chain
    # computes a norm and normalizes components before storing the result.
    for upper in ('zero', 'nonzero'):
        load = 'movsd' if upper == 'zero' else 'movupd'
        body = ['mov esi, lookup_double']
        for _ in range(16):
            body += [f'{load} xmm0, [esi]', f'{load} xmm1, [esi+8]', f'{load} xmm2, [esi+16]',
                     'mulsd xmm0, xmm0', 'mulsd xmm1, xmm1', 'mulsd xmm2, xmm2',
                     'addsd xmm0, xmm1', 'addsd xmm0, xmm2', 'sqrtsd xmm0, xmm0',
                     'divsd xmm1, xmm0', 'divsd xmm2, xmm0',
                     'movsd [edi], xmm1', 'movsd [edi+8], xmm2']
        result[f'scalar_{upper}_geometry'] = (body, len(body))
    for upper in ('zero', 'nonzero'):
        for mode in ('pair', 'ring8', 'arithmetic'):
            load = 'movsd' if upper == 'zero' else 'movupd'
            body = [f'{load} xmm{r}, [esi+{r*16}]' for r in range(8)]
            if mode == 'arithmetic':
                body += ['movsd xmm0, xmm1', 'addsd xmm0, xmm7', 'movsd xmm1, xmm0'] * 32
            else:
                count = 2 if mode == 'pair' else 8
                body += [f'movsd xmm{r}, xmm{(r-1)%count}' for _ in range(64//count) for r in range(count)]
            result[f'scalar_direct_movsd_{upper}_{mode}'] = (body, len(body))
    for operation in ('movsd', 'cvtsi2sd', 'cvtss2sd'):
        for upper in ('zero', 'nonzero'):
            for mode in ('same_destination', 'independent'):
                regs = [0] if mode == 'same_destination' else range(4)
                body = [f'movups xmm{r}, [esi]' for r in range(4, 8)]
                load = 'movsd' if upper == 'zero' else 'movupd'
                body += [f'{load} xmm{r}, [esi]' for r in regs]
                body += [f'{operation} xmm{r}, ' + ('eax' if operation == 'cvtsi2sd' else f'xmm{r+4}')
                         for _ in range(64 // len(regs)) for r in regs]
                result[f'scalar_direct_{operation}_{upper}_{mode}'] = (body, len(body))
    for upper in ('zero', 'nonzero'):
        for conversion in ('integer', 'single'):
            load = 'movsd' if upper == 'zero' else 'movupd'
            body = [f'{load} xmm{r}, [esi]' for r in range(4)]
            body += ['movsd xmm6, [lookup_double_scale]', 'movsd xmm7, [lookup_double_bias]']
            for _ in range(16):
                for r in range(4):
                    body += ([f'mov eax, [lane_input_integers+{r*4}]', f'cvtsi2sd xmm{r}, eax']
                             if conversion == 'integer' else
                             [f'movss xmm4, [lookup_float+{r*4}]', f'cvtss2sd xmm{r}, xmm4'])
                    body += [f'mulsd xmm{r}, xmm6', f'addsd xmm{r}, xmm7', f'cvttsd2si eax, xmm{r}', f'mov [edi+{r*4}], eax']
            result[f'scalar_direct_{conversion}_{upper}_quantize'] = (body, len(body))
        for mode in ('latency', 'throughput'):
            regs = [0] if mode == 'latency' else range(4)
            load = 'movss' if upper == 'zero' else 'movups'
            body = [f'{load} xmm{r}, [esi]' for r in regs]
            body += [instruction for _ in range(32 // len(regs)) for r in regs
                     for instruction in (f'cvtss2sd xmm{r}, xmm{r}', f'cvtsd2ss xmm{r}, xmm{r}')]
            result[f'scalar_direct_roundtrip_{upper}_{mode}'] = (body, len(body))
    for state in ('zero', 'memory_zero', 'nonzero', 'negative_zero'):
        load = 'movss' if state == 'zero' else 'movq'
        table = 'lane_input_' + ('zero' if state == 'memory_zero' else state)
        for mode in ('affine', 'geometry'):
            body = ['movsd xmm6, [lookup_double_scale]', 'movsd xmm7, [lookup_double_bias]']
            for _ in range(16):
                count = 4 if mode == 'affine' else 3
                body += [instruction for r in range(count) for instruction in
                         (f'{load} xmm{r}, [{table}+{r*8}]', f'cvtps2pd xmm{r}, xmm{r}')]
                if mode == 'affine':
                    body += [instruction for r in range(4) for instruction in
                             (f'mulsd xmm{r}, xmm6', f'addsd xmm{r}, xmm7', f'movsd [edi+{r*8}], xmm{r}')]
                else:
                    body += ['mulsd xmm0, xmm0', 'mulsd xmm1, xmm1', 'mulsd xmm2, xmm2',
                             'addsd xmm0, xmm1', 'addsd xmm0, xmm2', 'sqrtsd xmm0, xmm0',
                             'divsd xmm1, xmm0', 'divsd xmm2, xmm0', 'movsd [edi], xmm1', 'movsd [edi+8], xmm2']
            result[f'scalar_lane_{state}_{mode}'] = (body, len(body))
    for copy in ('movsd', 'movaps', 'movq'):
        body = ['movsd xmm6, [lookup_double_scale]', 'movsd xmm7, [lookup_double_bias]']
        for _ in range(16):
            body += ['movss xmm4, [lane_input_zero]', f'{copy} xmm0, xmm4', 'cvtps2pd xmm0, xmm0']
            body += ['mulsd xmm0, xmm6', 'addsd xmm0, xmm7'] * 4
            body += ['movsd [edi], xmm0']
        result[f'scalar_lane_{copy}_chain'] = (body, len(body))
    for copy in ('movaps', 'movups', 'movapd', 'movupd', 'movdqa', 'movdqu'):
        for upper in ('zero', 'nonzero'):
            load = 'movsd' if upper == 'zero' else 'movupd'
            body = [f'{load} xmm4, [lookup_double]', f'{load} xmm5, [lookup_double+8]',
                    'movsd xmm6, [lookup_double_scale]', 'movsd xmm7, [lookup_double_bias]']
            for _ in range(16):
                body += [f'{copy} xmm0, xmm4', f'{copy} xmm1, xmm5',
                         'mulsd xmm0, xmm6', 'mulsd xmm1, xmm6',
                         'addsd xmm0, xmm7', 'addsd xmm1, xmm7',
                         'movsd [edi], xmm0', 'movsd [edi+8], xmm1']
            result[f'scalar_copy_{copy}_{upper}'] = (body, len(body))
    for upper in ('zero', 'nonzero'):
        load = 'movsd' if upper == 'zero' else 'movupd'
        body = ['mov esi, lookup_double']
        for _ in range(16):
            body += [f'{load} xmm4, [esi]', f'{load} xmm5, [esi+8]', f'{load} xmm6, [esi+16]',
                     'movaps xmm0, xmm4', 'movaps xmm1, xmm5', 'movaps xmm2, xmm6',
                     'xor edx, edx', 'mulsd xmm0, xmm0', 'mulsd xmm1, xmm1', 'mulsd xmm2, xmm2',
                     'addsd xmm0, xmm1', 'addsd xmm0, xmm2', 'sqrtsd xmm0, xmm0',
                     'divsd xmm1, xmm0', 'divsd xmm2, xmm0',
                     'movsd [edi], xmm1', 'movsd [edi+8], xmm2']
        result[f'scalar_copy_geometry_{upper}'] = (body, len(body))
    for barrier in ('add eax, ebx', 'xor eax, ebx', 'cmp eax, ebx', 'test eax, ebx',
                    'cvtsi2sd xmm1, eax', 'cvttsd2si eax, xmm1'):
        body = []
        for _ in range(16):
            body += ['movsd xmm0, [lookup_double]', 'movsd xmm1, [lookup_double+8]',
                     barrier, 'mulsd xmm0, xmm1', 'addsd xmm0, xmm1', 'movsd [edi], xmm0']
        name = barrier.split()[0]
        result[f'scalar_integer_{name}'] = (body, len(body))
    for op in ('addps', 'mulps', 'addss', 'mulss', 'addsd', 'mulsd'):
        body = [f'{op} xmm{r}, xmm7' for _ in range(16) for r in range(4)]
        result[op + '_control'] = (body, len(body))
    for name, body in {
        'vector_load': ['movups xmm0, [esi]', 'movups xmm1, [esi+16]', 'movups xmm2, [esi+32]', 'movups xmm3, [esi+48]'],
        'vector_copy': ['movups xmm0, [esi]', 'movups [edi], xmm0', 'movups xmm1, [esi+16]', 'movups [edi+16], xmm1'],
        'scalar_load': ['mov eax, [esi]', 'mov ebx, [esi+4]', 'mov ecx, [esi+8]', 'mov edx, [esi+12]'],
        'scalar_store': ['mov [edi], eax', 'mov [edi+4], ebx', 'mov [edi+8], ecx', 'mov [edi+12], edx'],
        'double_load': ['movsd xmm0, [esi]', 'movsd xmm1, [esi+8]', 'movsd xmm2, [esi+16]', 'movsd xmm3, [esi+24]'],
        'double_copy': ['movsd xmm0, [esi]', 'movsd [edi], xmm0', 'movsd xmm1, [esi+8]', 'movsd [edi+8], xmm1'],
        'double_spill': ['movsd [edi], xmm0', 'movsd xmm1, [edi]', 'movsd [edi+8], xmm2', 'movsd xmm3, [edi+8]'],
        'double_dot': ['movsd xmm0, [esi]', 'mulsd xmm0, [esi+8]', 'movsd xmm1, [esi+16]',
                       'mulsd xmm1, [esi+24]', 'addsd xmm0, xmm1', 'movsd [edi], xmm0'],
        'double_stack_negative': ['movsd [edi+128-8], xmm0', 'movsd xmm1, [edi+128-8]',
                                  'movsd [edi+128-16], xmm2', 'movsd xmm3, [edi+128-16]'],
        'lea_negative': ['lea eax, [eax-4]', 'lea ebx, [ebx-8]', 'lea ecx, [ecx-64]', 'lea edx, [edx-128]'],
        'lea_positive_control': ['lea eax, [eax+4]', 'lea ebx, [ebx+8]', 'lea ecx, [ecx+64]', 'lea edx, [edx+128]'],
        'lea_mixed': ['lea eax, [ebx+ecx*4-4]', 'lea ebx, [eax+edx*2-8]',
                      'lea ecx, [eax+ebx*8-64]', 'lea edx, [ecx+eax*2-128]'],
        'helper_fsin': ['fld1', 'fsin', 'fstp st0'],
        'helper_cpuid': ['xor eax, eax', 'cpuid'],
    }.items():
        result[name] = (body * 16, len(body) * 16)
    for name in ('vector_load', 'scalar_load', 'double_load'):
        body, _ = result[name]
        remapped = []
        for line in body:
            for offset in (48, 32, 24, 16, 12, 8, 4):
                line = line.replace(f'[esi+{offset}]', f'[esi-{64-offset}]')
            remapped.append(line.replace('[esi]', '[esi-64]'))
        body = ['add esi, 64'] + remapped + ['sub esi, 64']
        result[name + '_negative'] = (body, len(body))
    for name, body in {
        'stack_scalar_spill': ['mov [esp], eax', 'mov ebx, [esp]', 'mov [esp+4], ecx', 'mov edx, [esp+4]'],
        'stack_double_spill': ['movsd [esp], xmm0', 'movsd xmm1, [esp]', 'movsd [esp+8], xmm2', 'movsd xmm3, [esp+8]'],
        'stack_vector_spill': ['movups [esp], xmm0', 'movups xmm1, [esp]', 'movups [esp+16], xmm2', 'movups xmm3, [esp+16]'],
    }.items():
        body = ['sub esp, 128'] + body * 16 + ['add esp, 128']
        result[name] = (body, len(body))
    for width, opcode in ((1, 'movzx'), (2, 'movzx'), (4, 'mov')):
        size = {1: 'byte', 2: 'word', 4: 'dword'}[width]
        body = [f'{opcode} {reg}, {size} [esi]' for _ in range(16) for reg in ('eax', 'ebx', 'ecx', 'edx')]
        result[f'borrow_load{width}_throughput'] = (body, len(body))
    for mode in ('latency', 'throughput'):
        regs = ['eax'] if mode == 'latency' else ['eax', 'ebx', 'ecx', 'edx']
        body = [f'mov {reg}, pointer_nodes+{i*4}' for i, reg in enumerate(regs)]
        body += [f'mov {reg}, [{reg}]' for _ in range(64 // len(regs)) for reg in regs]
        result[f'borrow_pointer_{mode}'] = (body, len(body))
    for base in ('esi', 'edi', 'esp', 'ebp'):
        for displacement in (0, 4):
            address = f'[{base}' + (f'+{displacement}]' if displacement else ']')
            for mode in ('independent', 'dependent'):
                if mode == 'independent':
                    body = [f'mov {address}, eax', f'mov ebx, {address}', f'mov {address}, ecx', f'mov edx, {address}'] * 16
                else:
                    body = [f'mov {address}, eax', f'mov eax, {address}', 'add eax, 1'] * 32
                if base == 'esp': body = ['sub esp, 128'] + body + ['add esp, 128']
                if base == 'ebp': body = ['push ebp', 'mov ebp, esi'] + body + ['pop ebp']
                result[f'borrow_forward_{base}_{displacement}_{mode}'] = (body, len(body))
    for mode in ('scalar', 'double', 'vector'):
        if mode == 'scalar':
            body = ['mov [esp+16], eax', 'mov ebx, [esp+16]'] * 32
        elif mode == 'double':
            body = ['movsd [esp+16], xmm0', 'movsd xmm1, [esp+16]'] * 32
        else:
            body = ['movups [esp+16], xmm0', 'movups xmm1, [esp+16]'] * 32
        body = ['mov [mapped_stack_saved_sp], esp', 'mov esp, [mapped_stack_top]', 'sub esp, 128'] + body + ['mov esp, [mapped_stack_saved_sp]']
        result[f'mmap_stack_{mode}_spill'] = (body, len(body))
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output', type=Path)
    parser.add_argument('--nasm', default='nasm')
    parser.add_argument('--iterations', type=int, default=2_000_000)
    parser.add_argument('--preheat-ms', type=int, default=200, help='Guest CPU preheat before kernel warmup, excluded from timings')
    parser.add_argument('--warmup-iterations', type=int, default=10_000)
    args = parser.parse_args()
    if args.output.exists() and any(args.output.iterdir()):
        parser.error('output must be a new or empty directory; existing results are preserved')
    if not 1 <= args.iterations <= 0xffffffff or not 1 <= args.warmup_iterations <= 0xffffffff:
        parser.error('iteration and warmup counts must be between 1 and 4294967295')
    if not 0 <= args.preheat_ms <= 2000:
        parser.error('--preheat-ms must be between 0 and 2000')
    args.output.mkdir(parents=True, exist_ok=True)
    template = Path(__file__).with_name('kernel.asm').read_text()
    data = []
    for label, directive in (('lookup_float', 'dd'), ('lookup_double', 'dq')):
        data += [label + ':', '    ' + directive + ' ' + ', '.join(f'{(i+17)%64}.0' for i in range(64)),
                 label + '_scale: ' + directive + ' 0.75', label + '_bias: ' + directive + ' 0.25']
    data += ['align 16, db 0', 'pointer_nodes:', '    dd ' + ', '.join(f'pointer_nodes+{((i+17)%64)*4}' for i in range(64))]
    for state, high in [('zero', '0'), ('nonzero', '0x40000000'), ('negative_zero', '0x80000000')]:
        data += ['align 16, db 0', 'lane_input_' + state + ':',
                 '    dd ' + ', '.join(value for i in range(4) for value in (f'{17+i}.0', high))]
    data += ['lane_input_integers: dd 17, 18, 19, 20']
    template = template.replace('; BENCHMARK_DATA', '\n'.join(data))
    manifest = {}
    for name, (body, operations) in cases().items():
        iterations = min(args.iterations, 20_000) if name.startswith('helper_') else args.iterations
        if name == 'double_stack_negative':
            body = ['add edi, 128'] + [line.replace('[edi+128-', '[edi-') for line in body] + ['sub edi, 128']
            operations = len(body)
        source = args.output / (name + '.asm')
        binary = args.output / name
        setup = ''
        if name.startswith('mmap_stack_'):
            setup = '''mov eax, 192
    xor ebx, ebx
    mov ecx, 0x401000
    xor edx, edx
    mov esi, 0x20022
    mov edi, -1
    xor ebp, ebp
    int 0x80
    cmp eax, -4095
    jae failed_start
    lea ebx, [eax+0x1000]
    add eax, 0x401000
    mov [mapped_stack_top], eax
    mov eax, 125
    mov ecx, 0x400000
    mov edx, 3
    int 0x80
    test eax, eax
    jnz failed_start'''
        source.write_text(template.replace('; BENCHMARK_BODY', '\n    '.join(body)).replace('; BENCHMARK_SETUP', setup))
        subprocess.run([args.nasm, '-f', 'bin', '-DITERATIONS=' + str(iterations), '-DPREHEAT_NS=' + str(args.preheat_ms * 1_000_000), '-DWARMUP_ITERATIONS=' + str(args.warmup_iterations), '-o', str(binary), str(source)], check=True)
        binary.chmod(0o755)
        manifest[name] = {'iterations': iterations, 'operations_per_iteration': operations, 'result_bytes': 432, 'preheat_ms': args.preheat_ms, 'warmup_iterations': args.warmup_iterations, 'body': body}
    (args.output / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    print(f'Generated {len(manifest)} kernels in {args.output}')


if __name__ == '__main__':
    main()
