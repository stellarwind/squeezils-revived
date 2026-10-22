import json
import os
from pathlib import Path
import shutil
import subprocess
from build import ROOT, BUILD, STAGE

gcc = shutil.which('gcc')
subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra', '-I', str(ROOT / 'src'),
                '-I', str(BUILD / 'sdl-headers/include'), str(ROOT / 'tests/frame_pixels.c'),
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
    subprocess.run([str(executable), str(ROOT.parent / 'SqueezilsGameFiles/SQZ.EXE')], check=True, timeout=10)
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
native_rm = ROOT.parent / 'SqueezilsGameFiles/d3drm.dll'
shutil.copy2(native_rm, PROBE / 'legacy/d3drm.dll')
subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra', '-I', str(ROOT / 'src'),
                str(ROOT / 'tests/forward_probe.c'), '-o', str(PROBE / 'forward_probe.exe'),
                '-lddraw', '-ld3drm', '-ldxguid', '-lole32'], check=True)
env = dict(os.environ, SQZ_DDRAW_BACKEND='system', SQZ_SDL_WINDOW='0', SQZ_TRACE='1', SQZ_RENDERER='native')
control = BUILD / 'probe-control'
control.mkdir(exist_ok=True)
for source, target in ((PROBE / 'forward_probe.exe', 'forward_probe.exe'),
                       (native_rm, 'd3drm.dll'),
                       (STAGE / 'SDL3.dll', 'SDL3.dll')):
    shutil.copy2(source, control / target)
baseline = subprocess.run([str(control / 'forward_probe.exe')], cwd=control, env=env,
                          capture_output=True, text=True, timeout=30)
print('CONTROL:', baseline.stdout, baseline.stderr, 'exit=', baseline.returncode)
assert baseline.returncode == 0
subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra',
                '-I', str(ROOT / 'src'), '-I', str(BUILD / 'sdl-headers/include'),
                str(ROOT / 'tests/scene_gpu.c'), str(ROOT / 'src/scene_extract.c'), str(ROOT / 'src/gpu_render.c'),
                '-lddraw', '-ld3drm', '-ldxguid', '-lgdi32', '-o', str(control / 'scene_gpu.exe')], check=True)
subprocess.run([str(control / 'scene_gpu.exe')], cwd=control, check=True, timeout=30)
subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra',
                '-I', str(ROOT / 'src'), '-I', str(BUILD / 'sdl-headers/include'),
                str(ROOT / 'tests/native_shading.c'), str(ROOT / 'src/scene_extract.c'), str(ROOT / 'src/gpu_render.c'),
                '-lddraw', '-ld3drm', '-ldxguid', '-lgdi32', '-o', str(control / 'native_shading.exe')], check=True)
subprocess.run([str(control / 'native_shading.exe')], cwd=control, check=True, timeout=30)
subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra', '-DSQZ_FIX_16BPP_WARP_BRIDGE',
                '-I', str(ROOT / 'src'), str(ROOT / 'tests/warp_compat.c'),
                str(ROOT / 'src/warp_compat.c'), '-lddraw', '-lgdi32', '-o', str(control / 'warp_compat.exe')], check=True)
subprocess.run([str(control / 'warp_compat.exe'), str(ROOT.parent / 'SqueezilsGameFiles/SQZ.EXE')],
               cwd=control, check=True, timeout=10)
probe = subprocess.run([str(PROBE / 'forward_probe.exe')], cwd=PROBE, env=env,
                       capture_output=True, text=True, timeout=30)
print(probe.stdout, probe.stderr, 'exit=', probe.returncode)
assert probe.returncode == 0
logs = sorted(PROBE.glob('sqz-forward-*.log'), key=lambda p: p.stat().st_mtime)
text = logs[-1].read_text()
required = ['IDirectDraw.CreateSurface', 'IDirect3D.Release', 'IDirectDrawSurface.Lock',
            'IDirect3DRM.CreateFrame', 'IDirect3DRMFrameArray.GetElement',
            'IDirect3DRMMaterial.GetPower', 'IDirect3DRMMeshBuilder.CreateFace',
            'IDirect3DRMFace.AddVertexAndNormalIndexed', 'IDirect3DRMFrame.Move']
assert all('CALL ' + name in text for name in required)
assert 'GAP ' not in text and 'FATAL ' not in text
diagnostic = logs[-1].with_name(logs[-1].name.replace('sqz-forward-', 'sqz-diagnostic-')).read_text()
for marker in ('DIAG BUILD transparency-dpi-v1', 'DIAG SURFACE actual', 'DIAG KEY',
               'DIAG IMAGE GetImage', 'DIAG PALETTE', 'value=ffff00ff',
               'DIAG RESULT IDirect3DRMTexture.SetDecalTransparentColor'):
    assert marker in diagnostic, marker
assert ' CALL ' not in diagnostic
from generate import exports
system_dd = Path(os.environ['WINDIR']) / 'SysWOW64/ddraw.dll'
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
dropin_env = dict(os.environ, SQZ_DROPIN_PROBE='1', SQZ_TRACE='1', SQZ_RENDERER='native')
dropin_env.pop('SQZ_SDL_WINDOW', None)
dropin_env.pop('__COMPAT_LAYER', None)
assert not (dropin / 'legacy').exists()
dropin_probe = subprocess.run([str(dropin / 'forward_probe.exe')], cwd=dropin,
                              env=dropin_env, capture_output=True, text=True, timeout=30)
print('DROP-IN:', dropin_probe.stdout, dropin_probe.stderr, 'exit=', dropin_probe.returncode)
assert dropin_probe.returncode == 0
dropin_logs = sorted(dropin.glob('sqz-forward-*.log'), key=lambda p: p.stat().st_mtime)
dropin_trace = dropin_logs[-1].read_text()
assert 'CALL IDirect3DRM.CreateFrame' in dropin_trace
assert 'GAP ' not in dropin_trace and 'FATAL ' not in dropin_trace
assert 'legacy\\' not in dropin_trace
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
window_logs = sorted(dropin.glob('sqz-forward-*.log'), key=lambda p: p.stat().st_mtime)
window_trace = window_logs[-1].read_text()
assert 'GDI CANVAS acquire' in window_trace and 'depth=2' in window_trace
assert window_trace.count('SDL REPAINT hr=00000000') >= 3
assert window_trace.index('GDI CANVAS release', window_trace.index('depth=2')) < window_trace.index('SDL REPAINT')
assert 'SDL PRESENT backend=GDI' in window_trace
assert 'SDL PRESENT GDI unavailable' not in window_trace
for marker in ('DIAG GAME_CAPS', 'DIAG FONT', 'height=-14', 'DIAG DISPLAY'):
    assert marker in window_trace, marker
launcher_dir = BUILD / 'probe-diagnostic-launcher'
launcher_dir.mkdir(exist_ok=True)
for name in ('ddraw.dll', 'sqz-runtime.dll', 'SDL3.dll', 'd3drm.dll'):
    shutil.copy2(dropin / name, launcher_dir / name)
shutil.copy2(PROBE / 'forward_probe.exe', launcher_dir / 'SQZ.EXE')
shutil.copy2(ROOT / 'tools/diagnose.ps1', launcher_dir / 'diagnose.ps1')
launcher = subprocess.run(['powershell.exe', '-NoProfile', '-ExecutionPolicy', 'Bypass',
                           '-File', str(launcher_dir / 'diagnose.ps1')], cwd=launcher_dir,
                          env=dropin_env, input='\n', capture_output=True, text=True, timeout=30)
assert launcher.returncode == 0 and 'Send this file back:' in launcher.stdout, launcher.stdout + launcher.stderr
import zipfile
archives = sorted(launcher_dir.glob('squeezils-diagnostics-*.zip'), key=lambda p: p.stat().st_mtime)
with zipfile.ZipFile(archives[-1]) as archive:
    assert len(archive.namelist()) == 1
    assert b'DIAG DECAL' in archive.read(archive.namelist()[0])
result = {'probe_exit': probe.returncode, 'log': str(logs[-1]), 'lines': len(text.splitlines()),
          'control_exit': baseline.returncode, 'window_probe_exit': window.returncode,
          'required_methods': required, 'coverage_gaps': 0, 'export_names_and_ordinals_match': True,
          'nested_gdi_canvas': True, 'resize_repaints_without_ui_action': True,
          'dropin_original_rm_unchanged': True, 'sdl_enabled_without_environment': True}
(BUILD / 'test-result.json').write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps(result, indent=2))
