"""Stage the stock game with only its missing-Z-depth fallback byte patch."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import struct
import zipfile

ROOT = Path(__file__).resolve().parents[1]
STOCK_SHA256 = '801931afc6ca09fdfffafb9c2f7223b2417b5c3895f57005091a817f3ec15194'
PATCH_ADDRESS = 0x475DB7
BEFORE = bytes.fromhex('68 80 18 4c 00 56 e8')
AFTER = bytes.fromhex('bd 10 00 00 00 eb 14')


def patch_depth(original):
    if hashlib.sha256(original).hexdigest() != STOCK_SHA256:
        raise ValueError('Expected the verified stock SQZ.EXE; refusing to patch another binary.')
    pe = struct.unpack_from('<I', original, 0x3c)[0]
    count = struct.unpack_from('<H', original, pe + 6)[0]
    optional_size = struct.unpack_from('<H', original, pe + 20)[0]
    image_base = struct.unpack_from('<I', original, pe + 24 + 28)[0]
    rva = PATCH_ADDRESS - image_base
    for index in range(count):
        section = pe + 24 + optional_size + index * 40
        _, virtual, raw_size, raw = struct.unpack_from('<IIII', original, section + 8)
        if virtual <= rva and rva + len(BEFORE) <= virtual + raw_size:
            offset = raw + rva - virtual
            break
    else:
        raise ValueError('Depth fallback address has no file-backed section.')
    assert original[offset:offset + len(BEFORE)] == BEFORE
    patched = original[:offset] + AFTER + original[offset + len(BEFORE):]
    assert len(patched) == len(original)
    assert patched[:offset] == original[:offset]
    assert patched[offset + len(AFTER):] == original[offset + len(BEFORE):]
    return patched, offset


def prepare(output):
    output.mkdir(parents=True, exist_ok=True)
    forbidden = ('ddraw.dll', 'sqz-runtime.dll', 'SDL3.dll')
    if any((output / name).exists() for name in forbidden):
        raise ValueError('Benchmark directory contains replacement DLLs; choose a fresh directory.')
    source = ROOT.parent / 'SqueezilsGameFiles'
    with zipfile.ZipFile(source / 'Squeezils_Win_EN_v042b_updated.zip') as archive:
        original = archive.read('Squeezils/SQZ.EXE')
        patched, offset = patch_depth(original)
        for item in archive.infolist():
            parts = Path(item.filename).parts
            if not parts or parts[0] != 'Squeezils' or len(parts) < 2:
                continue
            relative = Path(*parts[1:])
            if relative.name.lower().endswith('.dll'):
                raise ValueError(f'Unexpected DLL in original archive: {relative}')
            target = (output / relative).resolve()
            assert target.is_relative_to(output.resolve())
            if item.is_dir():
                target.mkdir(parents=True, exist_ok=True)
            elif relative.as_posix().lower() == 'sqz.exe':
                if target.exists() and target.read_bytes() != patched:
                    raise FileExistsError(f'Refusing to overwrite another executable: {target}')
                target.write_bytes(patched)
            elif not target.exists():
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(archive.read(item))
    native_rm = source / 'd3drm.dll'
    target_rm = output / 'd3drm.dll'
    if target_rm.exists() and target_rm.read_bytes() != native_rm.read_bytes():
        raise ValueError('Benchmark directory contains a different d3drm.dll.')
    shutil.copy2(native_rm, target_rm)
    saves = output / 'SQZ/SAVE'
    saves.mkdir(parents=True, exist_ok=True)
    for save in (ROOT / 'test-saves/SAVE').iterdir():
        target = saves / save.name
        if not target.exists():
            shutil.copy2(save, target)
    report = dict(stock_sha256=STOCK_SHA256,
                  patched_sha256=hashlib.sha256(patched).hexdigest(),
                  patch_address=hex(PATCH_ADDRESS), file_offset=hex(offset),
                  before=BEFORE.hex(' '), after=AFTER.hex(' '),
                  change='Only when no Z depths are reported: select 16-bit depth instead of failing.',
                  native_rm_sha256=hashlib.sha256(native_rm.read_bytes()).hexdigest(),
                  executable=str(output / 'SQZ.EXE'))
    (output / 'benchmark-build.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'build/native-benchmark')
    prepare(parser.parse_args().output.resolve())
