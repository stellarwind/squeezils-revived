import unittest
from surface_aliases import bindings, declaration


class SurfaceInterfaces(unittest.TestCase):
    def setUp(self):
        self.spec = bindings()['?ToWorld@MoveSurface@@QAE?AUMoveMatrix@@ABU2@@Z']

    def test_matrix_overload_requires_return_type_and_original_address(self):
        text = 'struct MoveSurface { MoveMatrix ToWorld(const MoveMatrix &m); // 0x4105f0\n};'
        self.assertTrue(declaration(text, self.spec))
        self.assertFalse(declaration(text.replace('0x4105f0', '0x40f850'), self.spec))
        self.assertFalse(declaration(text.replace('MoveMatrix', 'MoveVector'), self.spec))

    def test_same_name_in_another_class_cannot_supply_evidence(self):
        text = 'struct Other { MoveMatrix ToWorld(const MoveMatrix &m); // 0x4105f0\n};'
        self.assertFalse(declaration(text, self.spec))

    def test_class_end_and_nested_inline_body_are_respected(self):
        text = 'struct MoveSurface { void f() { if (1) {} } };\n'
        text += 'MoveMatrix ToWorld(const MoveMatrix &m); // 0x4105f0'
        self.assertFalse(declaration(text, self.spec))

    def test_contradictory_duplicate_declarations_are_rejected(self):
        text = 'struct MoveSurface { MoveMatrix ToWorld(const MoveMatrix &m); // 0x4105f0\n'
        text += 'MoveMatrix ToWorld(const MoveMatrix &m); // 0x40f850\n};'
        self.assertFalse(declaration(text, self.spec))

    def test_family_uses_full_symbols_and_excludes_nonexact_near_body(self):
        entries = bindings()
        self.assertEqual(len(entries), 36)
        self.assertFalse(any(n.startswith('?IsNear@') for n in entries))
        self.assertEqual(self.spec['return_bytes'], 64)
        self.assertEqual(entries['?ToWorld@MoveSurface@@QAE?AUMoveVector@@ABU2@@Z']['return_bytes'], 12)

    def test_contact_pointer_constness_and_string_buffer_are_distinct(self):
        entries = bindings()
        slippery = entries['?IsSlippery@NavSurfaces@@QAEHPBUContact@@@Z']
        text = 'struct NavSurfaces { int IsSlippery(const Contact *c); // 0x40f770\n};'
        self.assertTrue(declaration(text, slippery))
        self.assertFalse(declaration(text.replace('const ', ''), slippery))
        script = entries['?ActivateScript@NavSurfaces@@QAE?AVString@@PAUContact@@@Z']
        self.assertEqual(script['return_bytes'], 4)
        self.assertEqual(script['argument_mode'], 'pointer')


if __name__ == '__main__':
    unittest.main()
