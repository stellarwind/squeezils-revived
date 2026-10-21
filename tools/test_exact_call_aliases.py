"""Fail-closed tests for complete-body alias evidence."""
import struct
import unittest

from exact_call_aliases import exact_body, exact_shared_leaf, POINTER_GETTERS, pointer_getter_abi


class BodyProof(unittest.TestCase):
    def shared(self, originals, relocs=()):
        raw = bytearray(60) + b'\x8b\x41\x04\xc3'
        struct.pack_into('<I', raw, 36, 0x20)
        symbol = dict(section=1, value=0, storage=2, name='getter')
        return exact_shared_leaf(raw, {0: symbol},
            {1: dict(position=0, offset=60, size=4, relocations=relocs)},
            symbol, list(originals), lambda a, n: originals['%08x' % a][:n])

    def test_shared_leaf_compares_each_original_destination(self):
        self.assertEqual(self.shared({'00401000': b'\x8b\x41\x04\xc3',
                                      '00402000': b'\x8b\x41\x04\xc3'}),
                         ['00401000', '00402000'])
        self.assertEqual(self.shared({'00401000': b'\x8b\x41\x04\xc3',
                                      '00402000': b'\x8b\x41\x08\xc3'}), ['00401000'])

    def test_shared_leaf_requires_no_relocations_or_suffix(self):
        originals = {'00401000': b'\x8b\x41\x04\xc3', '00402000': b'\x8b\x41\x04\xc3'}
        self.assertEqual(self.shared(originals, [(0, 0, 6)]), [])
        self.assertEqual(self.shared({'00401000': originals['00401000']}), [])

    def proof(self, body, original, relocs=(), value=0, flags=0x20, extra=False,
              name='', address=0x401000):
        raw = bytearray(60) + body
        struct.pack_into('<I', raw, 36, flags)
        symbols = {0: dict(storage=2, section=1, value=0)}
        if extra:
            symbols[1] = dict(storage=2, section=1, value=1)
        return exact_body(raw, symbols, {1: dict(position=0, offset=60,
                          size=len(body), relocations=relocs)},
                          dict(section=1, value=value, name=name), address,
                          lambda a, n: original[:n])

    def test_exact_body(self):
        self.assertTrue(self.proof(b'\x55\x5d\xc3', b'\x55\x5d\xc3'))

    def test_call_relocation_only(self):
        self.assertTrue(self.proof(b'\x55\xe8\0\0\0\0\x5d\xc3',
                                  b'\x55\xe8\x11\x22\x33\x44\x5d\xc3', [(2, 0, 20)]))

    def test_wrong_cleanup_is_rejected(self):
        self.assertFalse(self.proof(b'\xc2\x04\0', b'\xc2\x08\0'))

    def test_short_original_read_is_rejected(self):
        self.assertFalse(self.proof(b'\x55\x5d\xc3', b'\x55\x5d'))

    def test_reviewed_empty_destructors_only(self):
        for address, name in ((0x401660, '??1Vector3f@@QAE@XZ'),
                              (0x402310, '??1MatrixLifetime@@QAE@XZ')):
            self.assertTrue(self.proof(b'\xc3', b'\xc3', name=name, address=address))
            self.assertFalse(self.proof(b'\xc3', b'\xc3', name=name, address=address + 1))
            self.assertFalse(self.proof(b'\xc3', b'\xc3', name='unknown', address=address))
            self.assertFalse(self.proof(b'\xc3', b'\x90', name=name, address=address))

    def getter(self, address, name, global_name, original_address, typ=6):
        raw = bytearray(60) + b'\xa1' + bytes(4) + b'\xc3'
        struct.pack_into('<I', raw, 36, 0x20)
        symbols = {0: dict(storage=2, section=1, value=0),
                   1: dict(storage=2, section=0, value=0, name=global_name)}
        sections = {1: dict(position=0, offset=60, size=6, relocations=[(1, 1, typ)])}
        return exact_body(raw, symbols, sections, dict(section=1, value=0, name=name),
                          address, lambda a, n: b'\xa1' + struct.pack('<I', original_address) + b'\xc3')

    def test_pointer_getters_require_original_storage_operand(self):
        for address, (name, global_name, original_address) in POINTER_GETTERS.items():
            self.assertTrue(self.getter(address, name, global_name, original_address))
            self.assertFalse(self.getter(address, name, global_name, original_address + 4))
            self.assertFalse(self.getter(address, name, 'wrong-global', original_address))
            self.assertFalse(self.getter(address, 'wrong-function', global_name, original_address))
            self.assertFalse(self.getter(address, name, global_name, original_address, typ=20))

    def test_pointer_getter_calling_convention(self):
        self.assertTrue(pointer_getter_abi('?GetSim@@YAPAUSim@@XZ'))
        self.assertTrue(pointer_getter_abi('?GetSim@@YAPAXXZ'))
        self.assertFalse(pointer_getter_abi('?GetHolder@@YGPAUHolder@@H@Z'))
        self.assertFalse(pointer_getter_abi('?GetSim@@YAHXZ'))
        self.assertFalse(pointer_getter_abi('?GetSim@@YAPAUSim@@H@Z'))

    def test_interior_and_data_rejected(self):
        self.assertFalse(self.proof(b'\x55\x5d\xc3', b'\x55\x5d\xc3', value=1))
        self.assertFalse(self.proof(b'\x55\x5d\xc3', b'\x55\x5d\xc3', flags=0x40))
        self.assertFalse(self.proof(b'\x55\x5d\xc3', b'\x55\x5d\xc3', extra=True))

    def test_bad_relocation_and_weak_evidence_rejected(self):
        self.assertFalse(self.proof(b'\x55\x5d\xc3', b'\x55\x5d\xc3', [(1, 0, 20)]))
        self.assertFalse(self.proof(b'\x55\x5d\xc3', b'\x55\x5d\xc3', [(0, 0, 7)]))
        self.assertFalse(self.proof(b'\xc3', b'\xc3'))


if __name__ == '__main__':
    unittest.main()
