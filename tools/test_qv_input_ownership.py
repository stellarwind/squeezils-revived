"""Input ABI boundary and shared registry provenance checks."""
import unittest
from qv_input_ownership import address_for, canonical_map, state_proof, STATE_TARGET


class QvInputOwnership(unittest.TestCase):
    def test_input_reference_and_pointer_widths(self):
        self.assertEqual(address_for('?ReadString@QvIn7@@QAEHPAUQvStrLocal@@@Z'), 0x4811d0)
        self.assertEqual(address_for('?ReadInt@QvInput@@QAEHPAH@Z'), 0x481820)
        self.assertEqual(address_for('?GetUInt@QvInput@@QAEHPAI@Z'), 0x481510)
        for name in ('?ReadInt@QvInput@@QAEHPA_J@Z', '?ReadChar@QvIn7@@YGHPAD@Z',
                     '?ReadString@Other@@QAEHPAUQvStrLocal@@@Z'):
            self.assertIsNone(address_for(name))

    def test_static_factory_and_diagnostic_abi(self):
        self.assertEqual(address_for('?QvFactory@@YAPAUQvNodeF@@PAPAUQvEntryF@@@Z'), 0x47f1d0)
        self.assertEqual(address_for('?QvError@@YAXPAVQvInputC@@PBDZZ'), 0x482bd0)
        self.assertEqual(address_for('?QvRemoveName@@YAXPAXPAD@Z'), 0x47ea90)
        self.assertIsNone(address_for('?QvError@@YGXPAVQvInputC@@PBDZZ'))

    def test_dictionary_outputs_are_addresses_and_keys_are_32_bits(self):
        self.assertEqual(address_for('?Find@QvDictN@@QAEHIPAPAX@Z'), 0x4822e0)
        self.assertIsNone(address_for('?Find@QvDictN@@QAEHIPAX@Z'))
        self.assertIsNone(address_for('?Enter@QvDictN@@QAEH_KPAX@Z'))

    def proof(self, changed_original=False, canonical_present=True):
        original = {0x47e5eb: bytes.fromhex('833d38424c0000'),
                    0x47e614: bytes.fromhex('68fb000000'),
                    0x47e627: bytes.fromhex('a338424c00')}
        if changed_original:
            original[0x47e614] = bytes.fromhex('68fc000000')
        symbols = {0: dict(name='?init@QvNode@@SAXXZ', storage=2, section=1),
                   1: dict(name=STATE_TARGET, storage=2, section=2 if canonical_present else 0)}
        return state_proof(lambda a, n: original[a][:n], symbols,
                           {1: dict(relocations=[(0, 1, 6)])})

    def test_original_slot_and_251_bucket_initializer(self):
        self.proof()
        with self.assertRaisesRegex(ValueError, 'initialization changed'):
            self.proof(changed_original=True)

    def test_canonical_state_owner_required(self):
        with self.assertRaisesRegex(ValueError, 'does not own'):
            self.proof(canonical_present=False)
        self.assertEqual(canonical_map([])[0x4c4238], STATE_TARGET)


if __name__ == '__main__':
    unittest.main()
