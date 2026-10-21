import pathlib
import subprocess
import tempfile
import unittest
from unittest import mock
import contextlib
import io
import json

from target_rows import parse_target_row
import integration


class TargetRows(unittest.TestCase):
    def test_matcher_and_inventory_agree(self):
        tools = pathlib.Path(__file__).resolve().parent
        src = tools.parent / 'src'
        lines = ['', '  # comment', '#FLAGS -GX -O2',
                 '^name 00401000 label', '^name 0X401000 label 12',
                 '^name 401000 label auto', '^name 401000 label ehend',
                 '^name 401000 label cc', '^name 401000', ' label',
                 '^name nope label', '^name 401000 label 0',
                 '^name 401000 label 0x10', '^name 401000 label 12 extra']
        for row in (src / 'manifest.txt').read_text().splitlines():
            if row.strip() and not row.lstrip().startswith('#'):
                lines.extend((src / row.split()[0]).read_text().splitlines())
        expected = []
        for line in lines:
            try:
                fields = parse_target_row(line)
                expected.append('\t'.join(fields) if fields else 'skip')
            except ValueError:
                expected.append('invalid')
        program = ('while (<STDIN>) { my $f = eval { parse_target_row($_) }; '
                   'print $@ ? "invalid" : defined($f) ? join("\\t", @$f) : "skip"; '
                   'print "\\n"; }')
        result = subprocess.run(['perl', '-I', str(tools), '-MTargetRows=parse_target_row',
                                 '-e', program], input='\n'.join(lines) + '\n',
                                text=True, capture_output=True, check=True)
        self.assertEqual(result.stdout.splitlines(), expected)

    def test_split_target_is_rejected_without_inventing_a_label(self):
        for line in ('^Grow 45de70', ' grow', '^Grow 45de70 grow 12 extra'):
            with self.assertRaises(ValueError):
                parse_target_row(line)

    def test_all_manifest_targets_are_well_formed(self):
        src = pathlib.Path(__file__).resolve().parents[1] / 'src'
        for row in (src / 'manifest.txt').read_text().splitlines():
            if row.strip() and not row.lstrip().startswith('#'):
                path = src / row.split()[0]
                for number, line in enumerate(path.read_text().splitlines(), 1):
                    with self.subTest(path=str(path), line=number):
                        parse_target_row(line)

    def test_inventory_stops_on_invalid_metadata_and_keeps_diagnostics(self):
        with tempfile.TemporaryDirectory() as folder:
            path = pathlib.Path(folder)
            (path / 'units.json').write_text('[]')
            (path / 'manifest.txt').write_text('targets.txt Owner.cpp\n')
            (path / 'targets.txt').write_text('^Grow 45de70\n grow\n')
            with mock.patch.object(integration, 'SRC', path), \
                    mock.patch.object(integration, 'OUT', path), \
                    contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(integration.inventory_ownership(), 1)
            report = json.loads((path / 'ownership.json').read_text())
            self.assertEqual([row[1] for row in report['invalid']], [1, 2])
            self.assertEqual(report['addresses'], {})


if __name__ == '__main__':
    unittest.main()
