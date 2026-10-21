"""Declaration and ABI boundary checks for the reviewed getter family."""
import pathlib
import tempfile
import unittest
from unittest.mock import patch

import global_getter_aliases as getters


class GetterProof(unittest.TestCase):
    def test_declarations_require_matching_pointer_and_address(self):
        text = 'Sim *GetSimulation(); // 0x420690\n'
        self.assertEqual(getters.declarations(text, 'GetSimulation', 'Sim'), [0x420690])
        self.assertEqual(getters.declarations(text, 'GetSimulation', 'Other'), [])
        self.assertEqual(getters.declarations(text, 'GetLand', 'Sim'), [])
        self.assertEqual(getters.declarations('void *GetSimulation(void); // 0x420690',
                                             'GetSimulation', None), [0x420690])

    def test_arguments_conventions_and_unannotated_declarations_rejected(self):
        for text in ('Sim *GetSimulation(int); // 0x420690',
                     'Sim *__stdcall GetSimulation(); // 0x420690',
                     'Sim *GetSimulation();', 'Sim GetSimulation(); // 0x420690'):
            self.assertEqual(getters.declarations(text, 'GetSimulation', 'Sim'), [])

    def test_conflicting_addresses_remain_visible(self):
        text = 'Sim *GetSimulation(); // 0x420690\nSim *GetSimulation(); // 0x4206a0'
        self.assertEqual(getters.declarations(text, 'GetSimulation', 'Sim'), [0x420690, 0x4206a0])

    def test_only_zero_argument_cdecl_pointer_decorations(self):
        self.assertIsNotNone(getters.PATTERN.fullmatch('?GetSimulation@@YAPAUSim@@XZ'))
        self.assertIsNotNone(getters.PATTERN.fullmatch('?GetPanel@@YAPAVPanel@@XZ'))
        for symbol in ('?GetSimulation@@YGPAUSim@@XZ', '?GetSimulation@@YAHXZ',
                       '?GetSimulation@@YAPAUSim@@H@Z'):
            self.assertIsNone(getters.PATTERN.fullmatch(symbol))

    def test_local_headers_cycles_and_source_root_boundary(self):
        with tempfile.TemporaryDirectory() as folder:
            root = pathlib.Path(folder) / 'src'
            root.mkdir()
            (root / 'A.cpp').write_text('#include "A.h"\n#include "../outside.h"')
            (root / 'A.h').write_text('#include "A.h"\nSim *GetSimulation(); // 0x420690')
            (root.parent / 'outside.h').write_text('Sim *GetSimulation(); // 0x4206a0')
            with patch.object(getters.build, 'SRC', root):
                self.assertEqual({p.name for p in getters.source_files('A.cpp')}, {'A.cpp', 'A.h'})


if __name__ == '__main__':
    unittest.main()
