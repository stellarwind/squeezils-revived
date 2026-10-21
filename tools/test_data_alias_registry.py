"""Reviewed singleton pointers must retain their original storage identity."""
import unittest
from data_aliases import validate_pointer_decision


class PointerRegistry(unittest.TestCase):
    def check(self, alias='?g_renderer@@3PAVRenderer@@A', address='4C75DC',
              found=None, targets=None, sites=None, declaration=None):
        decision = dict(address=address, storage='one mutable 32-bit object pointer',
                        evidence=['Renderer.cpp'])
        validate_pointer_decision(alias, decision,
            {'g_renderer': {'4C75DC'}} if found is None else found,
            {'4C75DC': ['canonical']} if targets is None else targets,
            {} if sites is None else sites,
            lambda _: 'extern Renderer *g_renderer; // 0x4c75dc' if declaration is None else declaration)

    def test_typed_pointer_storage(self):
        for alias in ('?g_renderer@@3PAVRenderer@@A', '?g_renderer@@3PAURenderer@@A',
                      '?g_renderer@@3PAXA'):
            self.check(alias=alias)

    def test_incompatible_storage(self):
        for alias in ('?g_renderer@@3HA', '?g_renderer@@3URenderer@@A',
                      '?g_renderer@@3PAPAVRenderer@@A'):
            with self.subTest(alias=alias), self.assertRaises(ValueError):
                self.check(alias=alias)

    def test_address_and_owner_changes(self):
        for kwargs in (dict(found={'g_renderer': {'4C75DC', '4C75E0'}}),
                       dict(targets={}), dict(targets={'4C75DC': ['a', 'b']}),
                       dict(declaration='extern Renderer *g_renderer; // 0x4c75e0')):
            with self.subTest(kwargs=kwargs), self.assertRaises(ValueError):
                self.check(**kwargs)

    def test_original_reference_conflict(self):
        with self.assertRaises(ValueError):
            self.check(sites={'?g_renderer@@3PAVRenderer@@A': [{'address': '004c75e0'}]})


if __name__ == '__main__':
    unittest.main()
