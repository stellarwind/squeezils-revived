import unittest
from destructor_aliases import choose, relocatable, object_digest


class DestructorAliases(unittest.TestCase):
    def test_only_compiler_timestamp_is_excluded_from_owner_digest(self):
        raw = bytes(range(32))
        self.assertEqual(object_digest(raw), object_digest(raw[:4] + bytes(4) + raw[8:]))
        self.assertNotEqual(object_digest(raw), object_digest(raw[:-1] + b'x'))

    def setUp(self):
        self.owners = [dict(symbol='??1Canonical@@QAE@XZ', source='Owner.cpp')]

    def test_one_address_and_complete_destructor(self):
        self.assertEqual(choose({0x405210}, [], self.owners),
                         ('??1Canonical@@QAE@XZ', 'Owner.cpp'))

    def test_conflicting_annotations_or_calls_are_rejected(self):
        self.assertIsNone(choose({0x405210, 0x405320}, [], self.owners))
        self.assertIsNone(choose({0x405210}, [dict(address='00405320')], self.owners))

    def test_ordinary_method_or_deleting_destructor_cannot_own_complete_lifetime(self):
        for symbol in ('?Dtor@Flat@@QAEXXZ', '??_GCanonical@@UAEPAXI@Z'):
            self.assertIsNone(choose({0x405210}, [], [dict(symbol=symbol, source='Owner.cpp')]))

    def test_ambiguous_owner_rejected(self):
        self.assertIsNone(choose({0x405210}, [], self.owners +
                                [dict(symbol='??1Other@@QAE@XZ', source='Other.cpp')]))

    def test_absolute_original_pointers_require_symbolic_relocations(self):
        section = dict(size=16, relocations=[(3, 0, 6)])
        self.assertTrue(relocatable(section, 0x405210, {0x405213, 0x405300}))
        self.assertFalse(relocatable(section, 0x405210, {0x405217}))


if __name__ == '__main__':
    unittest.main()
