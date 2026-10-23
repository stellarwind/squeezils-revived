import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import zipfile
from build_config import (ROOT, BUILD, GAME_ARCHIVE, add_game_files_argument, require_files,
                          toolchain, system_ddraw)


def prepare_assets(game_files):
    """Copy only test inputs; never depend on or modify a staged installation."""
    names = ('SQZ.EXE', 'SQZ/GOODIES/NUT1.SQG', 'SQZ/GOODIES/SPIT.SQG')
    contents = {}
    missing = []
    for name in names:
        path = game_files / name
        if path.is_file():
            contents[name] = path.read_bytes()
        else:
            missing.append(name)
    if missing:
        archive = game_files / GAME_ARCHIVE
        require_files([archive], 'Tests need the original ZIP, or extracted SQZ.EXE and SQZ/GOODIES files in --game-files PATH.')
        with zipfile.ZipFile(archive) as z:
            for name in missing:
                matches = [item for item in z.namelist()
                           if item.replace('\\', '/').upper() == name
                           or item.replace('\\', '/').upper().endswith('/' + name)]
                if len(matches) != 1:
                    raise SystemExit('Expected one ' + name + ' in ' + str(archive))
                contents[name] = z.read(matches[0])
    destination = BUILD / 'test-assets'
    destination.mkdir(parents=True, exist_ok=True)
    paths = []
    for name in names:
        path = destination / Path(name).name
        path.write_bytes(contents[name])
        paths.append(path)
    return paths


def main():
    parser = argparse.ArgumentParser(description='Run patch tests; no staged game is needed.')
    add_game_files_argument(parser)
    args = parser.parse_args()
    gcc, _ = toolchain()
    game_files = args.game_files.expanduser().resolve()
    require_files([game_files / 'd3drm.dll'],
                  'Supply the original d3drm.dll in --game-files PATH.')
    require_files([BUILD / name for name in ('ddraw.dll', 'd3drm.dll', 'sqz-runtime.dll', 'SDL3.dll',
                                           'sdl-headers/include/SDL3/SDL.h')]
                  + [BUILD / 'patch' / name for name in ('ddraw.dll', 'sqz-runtime.dll', 'SDL3.dll', 'd3drm.dll')],
                  'Build the patch first: python tools/build.py --game-files "' + str(game_files) + '"')
    original_exe, nut_model, spit_model = prepare_assets(game_files)
    subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra', '-I', str(ROOT / 'src'),
                    '-I', str(BUILD / 'sdl-headers/include'), str(ROOT / 'tests/frame_pixels.c'), str(ROOT / 'src/scene_cache.c'),
                    '-ldxguid', '-lgdi32', '-o', str(BUILD / 'frame_pixels.exe')], check=True)
    subprocess.run([str(BUILD / 'frame_pixels.exe')], check=True, timeout=10)
    save_probe = BUILD / 'save-compat-test'
    save_probe.mkdir(exist_ok=True)
    subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra', '-I', str(ROOT / 'src'),
                    str(ROOT / 'tests/save_compat.c'), str(ROOT / 'src/save_compat.c'),
                    '-o', str(save_probe / 'save_compat.exe')], check=True)
    subprocess.run([str(save_probe / 'save_compat.exe')], check=True, timeout=10)
    for fixed in (False, True):
        executable = BUILD / ('color-compat-fixed.exe' if fixed else 'color-compat-original.exe')
        flags = ['-DSQZ_FIX_16BPP_RGB_INDEX_REMAP', '-DSQZ_FIX_16BPP_DESKTOP_GATE', '-DSQZ_FIX_16BPP_COLORKEY_MASK'] if fixed else []
        subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra', *flags,
                        '-I', str(ROOT / 'src'), str(ROOT / 'tests/color_compat.c'),
                        str(ROOT / 'src/color_compat.c'), '-o', str(executable)], check=True)
        subprocess.run([str(executable), str(original_exe)], check=True, timeout=10)
    subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra', '-I', str(ROOT / 'src'),
                    str(ROOT / 'tests/geometry.c'), '-o', str(BUILD / 'geometry.exe')], check=True)
    subprocess.run([str(BUILD / 'geometry.exe')], check=True, timeout=10)
    subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra', '-I', str(ROOT / 'src'),
                    '-I', str(BUILD / 'sdl-headers/include'), str(ROOT / 'tests/surface_recovery.c'),
                    str(ROOT / 'src/platform.c'), str(ROOT / 'src/save_compat.c'), '-luser32', '-lgdi32', '-o', str(BUILD / 'surface_recovery.exe')], check=True)
    subprocess.run([str(BUILD / 'surface_recovery.exe')], check=True, timeout=10)
    subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra', '-I', str(ROOT / 'src'),
                    '-I', str(BUILD / 'sdl-headers/include'), str(ROOT / 'tests/canvas_dpi.c'),
                    str(ROOT / 'src/save_compat.c'), '-luser32', '-lgdi32', '-o', str(BUILD / 'canvas_dpi.exe')], check=True)
    subprocess.run([str(BUILD / 'canvas_dpi.exe')], check=True, timeout=10)
    PROBE = BUILD / 'probe'
    PROBE.mkdir(exist_ok=True)
    (PROBE / 'legacy').mkdir(exist_ok=True)
    for name in ('ddraw.dll', 'd3drm.dll', 'sqz-runtime.dll', 'SDL3.dll'):
        shutil.copy2(BUILD / name, PROBE / name)
    native_rm = game_files / 'd3drm.dll'
    shutil.copy2(native_rm, PROBE / 'legacy/d3drm.dll')
    subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra', '-I', str(ROOT / 'src'),
                    str(ROOT / 'tests/forward_probe.c'), '-o', str(PROBE / 'forward_probe.exe'),
                    '-lddraw', '-ld3drm', '-ldxguid', '-lole32'], check=True)
    env = dict(os.environ, SQZ_DDRAW_BACKEND='system', SQZ_SDL_WINDOW='0', SQZ_RENDERER='native')
    control = BUILD / 'probe-control'
    control.mkdir(exist_ok=True)
    for source, target in ((PROBE / 'forward_probe.exe', 'forward_probe.exe'),
                           (native_rm, 'd3drm.dll'),
                           (BUILD / 'SDL3.dll', 'SDL3.dll')):
        shutil.copy2(source, control / target)
    baseline = subprocess.run([str(control / 'forward_probe.exe')], cwd=control, env=env,
                              capture_output=True, text=True, timeout=30)
    print('CONTROL:', baseline.stdout, baseline.stderr, 'exit=', baseline.returncode)
    assert baseline.returncode == 0
    subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra',
                    '-I', str(ROOT / 'src'), '-I', str(BUILD / 'sdl-headers/include'),
                    str(ROOT / 'tests/scene_gpu.c'), str(ROOT / 'src/scene_cache.c'), str(ROOT / 'src/scene_extract.c'), str(ROOT / 'src/gpu_render.c'),
                    '-lddraw', '-ld3drm', '-ldxguid', '-lgdi32', '-o', str(control / 'scene_gpu.exe')], check=True)
    subprocess.run([str(control / 'scene_gpu.exe')], cwd=control, check=True, timeout=30)
    subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra',
                    '-I', str(ROOT / 'src'), '-I', str(BUILD / 'sdl-headers/include'),
                    str(ROOT / 'tests/native_shading.c'), str(ROOT / 'src/scene_cache.c'), str(ROOT / 'src/scene_extract.c'), str(ROOT / 'src/gpu_render.c'),
                    '-lddraw', '-ld3drm', '-ldxguid', '-lgdi32', '-o', str(control / 'native_shading.exe')], check=True)
    subprocess.run([str(control / 'native_shading.exe'), str(nut_model), str(spit_model)], cwd=control, check=True, timeout=30)
    subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra', '-DSQZ_CACHED_ORACLE',
                    '-I', str(ROOT / 'src'), '-I', str(BUILD / 'sdl-headers/include'),
                    str(ROOT / 'tests/native_shading.c'), str(ROOT / 'src/scene_cache.c'),
                    str(ROOT / 'src/scene_extract.c'), str(ROOT / 'src/gpu_render.c'),
                    '-lddraw', '-ld3drm', '-ldxguid', '-lgdi32', '-o', str(control / 'cached_shading.exe')], check=True)
    subprocess.run([str(control / 'cached_shading.exe'), str(nut_model), str(spit_model)], cwd=control, check=True, timeout=30)
    subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra', '-DSQZ_FIX_16BPP_WARP_BRIDGE',
                    '-I', str(ROOT / 'src'), str(ROOT / 'tests/warp_compat.c'),
                    str(ROOT / 'src/warp_compat.c'), '-lddraw', '-lgdi32', '-o', str(control / 'warp_compat.exe')], check=True)
    subprocess.run([str(control / 'warp_compat.exe'), str(original_exe)],
                   cwd=control, check=True, timeout=10)
    probe = subprocess.run([str(PROBE / 'forward_probe.exe')], cwd=PROBE, env=env,
                           capture_output=True, text=True, timeout=30)
    print(probe.stdout, probe.stderr, 'exit=', probe.returncode)
    assert probe.returncode == 0
    from generate import exports
    system_dd = system_ddraw()
    assert exports(PROBE / 'ddraw.dll') == exports(system_dd)
    assert exports(PROBE / 'd3drm.dll') == exports(native_rm)
    subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra', '-I', str(ROOT / 'src'),
                    str(ROOT / 'tests/window_probe.c'), '-o', str(PROBE / 'window_probe.exe'),
                    '-lddraw', '-ldxguid', '-lgdi32'], check=True)
    dropin = BUILD / 'probe-dropin'
    dropin.mkdir(exist_ok=True)
    for name in ('ddraw.dll', 'sqz-runtime.dll', 'SDL3.dll', 'd3drm.dll'):
        shutil.copy2(BUILD / 'patch' / name, dropin / name)
    for name in ('forward_probe.exe', 'window_probe.exe'):
        shutil.copy2(PROBE / name, dropin / name)
    dropin_env = dict(os.environ, SQZ_DROPIN_PROBE='1', SQZ_RENDERER='native')
    dropin_env.pop('SQZ_SDL_WINDOW', None)
    dropin_env.pop('__COMPAT_LAYER', None)
    assert not (dropin / 'legacy').exists()
    dropin_probe = subprocess.run([str(dropin / 'forward_probe.exe')], cwd=dropin,
                                  env=dropin_env, capture_output=True, text=True, timeout=30)
    print('DROP-IN:', dropin_probe.stdout, dropin_probe.stderr, 'exit=', dropin_probe.returncode)
    assert dropin_probe.returncode == 0
    quiet_env = dict(dropin_env)
    quiet_env.pop('SQZ_TRACE', None)
    quiet = subprocess.Popen([str(dropin / 'forward_probe.exe')], cwd=dropin,
                             env=quiet_env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    quiet_stdout, quiet_stderr = quiet.communicate(timeout=30)
    assert quiet.returncode == 0, quiet_stdout + quiet_stderr
    assert not (dropin / f'sqz-forward-{quiet.pid}.log').exists()
    assert not (dropin / f'sqz-diagnostic-{quiet.pid}.log').exists()
    assert (dropin / 'd3drm.dll').read_bytes() == native_rm.read_bytes()
    window = subprocess.run([str(dropin / 'window_probe.exe')], cwd=dropin,
                            env=dropin_env, capture_output=True, text=True, timeout=30)
    print(window.stdout, window.stderr, 'window_exit=', window.returncode)
    assert window.returncode == 0
    result = {'probe_exit': probe.returncode, 'control_exit': baseline.returncode,
              'window_probe_exit': window.returncode, 'export_names_and_ordinals_match': True,
              'dropin_original_rm_unchanged': True, 'sdl_enabled_without_environment': True}
    (BUILD / 'test-result.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
