"""Build/stage the x86 forwarding checkpoint; never overwrite supplied game files."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import urllib.request
import zipfile
from generate import generate
from build_config import (ROOT, BUILD, STAGE, GAME_ARCHIVE, add_game_files_argument,
                          require_files, toolchain, system_ddraw)

SDL_URL = 'https://github.com/libsdl-org/SDL/releases/download/release-3.4.18/SDL3-3.4.18-win32-x86.zip'
SDL_SHA = '99f2ee6e126d828c5e5b3d20201930f3d16a8c6e147d1de90a59f4ae6df138c9'


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def fetch_verified(url, destination, expected):
    if not destination.exists():
        temporary = destination.with_suffix(destination.suffix + '.part')
        try:
            print('Downloading ' + destination.name)
            urllib.request.urlretrieve(url, temporary)
            if sha(temporary) != expected:
                raise SystemExit('Downloaded file failed SHA-256 verification: ' + destination.name)
            temporary.replace(destination)
        finally:
            temporary.unlink(missing_ok=True)
    if sha(destination) != expected:
        raise SystemExit('Cached file failed SHA-256 verification; remove it and retry: ' + str(destination))


def run(*args):
    subprocess.run([str(a) for a in args], cwd=ROOT, check=True)


def main():
    parser = argparse.ArgumentParser(description='Build a complete x86 Squeezils patch ZIP.')
    parser.add_argument('--stage', action='store_true', help='also prepare build/forward/game from the original ZIP')
    parser.add_argument('--sdl', action='store_true', help=argparse.SUPPRESS)  # Old commands remain valid.
    add_game_files_argument(parser)
    args = parser.parse_args()
    gcc, sdk = toolchain()
    system = system_ddraw()
    game_files = args.game_files.expanduser().resolve()
    legacy = game_files / 'd3drm.dll'
    required = [legacy]
    if args.stage:
        required.append(game_files / GAME_ARCHIVE)
    require_files(required, 'Supply original game files, or select their directory with --game-files PATH.')
    BUILD.mkdir(parents=True, exist_ok=True)
    headers = BUILD / 'sdl-headers/include'
    if not (headers / 'SDL3/SDL.h').exists():
        devel = BUILD / 'SDL3-devel-3.4.18-VC.zip'
        fetch_verified('https://github.com/libsdl-org/SDL/releases/download/release-3.4.18/' + devel.name,
                       devel, '78d84602ae616cfe26b33a73b7d3b9b6b56e8a6650e53d92c9c6ca938f747acd')
        with zipfile.ZipFile(devel) as z:
            for name in z.namelist():
                if '/include/' in name and not name.endswith('/'):
                    target = BUILD / 'sdl-headers' / Path(name).relative_to(Path(name).parts[0])
                    target.parent.mkdir(parents=True, exist_ok=True)
                    target.write_bytes(z.read(name))
    archive = BUILD / 'SDL3-3.4.18-win32-x86.zip'
    fetch_verified(SDL_URL, archive, SDL_SHA)
    with zipfile.ZipFile(archive) as z:
        for name in z.namelist():
            if Path(name).name in ('SDL3.dll', 'LICENSE.txt'):
                (BUILD / Path(name).name).write_bytes(z.read(name))
    record = {'version': '3.4.18', 'architecture': 'x86', 'url': SDL_URL,
              'archive_sha256': SDL_SHA, 'dll_sha256': sha(BUILD / 'SDL3.dll'),
              'headers_url': 'https://github.com/libsdl-org/SDL/releases/download/release-3.4.18/SDL3-devel-3.4.18-VC.zip',
              'headers_archive_sha256': '78d84602ae616cfe26b33a73b7d3b9b6b56e8a6650e53d92c9c6ca938f747acd'}
    (ROOT / 'third_party/SDL3').mkdir(parents=True, exist_ok=True)
    (ROOT / 'third_party/SDL3/source.json').write_text(json.dumps(record, indent=2) + '\n')
    generate(sdk, BUILD / 'generated', system, legacy)
    flags = ['-m32', '-static-libgcc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror=incompatible-pointer-types', '-I', ROOT / 'src', '-I', headers]
    color_fixes = ['-DSQZ_FIX_16BPP_RGB_INDEX_REMAP', '-DSQZ_FIX_16BPP_DESKTOP_GATE', '-DSQZ_FIX_16BPP_COLORKEY_MASK', '-DSQZ_FIX_16BPP_WARP_BRIDGE']
    run(gcc, *flags, *color_fixes, '-shared', ROOT / 'src/runtime.c', ROOT / 'src/platform.c', ROOT / 'src/color_compat.c', ROOT / 'src/warp_compat.c',
        ROOT / 'src/save_compat.c', ROOT / 'src/input_config.c', ROOT / 'src/scene_cache.c', ROOT / 'src/scene_extract.c', ROOT / 'src/gpu_bridge.c', ROOT / 'src/gpu_render.c', BUILD / 'generated/observers.c',
        '-o', BUILD / 'sqz-runtime.dll', '-Wl,--out-implib,' + str(BUILD / 'libsqz-runtime.a'), '-ldxguid', '-lole32', '-luser32', '-lgdi32')
    for label in ('ddraw', 'd3drm'):
        run(gcc, '-m32', '-shared', BUILD / ('generated/' + label + '.S'),
            BUILD / ('generated/' + label + '.def'), BUILD / 'libsqz-runtime.a', '-o', BUILD / (label + '.dll'))
    if args.stage:
        STAGE.mkdir(parents=True, exist_ok=True)
        archive = game_files / GAME_ARCHIVE
        with zipfile.ZipFile(archive) as z:
            for item in z.infolist():
                relative = Path(*Path(item.filename).parts[1:])
                if not relative.parts:
                    continue
                target = (STAGE / relative).resolve()
                if not target.is_relative_to(STAGE.resolve()):
                    raise SystemExit('Unsafe archive path: ' + item.filename)
                if item.is_dir():
                    target.mkdir(parents=True, exist_ok=True)
                    continue
                target.parent.mkdir(parents=True, exist_ok=True)
                if not target.exists():
                    target.write_bytes(z.read(item))
        if sha(STAGE / 'SQZ.EXE') != '801931afc6ca09fdfffafb9c2f7223b2417b5c3895f57005091a817f3ec15194':
            raise SystemExit('Staged SQZ.EXE does not match the supported v0.42b demo.')
        shutil.copy2(legacy, STAGE / 'd3drm.dll')
        for label in ('ddraw', 'sqz-runtime'):
            shutil.copy2(BUILD / (label + '.dll'), STAGE / (label + '.dll'))
        shutil.copy2(ROOT / 'tools/play-native.bat', STAGE / 'play-native.bat')
        shutil.copy2(BUILD / 'SDL3.dll', STAGE / 'SDL3.dll')
        shutil.copy2(BUILD / 'LICENSE.txt', STAGE / 'SDL3-LICENSE.txt')
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
    print('Patch ZIP: ' + str(BUILD / 'squeezils-patch.zip'))
    print('Patch files: ' + str(BUILD / 'patch'))
    if args.stage:
        print('Playable copy: ' + str(STAGE))


if __name__ == '__main__':
    main()
