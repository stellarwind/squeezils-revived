import unittest
from application_aliases import bindings, OWNERS
from surface_aliases import declaration


class ApplicationInterfaces(unittest.TestCase):
    def test_set_and_clear_have_distinct_destinations_and_symbols(self):
        specs = bindings()
        set_flag = specs['?SetFlag@Application@@QAEXXZ']
        clear_flag = specs['?ClearFlag@Application@@QAEXXZ']
        self.assertEqual(set_flag['address'], 0x43aa10)
        self.assertEqual(clear_flag['address'], 0x43aa20)
        text = 'struct Application { void SetFlag(); // 0x43aa20\n};'
        self.assertFalse(declaration(text, set_flag))
        self.assertFalse(declaration(text, clear_flag))

    def test_float_returns_remain_in_x87(self):
        for name in ('ForwardAxis', 'TurnAxis', 'ForwardTap', 'TurnTap'):
            spec = bindings()['?' + name + '@Application@@QAEMXZ']
            self.assertIn('x87 ST0', spec['abi'])
            self.assertEqual(spec['returned'], 'float')

    def test_value_argument_requires_int_declaration(self):
        spec = bindings()['?SetInputMode@Application@@QAEXH@Z']
        text = 'struct Application { void SetInputMode(int m); // 0x43b980\n};'
        self.assertTrue(declaration(text, spec))
        self.assertFalse(declaration(text.replace('int m', 'float m'), spec))

    def test_window_callback_pointer_and_shared_flush_owner(self):
        specs = bindings()
        init = specs['?Initialize@Application@@QAEHPAUWindowCallbacks@@@Z']
        self.assertTrue(declaration('struct Application { int Initialize(WindowCallbacks *c); // 0x43a740\n};', init))
        self.assertEqual(specs['?ResetClock@Application@@QAEXXZ']['address'],
                         specs['?FlushInput@Application@@QAEXXZ']['address'])
        self.assertEqual(len(specs), 11)
        self.assertEqual(len(OWNERS), 10)


if __name__ == '__main__':
    unittest.main()
