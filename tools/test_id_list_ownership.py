import unittest
import struct
from id_list_ownership import require_slot, BINDINGS, TABLES
from link_containers import project
from project_eh import parse


class IdListOwnership(unittest.TestCase):
    def test_original_slot_identity_accepts_a_reviewed_shared_entry(self):
        require_slot('getter', 0x40ec80, {'getter': ['0040ec80', '0040ec90']})

    def test_primitive_create_cannot_replace_set_create(self):
        with self.assertRaises(ValueError):
            require_slot('PrimitiveArray::Create', 0x45f310,
                         {'PrimitiveArray::Create': ['0045ed90']})

    def test_missing_or_wrong_mapping_is_rejected(self):
        for mapped in ({}, {'slot': ['00401000']}):
            with self.assertRaises(ValueError):
                require_slot('slot', 0x402000, mapped)

    def test_two_complete_lifetimes_and_both_vptr_views_are_reviewed(self):
        self.assertEqual(len(BINDINGS), 2)
        self.assertEqual(sorted(count for _, count in TABLES.values()), [4, 9])
        self.assertEqual({a for a, _, _ in BINDINGS.values()}, {0x45eef0, 0x45efa0})

    def test_only_reviewed_data_may_have_nonzero_section_offset(self):
        raw = bytearray(68)
        struct.pack_into('<HHIIIHH', raw, 0, 0x14c, 1, 0, 68, 1, 0, 0)
        struct.pack_into('<8sIIIIIIHHI', raw, 20, b'.rdata', 0, 0, 8, 60, 0, 0, 0, 0, 0x1040)
        raw.extend(struct.pack('<8sIhHBB', b'table', 4, 1, 0, 2, 0))
        raw.extend(struct.pack('<I', 4))
        with self.assertRaises(ValueError):
            project(raw, {'table'})
        output, removed = project(raw, {'table'}, {'table'})
        self.assertEqual(removed, [1])
        symbols, _ = parse(output)
        self.assertEqual(symbols[0]['section'], 0)
        code = bytearray(raw)
        struct.pack_into('<I', code, 56, 0x1020)
        with self.assertRaises(ValueError):
            project(code, {'table'}, {'table'})


if __name__ == '__main__':
    unittest.main()
