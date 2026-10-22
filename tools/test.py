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
shutil.copy2(STAGE / 'legacy/d3drm.dll', PROBE / 'legacy/d3drm.dll')
subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra', '-I', str(ROOT / 'src'),
                str(ROOT / 'tests/forward_probe.c'), '-o', str(PROBE / 'forward_probe.exe'),
                '-lddraw', '-ldxguid', '-lole32'], check=True)
env = dict(os.environ, SQZ_DDRAW_BACKEND='system')
control = BUILD / 'probe-control'
control.mkdir(exist_ok=True)
for source, target in ((PROBE / 'forward_probe.exe', 'forward_probe.exe'),
                       (STAGE / 'legacy/d3drm.dll', 'd3drm.dll'),
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
assert exports(PROBE / 'd3drm.dll') == exports(STAGE / 'legacy/d3drm.dll')
subprocess.run([gcc, '-m32', '-std=c11', '-O2', '-Wall', '-Wextra', '-I', str(ROOT / 'src'),
                str(ROOT / 'tests/window_probe.c'), '-o', str(PROBE / 'window_probe.exe'),
                '-lddraw', '-ldxguid', '-lgdi32'], check=True)
window = subprocess.run([str(PROBE / 'window_probe.exe')], cwd=PROBE,
                        env=dict(env, SQZ_SDL_WINDOW='1'), capture_output=True, text=True, timeout=30)
print(window.stdout, window.stderr, 'window_exit=', window.returncode)
assert window.returncode == 0
window_logs = sorted(PROBE.glob('sqz-forward-*.log'), key=lambda p: p.stat().st_mtime)
window_trace = window_logs[-1].read_text()
assert 'GDI CANVAS acquire' in window_trace and 'depth=2' in window_trace
assert window_trace.count('SDL REPAINT hr=00000000') >= 3
assert window_trace.index('GDI CANVAS release', window_trace.index('depth=2')) < window_trace.index('SDL REPAINT')
assert 'SDL CLIPPER refresh=00000000' in window_trace
result = {'probe_exit': probe.returncode, 'log': str(logs[-1]), 'lines': len(text.splitlines()),
          'control_exit': baseline.returncode, 'window_probe_exit': window.returncode,
          'required_methods': required, 'coverage_gaps': 0, 'export_names_and_ordinals_match': True,
          'nested_gdi_canvas': True, 'resize_repaints_without_ui_action': True}
(BUILD / 'test-result.json').write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps(result, indent=2))
