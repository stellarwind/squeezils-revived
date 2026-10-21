"""Multi-variable annotations must preserve declaration/address ordering."""
import unittest
from source_annotations import annotated


class SourceAddresses(unittest.TestCase):
    def test_multiple_scalar_addresses(self):
        self.assertEqual(annotated('extern float a, b, c; // 0x4c1000 0x4c1004 0x4c1008'),
                         [('a', '004c1000'), ('b', '004c1004'), ('c', '004c1008')])

    def test_multiple_strings_and_pointers(self):
        self.assertEqual(annotated('extern const char a[], b[]; // 0x4bbfe4, 0x4bbfec'),
                         [('a', '004bbfe4'), ('b', '004bbfec')])
        self.assertEqual(annotated('extern Renderer *a, *b; // 0x4c1000 0x4c1004'),
                         [('a', '004c1000'), ('b', '004c1004')])

    def test_ambiguous_counts_are_rejected(self):
        for text in ('extern int a, b; // 0x4c1000',
                     'extern int a; // 0x4c1000 or 0x4c1004',
                     'extern Clip *a, *b; // 0x4c1000 04'):
            self.assertEqual(annotated(text), [])

    def test_explicit_scalar_range(self):
        self.assertEqual(annotated('extern float a, b, c; // 0x4c63c0..c8'),
                         [('a', '004c63c0'), ('b', '004c63c4'), ('c', '004c63c8')])

    def test_range_requires_known_scalar_size_and_extent(self):
        for text in ('extern float a, b; // 0x4c63c0..c8',
                     'extern float a, b; // 0x4c63c8..c0',
                     'extern double a, b; // 0x4c63c0..c8',
                     'extern int *a, *b; // 0x4c63c0..c4',
                     'extern char a[4], b[4]; // 0x4c63c0..c4'):
            self.assertEqual(annotated(text), [])


if __name__ == '__main__':
    unittest.main()
