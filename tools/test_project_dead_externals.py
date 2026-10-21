import struct
import unittest

from project_dead_externals import retire
from project_eh import parse


def fixture(retained=False, common=False, weak=False):
    # Two sections, each with a relocation to external row 0. Projection drops
    # only the first section; a weak alias optionally retains row 0 indirectly.
    count = 3 if weak else 1
    raw = bytearray(120)
    struct.pack_into('<HHIIIHH', raw, 0, 0x14c, 2, 0, 120, count, 0, 0)
    for i in range(2):
        p = 20 + 40 * i
        struct.pack_into('<8sIIIIIIHHI', raw, p, b'.text', 0, 0, 0, 0,
                         100 + 10 * i, 0, 1 if i == 0 or retained else 0, 0, 0)
        struct.pack_into('<IIH', raw, 100 + 10 * i, 0, 0, 6)
    raw.extend(struct.pack('<8sIhHBB', b'missing', 4 if common else 0, 0, 0, 2, 0))
    if weak:
        raw.extend(struct.pack('<8sIhHBB', b'weak', 0, 0, 0, 105, 1))
        raw.extend(struct.pack('<II10s', 0, 3, bytes(10)))
    raw.extend(struct.pack('<I', 4))
    projected = bytearray(raw)
    struct.pack_into('<H', projected, 20 + 32, 0)
    return raw, projected


class DeadExternals(unittest.TestCase):
    def test_only_discarded_reference_is_retired_without_moving_indices(self):
        original, projected = fixture()
        output, names = retire(original, projected, [1])
        self.assertEqual(names, ['missing'])
        self.assertEqual(len(output), len(projected))
        symbols, _ = parse(output)
        self.assertEqual((symbols[0]['section'], symbols[0]['storage']), (-1, 3))
        self.assertEqual(output[:120], projected[:120])

    def test_kept_relocation_preserves_external(self):
        original, projected = fixture(retained=True)
        self.assertEqual(retire(original, projected, [1]), (projected, []))

    def test_weak_auxiliary_reference_preserves_external(self):
        original, projected = fixture(weak=True)
        self.assertEqual(retire(original, projected, [1]), (projected, []))

    def test_common_storage_is_not_retired(self):
        original, projected = fixture(common=True)
        self.assertEqual(retire(original, projected, [1]), (projected, []))

    def test_unreferenced_symbol_without_discarded_reference_is_preserved(self):
        original, projected = fixture()
        self.assertEqual(retire(original, projected, []), (projected, []))


if __name__ == '__main__':
    unittest.main()
