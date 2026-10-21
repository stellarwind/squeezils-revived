"""Node identity must follow complete ctor/dtor entries and preserve ABI."""
import unittest
from qv_node_ownership import tables, binding, class_identity


class NodeOwnership(unittest.TestCase):
    def setUp(self):
        self.maps = tables([
            dict(address='00484530', symbol='??0QvSpotLight@@QAE@XZ', score='116'),
            dict(address='00484890', symbol='??1QvSpotLight@@UAE@XZ', score='0'),
            dict(address='0047e780', symbol='?getName@QvNode@@QBEABVQvName@@XZ', score='0')])

    def test_nonexact_constructor_is_a_functional_owner(self):
        d = binding('??0QvNodeC484530@@QAE@XZ', self.maps)
        self.assertEqual(d['target'], '??0QvSpotLight@@QAE@XZ')
        self.assertEqual(class_identity('QvC484530', self.maps), 'QvSpotLight')

    def test_ctor_identity_selects_its_actual_destructor(self):
        d = binding('??1QvNodeC484530@@UAE@XZ', self.maps)
        self.assertEqual(d['address'], '00484890')
        self.assertEqual(class_identity('QvNode484890', self.maps), 'QvSpotLight')

    def test_wrong_entry_kind_and_interior_address_rejected(self):
        for name in ('QvNodeC484531', 'QvNodeC484890', 'QvNode484530'):
            with self.assertRaisesRegex(ValueError, 'entry'):
                class_identity(name, self.maps)

    def test_nonvirtual_or_wrong_argument_destructors_rejected(self):
        with self.assertRaisesRegex(ValueError, 'ABI'):
            binding('??1QvNodeC484530@@QAE@XZ', self.maps)
        self.assertIsNone(binding('??_GQvNodeC484530@@UAEPAXI@Z', self.maps))
        self.assertIsNone(binding('??0QvNodeC484530@@QAE@H@Z', self.maps))

    def test_name_getter_is_a_handle_address(self):
        d = binding('?GetName@QvNode@@QAEPAXXZ', self.maps)
        self.assertEqual(d['target'], '?getName@QvNode@@QBEABVQvName@@XZ')
        with self.assertRaisesRegex(ValueError, 'Ambiguous'):
            tables([dict(address='00484530', symbol='a'), dict(address='00484530', symbol='b')])


if __name__ == '__main__':
    unittest.main()
