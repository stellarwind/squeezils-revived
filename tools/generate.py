"""Generate typed observers from the installed MinGW SDK, not guessed vtables."""
import json
import re
import struct
from pathlib import Path


def exports(path):
    data = Path(path).read_bytes()
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    sections, optional = struct.unpack_from('<H', data, pe + 6)[0], pe + 24
    assert struct.unpack_from('<H', data, optional)[0] == 0x10b, 'requires PE32 x86'
    size = struct.unpack_from('<H', data, pe + 20)[0]
    table = []
    for i in range(sections):
        p = optional + size + i * 40
        virtual_size, va, raw_size, raw = struct.unpack_from('<IIII', data, p + 8)
        table.append((va, max(virtual_size, raw_size), raw))

    def offset(va):
        for start, length, raw in table:
            if start <= va < start + length:
                return raw + va - start
        raise ValueError(hex(va))

    directory = offset(struct.unpack_from('<I', data, optional + 96)[0])
    base, functions, count, eat, names, ordinals = struct.unpack_from('<IIIIII', data, directory + 16)
    result = []
    for i in range(count):
        p = offset(struct.unpack_from('<I', data, offset(names) + i * 4)[0])
        name = data[p:data.index(b'\0', p)].decode('ascii')
        ordinal = base + struct.unpack_from('<H', data, offset(ordinals) + i * 2)[0]
        result.append((name, ordinal))
    assert functions == count, 'unnamed ordinal exports need explicit handling'
    return result


def generate(sdk, output, system_ddraw, legacy_rm):
    output = Path(output)
    output.mkdir(parents=True, exist_ok=True)
    interfaces = {}
    texts = [(Path(sdk) / name).read_text() for name in ('ddraw.h', 'd3d.h', 'd3drm.h', 'd3drmobj.h')]
    for text in texts:
        text = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
        for name, body in re.findall(r'DECLARE_INTERFACE_\(\s*(\w+)\s*,\s*\w+\s*\)\s*\{(.*?)\};', text, re.S):
            methods = []
            for explicit, default, params in re.findall(r'STDMETHOD(?:_\(([^)]+)\)|\((\w+)\))\s*\(([^;]*?)\)\s*(?:PURE\s*)?;', body, re.S):
                ret, method = [x.strip() for x in explicit.split(',', 1)] if explicit else ('HRESULT', default)
                params = re.sub(r'\s+', ' ', params).strip()
                params = re.sub(r'^THIS_\s*', '', params) if params.startswith('THIS_') else ''
                args = []
                for i, declaration in enumerate(params.split(',') if params else []):
                    declaration = declaration.strip()
                    match = re.match(r'(.+?[\s*])([A-Za-z_]\w*)$', declaration)
                    if match:
                        typ, arg = match.groups()
                    else:
                        typ, arg = declaration + ' ', 'arg' + str(i)
                    args.append((typ.strip(), 'sqz_arg_' + str(i)))
                methods.append((ret, method, args))
            assert methods and methods[0][1] == 'QueryInterface', name
            interfaces[name] = methods

    # IUnknown is only used for identity; an existing fuller view is never downgraded.
    interfaces['IUnknown'] = [('HRESULT', 'QueryInterface', [('REFIID', 'riid'), ('void **', 'out')]),
                              ('ULONG', 'AddRef', []), ('ULONG', 'Release', [])]
    code = ['#include <initguid.h>', '#include "scene_gpu.h"', '#include <stddef.h>']
    manifest = {}
    for name, methods in interfaces.items():
        manifest[name] = [method for _, method, _ in methods]
        for slot, (ret, method, args) in enumerate(methods):
            declarations = ', '.join([name + ' *self'] + [typ + ' ' + arg for typ, arg in args])
            actual = ', '.join(['self'] + [arg for _, arg in args])
            invocation = f'original->{method}({actual})'
            if name == 'IDirect3DRMViewport' and method == 'Render':
                invocation = f'sqz_gpu_render(original, {actual})'
            if name == 'IDirectDrawSurface' and method == 'Blt':
                invocation = f'sqz_warp_blt(original, {actual})'
            if name == 'IDirectDraw' and method == 'SetCooperativeLevel':
                invocation = f'sqz_cooperative(original, {actual})'
            if name == 'IDirectDraw' and method == 'SetDisplayMode':
                invocation = f'sqz_display_mode(original, {actual})'
            if name == 'IDirectDraw' and method == 'GetCaps':
                invocation = f'sqz_get_caps(original, {actual})'
            if name == 'IDirectDraw' and method == 'CreateSurface':
                invocation = f'sqz_warp_create_surface(original, {actual}, __builtin_return_address(0))'
            if name == 'IDirectDrawSurface' and method == 'ReleaseDC':
                invocation = f'sqz_release_dc(original, {actual})'
            code += [f'static {ret} WINAPI hook_{name}_{method}({declarations}) {{',
                     f'    const {name}Vtbl *original = sqz_original(self);']
            code += [f'    {ret} result = {invocation};']
            if name in ('IDirect3DRMMesh', 'IDirect3DRMMeshBuilder', 'IDirect3DRMFace') and method != 'AddRef' and ret in ('HRESULT', 'int') and (
                    method.startswith(('Set', 'Add', 'Delete')) or
                    method in ('Scale', 'Translate', 'Load', 'GenerateNormals', 'Optimize', 'Empty')):
                owner = 'NULL' if name == 'IDirect3DRMFace' else 'self'
                code += [f'    if (SUCCEEDED(result)) sqz_scene_geometry_changed({owner});']
            if name == 'IDirect3DRM' and method == 'CreateDeviceFromSurface':
                code += ['    if (SUCCEEDED(result) && sqz_arg_3) sqz_gpu_bind_device(*sqz_arg_3, sqz_arg_2);']
            if ret == 'HRESULT':
                code += ['    if (SUCCEEDED(result)) {']
                iid = next((arg for typ, arg in args if typ == 'REFIID'), None)
                for typ, arg in args:
                    direct = re.fullmatch(r'(?:struct\s+)?(IDirect\w+)\s*\*\s*\*', typ)
                    alias = re.fullmatch(r'LP(DIRECT\w+)\s*\*', typ)
                    target = direct.group(1) if direct else None
                    if alias:
                        target = next((n for n in interfaces if n.upper() == 'I' + alias.group(1)), None)
                    if target:
                        code.append(f'        if ({arg}) sqz_hook(*{arg}, "{target}");')
                    elif iid and re.fullmatch(r'void\s*\*\s*\*', typ):
                        code.append(f'        if ({arg}) sqz_hook_iid(*{arg}, {iid});')
                code += ['    }']
            if method == 'Release':
                if name == 'IDirectDrawSurface':
                    code += ['    sqz_warp_surface_released(self, result);']
                code += ['    sqz_surface_released(self, result);']
                code += ['    if (!result) sqz_forget(self);']
            code += ['    return result;', '}']
        code += [f'static const {name}Vtbl table_{name} = {{' + ','.join(f'hook_{name}_{m}' for _, m, _ in methods) + '};',
                 f'_Static_assert(sizeof({name}Vtbl) == {len(methods)} * sizeof(void *), "{name} slots");']
    code += ['const SqzInterface sqz_interfaces[] = {']
    for name, methods in interfaces.items():
        iid = 'NULL' if name == 'IDirect3DRMArray' else '&IID_' + name
        code += [f'{{ {iid}, "{name}", {len(methods)}, &table_{name} }},']
    code += ['};', 'const size_t sqz_interface_count = sizeof(sqz_interfaces)/sizeof(sqz_interfaces[0]);']
    (output / 'observers.c').write_text('\n'.join(code) + '\n')
    (output / 'interfaces.json').write_text(json.dumps(manifest, indent=2) + '\n')
    for library, path in enumerate((system_ddraw, legacy_rm)):
        label = ('ddraw', 'd3drm')[library]
        assembly = ['.text']
        definition = ['LIBRARY ' + label, 'EXPORTS']
        for name, ordinal in exports(path):
            assert re.fullmatch(r'\w+', name), name
            # Save flags and all integer registers before calling our C resolver.
            # The export's original arguments and x87 stack remain untouched.
            assembly += [f'.data\n.align 4\np_{name}: .long 0',
                         f'.section .rdata\nn_{name}: .asciz "{name}"', '.text',
                         f'.globl _{name}\n_{name}:', 'pushfl', 'pushal',
                         f'pushl $n_{name}', f'pushl ${library}', 'call _sqz_resolve', 'addl $8, %esp',
                         f'movl %eax, p_{name}', 'popal', 'popfl', f'jmp *p_{name}']
            definition += [f'    {name} @{ordinal}']
        (output / (label + '.S')).write_text('\n'.join(assembly) + '\n')
        (output / (label + '.def')).write_text('\n'.join(definition) + '\n')
    return manifest
