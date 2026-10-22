"""Validate byte-patch boundaries and the external read-only counter reader."""
import ctypes
import os
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from prepare_native_benchmark import ROOT, BEFORE, AFTER, patch_depth
from measure_frame_time import windows_api, snapshot


class BenchmarkTools(unittest.TestCase):
    def test_only_depth_fallback_changes(self):
        original = (ROOT.parent / 'SqueezilsGameFiles/SQZ.EXE').read_bytes()
        patched, offset = patch_depth(original)
        self.assertEqual(offset, 0x751b7)
        self.assertEqual(original[offset:offset + 7], BEFORE)
        self.assertEqual(patched[offset:offset + 7], AFTER)
        self.assertEqual(len(original), len(patched))
        self.assertEqual(original[:offset], patched[:offset])
        self.assertEqual(original[offset + 7:], patched[offset + 7:])
        with self.assertRaises(ValueError):
            patch_depth(patched)
        with self.assertRaises(ValueError):
            patch_depth(original[:-1])

    def test_read_only_snapshot(self):
        values = (ctypes.c_uint32 * 4)(123, 456, 789, 42)
        api = windows_api()
        handle = api.OpenProcess(0x0010 | 0x1000, False, os.getpid())
        self.assertTrue(handle)
        try:
            self.assertEqual(snapshot(api, handle, ctypes.addressof(values)), (123, 456, 789, 42))
        finally:
            api.CloseHandle(handle)


if __name__ == '__main__':
    unittest.main()
