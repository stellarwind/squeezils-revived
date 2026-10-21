"""Verify real TypeDescriptor content, CRT fixups and annotation association."""
import pathlib
import struct
import tempfile
import unittest
from unittest.mock import patch

from rtti_aliases import annotated, descriptor, emit, existing_owner, extracted_descriptors, VTABLE
from project_eh import parse


class TypeDescriptors(unittest.TestCase):
    def extracted(self, addend=0, fixups=None, pointer=0x2000, typ=6):
        symbols = {0: dict(name='?g_2000@@3PAEA', section=0, storage=2),
                   1: dict(name='?g_1000@@3UT_g_1000@@A', section=1, storage=2, value=0)}
        sections = {1: dict(offset=0, size=4, relocations=[(0, 0, typ)])}
        header = struct.pack('<II', 0x4a177c, 0) + b'.?AVSoul@@\0' + bytes(256)
        def read(a, n):
            if a == 0x1000:
                return struct.pack('<I', pointer)
            return header[a-0x2000:a-0x2000+n]
        with patch('rtti_aliases.parse', return_value=(symbols, sections)):
            return extracted_descriptors(struct.pack('<I', addend), read,
                                         {0x1000} if fixups is None else fixups)

    def test_extracted_original_pointer(self):
        decision = self.extracted()['?g_2000@@3PAEA']
        self.assertEqual(decision['address'], '00002000')
        self.assertEqual(decision['original_reference_sites'], ['00001000'])
        self.assertEqual(decision['canonical_symbol'], '??_R0?AVSoul@@@8')

    def test_extracted_rejects_unproven_pointer(self):
        for kwargs in (dict(addend=4), dict(fixups=set()), dict(pointer=0x2004), dict(typ=20)):
            with self.subTest(kwargs=kwargs), self.assertRaises(ValueError):
                self.extracted(**kwargs)

    def test_multiple_names_and_addresses(self):
        self.assertEqual(annotated('extern const char A[], B[]; // 0x4ba1d0, 0x4bae38'),
                         [('A', '004ba1d0'), ('B', '004bae38')])
        self.assertEqual(annotated('extern const char A[], B[]; // 0x4ba1d0'), [])

    def test_original_header_and_name(self):
        good = struct.pack('<II', 0x4a177c, 0) + b'.?AVSoul@@\0' + bytes(256)
        self.assertEqual(descriptor(lambda a, n: good[a:a+n], 0), '.?AVSoul@@')
        for bad in (struct.pack('<II', 123, 0) + good[8:],
                    struct.pack('<II', 0x4a177c, 1) + good[8:],
                    good[:8] + b'ordinary string\0' + bytes(256)):
            with self.assertRaises(ValueError):
                descriptor(lambda a, n: bad[a:a+n], 0)

    def test_generated_metadata_and_symbolic_vtable(self):
        with tempfile.TemporaryDirectory() as folder:
            path = pathlib.Path(folder) / 'metadata.obj'
            emit([('_Soul', '.?AVSoul@@'), ('_Place', '.?AVPlaceSimulation@@')], path)
            raw = path.read_bytes()
            symbols, sections = parse(raw)
            section = sections[1]
            self.assertFalse(struct.unpack_from('<I', raw, section['position'] + 36)[0] & 0x20)
            self.assertEqual(len(section['relocations']), 2)
            self.assertEqual(symbols[0]['name'], VTABLE)
            self.assertEqual(symbols[0]['section'], 0)
            for symbol in symbols.values():
                if symbol['section'] > 0:
                    name = '.?AVSoul@@' if symbol['name'] == '_Soul' else '.?AVPlaceSimulation@@'
                    self.assertTrue(existing_owner(raw, symbol, symbols, sections, name))
                    changed = bytearray(raw)
                    changed[section['offset'] + symbol['value'] + 4] = 1
                    self.assertFalse(existing_owner(changed, symbol, symbols, sections, name))


if __name__ == '__main__':
    unittest.main()
