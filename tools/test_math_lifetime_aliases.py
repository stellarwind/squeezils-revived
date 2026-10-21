"""Exercise declaration identity and current-object proof failures."""
import contextlib
import json
import pathlib
import struct
import tempfile
import unittest
from unittest.mock import patch

import math_lifetime_aliases as lifetimes


class LifetimeProof(unittest.TestCase):
    def test_inline_multiline_and_header_annotations(self):
        self.assertEqual(lifetimes.annotations('struct V { float x,y,z; ~V(); }; // dtor 0x401660', 'V'), [0x401660])
        self.assertEqual(lifetimes.annotations('~M(); // empty, out of line (0x402310)', 'M'), [0x402310])
        self.assertEqual(lifetimes.annotations('struct V { // dtor 0x401660 and assignment 0x401670\n~V();\n};', 'V'), [0x401660])
        self.assertEqual(lifetimes.annotations('struct V { // dtor 0x401660\n};', 'V'), [])
        self.assertEqual(lifetimes.annotations('~V(); // 0x401660\n~V(); // 0x4383d0', 'V'), [0x401660, 0x4383d0])

    def test_nonvirtual_complete_zero_argument_destructors_only(self):
        self.assertIsNotNone(lifetimes.PATTERN.fullmatch('??1VectorAlias@@QAE@XZ'))
        for name in ('??1VectorAlias@@UAE@XZ', '??_GVectorAlias@@UAEPAXI@Z', '??1VectorAlias@@QAE@H@Z'):
            self.assertIsNone(lifetimes.PATTERN.fullmatch(name))

    def discover(self, declaration='~VectorAlias(); // 0x401660', original_call=0x401660, body=b'\xc3'):
        with tempfile.TemporaryDirectory() as folder, contextlib.ExitStack() as stack:
            repo = pathlib.Path(folder)
            src, out = repo / 'src', repo / 'out'
            src.mkdir()
            out.mkdir()
            (repo / 'tools').mkdir()
            (src / 'A.cpp').write_text(declaration)
            for name in ('ordinary_alias_registry.json', 'exact_call_registry.json'):
                (repo / 'tools' / name).write_text('{}')
            target = lifetimes.EMPTY_OWNERS[0x401660]
            owner = lifetimes.EMPTY_SOURCES[0x401660]
            alias = '??1VectorAlias@@QAE@XZ'
            (out / 'units.json').write_text(json.dumps([
                dict(source='A.cpp', object=str(out / 'A.obj'), returncode=0),
                dict(source=owner, object=str(out / 'owner.obj'), returncode=0)]))
            (out / 'ownership.json').write_text(json.dumps(dict(
                definitions={target: [owner]}, symbol_addresses={target: ['00401660']})))
            (out / 'exact-call-candidates.json').write_text(json.dumps(dict(
                call_sites={alias: [dict(address='%08x' % original_call)]})))
            raw = bytearray(60) + body
            struct.pack_into('<I', raw, 36, 0x20)
            (out / 'owner.obj').write_bytes(raw)
            symbol = dict(name=target, storage=2, section=1, value=0)
            for name, value in [('SRC', src), ('REPO', repo), ('OUT', out)]:
                stack.enter_context(patch.object(lifetimes.build, name, value))
            stack.enter_context(patch.object(lifetimes.build, 'coff_symbols', side_effect=lambda p:
                [(alias, 'undefined')] if p.name == 'A.obj' else [(target, 'comdat')]))
            stack.enter_context(patch.object(lifetimes, 'parse', return_value=(
                {0: symbol}, {1: dict(position=0, offset=60, size=len(body), relocations=[])})))
            stack.enter_context(patch.object(lifetimes, 'original_reader', return_value=lambda a, n: b'\xc3'[:n]))
            return lifetimes.discover()

    def test_current_declaration_and_original_entry_acceptance(self):
        self.assertIn('??1VectorAlias@@QAE@XZ', self.discover()['accepted'])

    def test_conflicting_or_missing_identity_withheld(self):
        for args in [dict(declaration='~VectorAlias();'), dict(original_call=0x4383d0),
                     dict(declaration='~VectorAlias(); // 0x401660\n~VectorAlias(); // 0x402310')]:
            self.assertFalse(self.discover(**args)['accepted'])

    def test_changed_canonical_body_fails(self):
        with self.assertRaisesRegex(ValueError, 'body changed'):
            self.discover(body=b'\x90')


if __name__ == '__main__':
    unittest.main()
