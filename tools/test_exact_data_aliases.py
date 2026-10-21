"""Reject incomplete identity evidence and malformed original fixups."""
import struct
import unittest

from exact_data_aliases import pe_fixups, resolve


class DataProof(unittest.TestCase):
    def test_single_owner(self):
        self.assertEqual(resolve([{'address': '004c0000'}], [], {'004c0000': ['owner']}),
                         ('004c0000', 'owner'))

    def test_unproven_occurrence(self):
        self.assertIsNone(resolve([{'address': '004c0000'}], ['unmatched.cpp'],
                                  {'004c0000': ['owner']}))

    def test_conflicting_reference(self):
        self.assertIsNone(resolve([{'address': '004c0000'}, {'address': '004c0004'}], [],
                                  {'004c0000': ['owner']}))

    def test_missing_or_ambiguous_owner(self):
        self.assertIsNone(resolve([{'address': '004c0000'}], [], {}))
        self.assertIsNone(resolve([{'address': '004c0000'}], [], {'004c0000': ['a', 'b']}))
        self.assertIsNone(resolve([], [], {}))

    def fixups(self, blob):
        raw = bytearray(256)
        struct.pack_into('<I', raw, 60, 64)
        struct.pack_into('<I', raw, 64 + 24 + 28, 0x400000)
        struct.pack_into('<II', raw, 64 + 24 + 96 + 40, 0x1000, len(blob))
        return pe_fixups(raw, lambda a, n: blob)

    def test_highlow_only(self):
        self.assertEqual(self.fixups(struct.pack('<IIHH', 0x2000, 12, 0x3010, 0)), {0x402010})

    def test_malformed_fixups(self):
        with self.assertRaises(ValueError):
            self.fixups(struct.pack('<II', 0x2000, 6))
        with self.assertRaises(ValueError):
            self.fixups(struct.pack('<IIHH', 0x2000, 12, 0x1010, 0))


if __name__ == '__main__':
    unittest.main()
