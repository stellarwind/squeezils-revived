"""Reviewed ownership must fail closed when declaration evidence changes."""
import json
import pathlib
import tempfile
import unittest
from unittest.mock import patch
import qv_name_ownership as names


class QvNames(unittest.TestCase):
    def test_entry_and_runtime_bindings(self):
        specs = names.specifications()
        self.assertEqual(specs['?QvIntern@@YAPAXPBD@Z']['target'], '?insert@QvNameEntry@@CAPBV1@PBD@Z')
        self.assertEqual(specs['_strdup']['target'], '__strdup')
        self.assertEqual(specs['?Reset@QvStrA@@QAEXH@Z']['address'], '00482810')

    def test_shared_state_is_library_storage(self):
        slots = [d for d in names.specifications().values() if d.get('data')]
        self.assertEqual({d['address'] for d in slots}, {'004c8868', '004c886c', '004c8870'})
        self.assertTrue(all('@QvNameEntry@@' in d['target'] for d in slots))

    def test_changed_or_missing_decision_rejected(self):
        with tempfile.TemporaryDirectory() as folder:
            repo = pathlib.Path(folder)
            (repo / 'tools').mkdir()
            saved = {'alias': {'target': 'owner', 'source_sha256': {'source': 'old'}}}
            (repo / 'tools/qv_name_registry.json').write_text(json.dumps(saved))
            with patch.object(names.build, 'REPO', repo):
                with patch.object(names, 'discover', return_value=saved):
                    self.assertEqual(names.reviewed(), saved)
                for current in ({}, {'alias': {'target': 'wrong'}}, {'alias': {'target': 'owner', 'source_sha256': {'source': 'changed'}}}):
                    with patch.object(names, 'discover', return_value=current):
                        with self.assertRaisesRegex(ValueError, 'evidence changed'):
                            names.reviewed()


if __name__ == '__main__':
    unittest.main()
