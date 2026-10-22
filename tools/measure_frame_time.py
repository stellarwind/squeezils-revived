"""Read SQZ's existing cumulative timing counters externally; never inject or write.

Run after loading the test scene. Keep the game focused and stationary during
the sample. These are coarse timeGetTime CPU-stage averages and wall-clock frame
cadence, not GPU timestamps or frame-time percentiles.
"""
import argparse
import ctypes
from ctypes import wintypes
import hashlib
import json
from pathlib import Path
import struct
import time
from prepare_native_benchmark import ROOT, STOCK_SHA256, patch_depth

COUNTERS_ADDRESS = 0x4C7318


def windows_api():
    api = ctypes.WinDLL('kernel32', use_last_error=True)
    api.OpenProcess.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.DWORD]
    api.OpenProcess.restype = wintypes.HANDLE
    api.ReadProcessMemory.argtypes = [wintypes.HANDLE, ctypes.c_void_p, ctypes.c_void_p,
                                     ctypes.c_size_t, ctypes.POINTER(ctypes.c_size_t)]
    api.ReadProcessMemory.restype = wintypes.BOOL
    api.QueryFullProcessImageNameW.argtypes = [wintypes.HANDLE, wintypes.DWORD,
                                             wintypes.LPWSTR, ctypes.POINTER(wintypes.DWORD)]
    api.QueryFullProcessImageNameW.restype = wintypes.BOOL
    api.CloseHandle.argtypes = [wintypes.HANDLE]
    api.CloseHandle.restype = wintypes.BOOL
    return api


def snapshot(api, handle, address=COUNTERS_ADDRESS):
    data = ctypes.create_string_buffer(16)
    read = ctypes.c_size_t()
    if not api.ReadProcessMemory(handle, address, data, 16, ctypes.byref(read)) or read.value != 16:
        raise ctypes.WinError(ctypes.get_last_error())
    return struct.unpack('<4I', data.raw)


def measure(pid, seconds, delay, output):
    api = windows_api()
    # VM_READ and QUERY_LIMITED_INFORMATION only: no write/debug/injection rights.
    handle = api.OpenProcess(0x0010 | 0x1000, False, pid)
    if not handle:
        raise ctypes.WinError(ctypes.get_last_error())
    try:
        name = ctypes.create_unicode_buffer(32768)
        size = wintypes.DWORD(len(name))
        if not api.QueryFullProcessImageNameW(handle, 0, name, ctypes.byref(size)):
            raise ctypes.WinError(ctypes.get_last_error())
        executable = Path(name.value)
        digest = hashlib.sha256(executable.read_bytes()).hexdigest()
        original = (ROOT.parent / 'SqueezilsGameFiles/SQZ.EXE').read_bytes()
        patched, _ = patch_depth(original)
        if digest not in (STOCK_SHA256, hashlib.sha256(patched).hexdigest()):
            raise ValueError('Timing counter addresses are verified only for the stock/depth-patched EXE.')
        print(f'Sampling PID {pid} after {delay:g}s; focus the game and hold the same view.', flush=True)
        time.sleep(delay)
        begin = snapshot(api, handle)
        start = time.perf_counter()
        time.sleep(seconds)
        end = snapshot(api, handle)
        elapsed = time.perf_counter() - start
        if any(b < a for a, b in zip(begin, end)):
            raise ValueError('Timing counters reset during capture; remain in one level and retry.')
        prep, draw, present, frames = [b - a for a, b in zip(begin, end)]
        if not frames:
            raise ValueError('No rendered frames counted; enter gameplay before sampling.')
        report = dict(pid=pid, executable=str(executable), executable_sha256=digest,
                      seconds=elapsed, frames=frames, wall_ms_per_frame=elapsed * 1000 / frames,
                      wall_fps=frames / elapsed, prepare_ms_per_frame=prep / frames,
                      scene_and_hud_ms_per_frame=draw / frames,
                      present_ms_per_frame=present / frames,
                      measured_stages_ms_per_frame=(prep + draw + present) / frames,
                      note='Existing timeGetTime stage counters; boundary error up to a frame. '
                           'Wall cadence includes simulation/waiting. No per-frame percentiles.')
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps(report, indent=2))
    finally:
        api.CloseHandle(handle)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--pid', type=int, required=True)
    parser.add_argument('--seconds', type=float, default=10)
    parser.add_argument('--delay', type=float, default=5)
    parser.add_argument('--output', type=Path, default=ROOT / 'build/frame-time.json')
    args = parser.parse_args()
    if not 1 <= args.seconds <= 60 or not 0 <= args.delay <= 60:
        parser.error('Use 1..60 sample seconds and 0..60 delay seconds.')
    measure(args.pid, args.seconds, args.delay, args.output)
