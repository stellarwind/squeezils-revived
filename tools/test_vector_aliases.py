"""Conservative source-layout and operation-identity checks."""
import unittest

from vector_aliases import class_blocks, classify, member_addresses


class VectorProof(unittest.TestCase):
    def test_plain_nontrivial_vector_with_inline_methods(self):
        code = '''struct Vec {
    float x, y, z;
    Vec() {}
    Vec(float a, float b, float c) : x(a), y(b), z(c) {}
    Vec &operator=(const Vec &o) { x=o.x; y=o.y; z=o.z; return *this; }
    ~Vec(); // 0x401660
    Vec operator*(float value) const; // 0x401b60
};'''
        blocks = class_blocks(code, 'Vec')
        self.assertEqual(len(blocks), 1)
        self.assertEqual(member_addresses(blocks[0], 'Vec', r'operator\s*\*'), [0x401b60])

    def test_wrong_layout_and_trivial_return_rejected(self):
        for fields in ('float z, y, x;', 'float x,y,z,w;', 'double x,y,z;',
                       'float x,y,z; int flags;', 'float x,y,z; float (*callback)(float);',
                       'float x,y,z; float extra = initial();'):
            self.assertEqual(class_blocks('struct V { ' + fields + ' ~V(); };', 'V'), [])
        self.assertEqual(class_blocks('struct V { float x,y,z; };', 'V'), [])
        self.assertEqual(class_blocks('struct V : Base { float x,y,z; ~V(); };', 'V'), [])
        self.assertEqual(class_blocks('struct V { float x,y,z; virtual ~V(); };', 'V'), [])

    def test_compact_annotation_order_and_named_partial_list(self):
        block = 'struct V { float x,y,z; ~V(); V operator*(float s) const; float Length() const; }; // dtor 0x401660, 0x401b60, 0x47d5f0'
        kept = class_blocks(block, 'V')[0]
        self.assertEqual(member_addresses(kept, 'V', r'operator\s*\*'), [0x401b60])
        self.assertEqual(member_addresses(kept, 'V', r'\bLength'), [0x47d5f0])
        block = 'struct V { float x,y,z; ~V(); float Normalize(); }; // Normalize 0x47d630'
        self.assertEqual(member_addresses(class_blocks(block, 'V')[0], 'V', r'\bNormalize'), [0x47d630])

    def test_ambiguous_annotations_do_not_guess(self):
        block = 'struct V { float x,y,z; ~V(); float Length() const; }; // 0x47d5f0'
        self.assertEqual(member_addresses(block, 'V', r'\bLength'), [])
        block = 'float Length() const; // 0x47d5f0\nfloat Length() const; // 0x47d610'
        self.assertEqual(member_addresses(block, 'V', r'\bLength'), [0x47d5f0, 0x47d610])

    def test_decorated_operation_abi_boundaries(self):
        self.assertEqual(classify('??DV3@@QBE?AU0@M@Z')[0][0], 'multiply')
        self.assertEqual(classify('??GNearVector@@QAE?AU0@ABU0@@Z')[0][0], 'subtract')
        for symbol in ('??DV3@@QBE?AT0@M@Z', '??DV3@@QBE?AU0@N@Z',
                       '??DV3@@QBE?AU0@H@Z', '?Normalize@V3@@QAEHXZ',
                       '?Cross@V3@@QBE?AU1@U1@@Z'):
            self.assertIsNone(classify(symbol))


if __name__ == '__main__':
    unittest.main()
