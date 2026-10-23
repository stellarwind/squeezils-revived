"""Build/stage the x86 forwarding checkpoint; never overwrite supplied game files."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import urllib.request
import zipfile
from generate import generate

ROOT = Path(__file__).resolve().parents[1]
WORKSPACE = ROOT.parent
BUILD = ROOT / 'build' / 'forward'
STAGE = BUILD / 'game'
SDL_URL = 'https://github.com/libsdl-org/SDL/releases/download/release-3.4.18/SDL3-3.4.18-win32-x86.zip'
SDL_SHA = '99f2ee6e126d828c5e5b3d20201930f3d16a8c6e147d1de90a59f4ae6df138c9'


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def run(*args):
    subprocess.run([str(a) for a in args], cwd=ROOT, check=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--stage', action='store_true', help='extract original ZIP into build/forward/game')
    parser.add_argument('--sdl', action='store_true', help='download pinned official x86 SDL3 runtime')
    args = parser.parse_args()
    gcc = Path(shutil.which('gcc') or 'D:/Decomp/mingw32/bin/gcc.exe')
    sdk = gcc.parent.parent / 'i686-w64-mingw32' / 'include'
    if subprocess.check_output([gcc, '-dumpmachine'], text=True).strip() != 'i686-w64-mingw32':
        raise SystemExit('An i686-w64-mingw32 GCC is required.')
    system = Path(os.environ['WINDIR']) / ('SysWOW64' if sys.maxsize > 2**32 else 'System32') / 'ddraw.dll'
    legacy = WORKSPACE / 'SqueezilsGameFiles' / 'd3drm.dll'
    BUILD.mkdir(parents=True, exist_ok=True)
    headers = BUILD / 'sdl-headers/include'
    if not (headers / 'SDL3/SDL.h').exists():
        devel = BUILD / 'SDL3-devel-3.4.18-VC.zip'
        if not devel.exists():
            urllib.request.urlretrieve('https://github.com/libsdl-org/SDL/releases/download/release-3.4.18/' + devel.name, devel)
        assert sha(devel) == '78d84602ae616cfe26b33a73b7d3b9b6b56e8a6650e53d92c9c6ca938f747acd'
        with zipfile.ZipFile(devel) as z:
            for name in z.namelist():
                if '/include/' in name and not name.endswith('/'):
                    target = BUILD / 'sdl-headers' / Path(name).relative_to(Path(name).parts[0])
                    target.parent.mkdir(parents=True, exist_ok=True)
                    target.write_bytes(z.read(name))
    manifest = generate(sdk, BUILD / 'generated', system, legacy)
    flags = ['-m32', '-static-libgcc', '-std=c11', '-O2', '-g', '-Wall', '-Wextra', '-Werror=incompatible-pointer-types', '-I', ROOT / 'src', '-I', headers]
    color_fixes = ['-DSQZ_FIX_16BPP_RGB_INDEX_REMAP', '-DSQZ_FIX_16BPP_DESKTOP_GATE', '-DSQZ_FIX_16BPP_COLORKEY_MASK', '-DSQZ_FIX_16BPP_WARP_BRIDGE']
    run(gcc, *flags, *color_fixes, '-shared', ROOT / 'src/runtime.c', ROOT / 'src/platform.c', ROOT / 'src/color_compat.c', ROOT / 'src/warp_compat.c',
        ROOT / 'src/save_compat.c', ROOT / 'src/scene_cache.c', ROOT / 'src/scene_extract.c', ROOT / 'src/gpu_bridge.c', ROOT / 'src/gpu_render.c', ROOT / 'src/shading_capture.c', BUILD / 'generated/observers.c',
        '-o', BUILD / 'sqz-runtime.dll', '-Wl,--out-implib,' + str(BUILD / 'libsqz-runtime.a'), '-ldxguid', '-lole32', '-luser32', '-lgdi32')
    for label in ('ddraw', 'd3drm'):
        run(gcc, '-m32', '-shared', BUILD / ('generated/' + label + '.S'),
            BUILD / ('generated/' + label + '.def'), BUILD / 'libsqz-runtime.a', '-o', BUILD / (label + '.dll'))
    if args.stage:
        STAGE.mkdir(parents=True, exist_ok=True)
        archive = WORKSPACE / 'SqueezilsGameFiles/Squeezils_Win_EN_v042b_updated.zip'
        with zipfile.ZipFile(archive) as z:
            for item in z.infolist():
                relative = Path(*Path(item.filename).parts[1:])
                if not relative.parts:
                    continue
                target = (STAGE / relative).resolve()
                assert target.is_relative_to(STAGE.resolve()), item.filename
                if item.is_dir():
                    target.mkdir(parents=True, exist_ok=True)
                    continue
                target.parent.mkdir(parents=True, exist_ok=True)
                if not target.exists():
                    target.write_bytes(z.read(item))
        assert sha(STAGE / 'SQZ.EXE') == '801931afc6ca09fdfffafb9c2f7223b2417b5c3895f57005091a817f3ec15194'
        shutil.copy2(legacy, STAGE / 'd3drm.dll')
        for label in ('ddraw', 'sqz-runtime'):
            shutil.copy2(BUILD / (label + '.dll'), STAGE / (label + '.dll'))
        shutil.copy2(ROOT / 'tools/play-native.bat', STAGE / 'play-native.bat')
    if args.sdl:
        archive = BUILD / 'SDL3-3.4.18-win32-x86.zip'
        if not archive.exists():
            urllib.request.urlretrieve(SDL_URL, archive)
        assert sha(archive) == SDL_SHA, 'SDL download does not match official GitHub asset digest'
        with zipfile.ZipFile(archive) as z:
            for name in z.namelist():
                if Path(name).name in ('SDL3.dll', 'LICENSE.txt'):
                    (BUILD / Path(name).name).write_bytes(z.read(name))
        if STAGE.exists():
            shutil.copy2(BUILD / 'SDL3.dll', STAGE / 'SDL3.dll')
            shutil.copy2(BUILD / 'LICENSE.txt', STAGE / 'SDL3-LICENSE.txt')
        record = {'version': '3.4.18', 'architecture': 'x86', 'url': SDL_URL,
                  'archive_sha256': SDL_SHA, 'dll_sha256': sha(BUILD / 'SDL3.dll'),
                  'headers_url': 'https://github.com/libsdl-org/SDL/releases/download/release-3.4.18/SDL3-devel-3.4.18-VC.zip',
                  'headers_archive_sha256': '78d84602ae616cfe26b33a73b7d3b9b6b56e8a6650e53d92c9c6ca938f747acd'}
        (ROOT / 'third_party/SDL3').mkdir(parents=True, exist_ok=True)
        (ROOT / 'third_party/SDL3/source.json').write_text(json.dumps(record, indent=2) + '\n')
    if (BUILD / 'SDL3.dll').exists():
        patch = BUILD / 'patch'
        patch.mkdir(exist_ok=True)
        files = {'ddraw.dll': 'ddraw.dll', 'sqz-runtime.dll': 'sqz-runtime.dll',
                 'SDL3.dll': 'SDL3.dll', 'LICENSE.txt': 'SDL3-LICENSE.txt'}
        with zipfile.ZipFile(BUILD / 'squeezils-patch.zip', 'w', zipfile.ZIP_DEFLATED) as archive:
            for source, target in files.items():
                shutil.copy2(BUILD / source, patch / target)
                archive.write(patch / target, target)
            shutil.copy2(legacy, patch / 'd3drm.dll')
            archive.write(patch / 'd3drm.dll', 'd3drm.dll')
            shutil.copy2(ROOT / 'tools/play-native.bat', patch / 'play-native.bat')
            archive.write(patch / 'play-native.bat', 'play-native.bat')
        with zipfile.ZipFile(BUILD / 'squeezils-diagnostic-patch.zip', 'w', zipfile.ZIP_DEFLATED) as archive:
            for path in [*(patch / target for target in files.values()), patch / 'd3drm.dll']:
                archive.write(path, path.name)
            for name in ('diagnose.cmd', 'diagnose.ps1'):
                archive.write(ROOT / 'tools' / name, name)
        with zipfile.ZipFile(BUILD / 'squeezils-sdlgpu-experiment.zip', 'w', zipfile.ZIP_DEFLATED) as archive:
            for path in [*(patch / target for target in files.values()), patch / 'd3drm.dll']:
                archive.write(path, path.name)
            archive.write(ROOT / 'tools/play-gpu.cmd', 'play-gpu.cmd')
    print(json.dumps({'interfaces': len(manifest), 'methods': sum(map(len, manifest.values())),
                      'build': str(BUILD), 'stage': str(STAGE) if args.stage else None}, indent=2))


if __name__ == '__main__':
    main()
