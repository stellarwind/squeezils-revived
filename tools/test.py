import json
import os
from pathlib import Path
import shutil
import subprocess
from build import ROOT, BUILD, STAGE

gcc = shutil.which('gcc')
subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra', '-I', str(ROOT / 'src'),
                str(ROOT / 'tests/geometry.c'), '-o', str(BUILD / 'geometry.exe')], check=True)
subprocess.run([str(BUILD / 'geometry.exe')], check=True, timeout=10)
subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra', '-I', str(ROOT / 'src'),
                '-I', str(BUILD / 'sdl-headers/include'), str(ROOT / 'tests/surface_recovery.c'),
                str(ROOT / 'src/platform.c'), '-luser32', '-lgdi32', '-o', str(BUILD / 'surface_recovery.exe')], check=True)
subprocess.run([str(BUILD / 'surface_recovery.exe')], check=True, timeout=10)
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
env = dict(os.environ, SQZ_DDRAW_BACKEND='system', SQZ_SDL_WINDOW='0')
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
dropin_env = dict(os.environ, SQZ_DROPIN_PROBE='1')
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
result = {'probe_exit': probe.returncode, 'log': str(logs[-1]), 'lines': len(text.splitlines()),
          'control_exit': baseline.returncode, 'window_probe_exit': window.returncode,
          'required_methods': required, 'coverage_gaps': 0, 'export_names_and_ordinals_match': True,
          'nested_gdi_canvas': True, 'resize_repaints_without_ui_action': True,
          'dropin_original_rm_unchanged': True, 'sdl_enabled_without_environment': True}
(BUILD / 'test-result.json').write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps(result, indent=2))
