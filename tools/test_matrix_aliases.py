"""Affine method ABI, aggregate layout and annotation boundaries."""
import unittest
from matrix_aliases import classify, layout, annotated_addresses


class MatrixProof(unittest.TestCase):
    def test_matrix_and_zero_offset_aggregate(self):
        text = 'struct Data { float m[16]; };\nstruct Mat { Data d; Mat() {} ~Mat(); };'
        self.assertEqual(len(layout(text, 'Mat')), 2)
        self.assertEqual(len(layout('struct Mat { float v[16]; void Copy(const Mat &o) {} };', 'Mat')), 1)

    def test_wrong_storage_and_cycles_rejected(self):
        for body in ('float m[15];', 'double m[16];', 'float m[16]; int tag;',
                     'float m[16]; float (*callback)(float);', 'float m[16]; float extra = get();',
                     'Mat self;', 'float *m;', 'float m[16]; virtual void Update();'):
            self.assertFalse(layout('struct Mat { ' + body + ' };', 'Mat'))
        self.assertFalse(layout('struct Mat : Base { float m[16]; };', 'Mat'))
        self.assertFalse(layout('struct D { float m[16]; int tag; }; struct Mat { D d; };', 'Mat'))

    def test_typed_vector_references_require_xyz_order(self):
        self.assertTrue(layout('struct V { float x,y,z; };', 'V', vector=True))
        self.assertFalse(layout('struct V { float z,y,x; };', 'V', vector=True))
        self.assertFalse(layout('struct V { float x,y,z; int tag; };', 'V', vector=True))

    def test_multiline_annotations_and_conflicting_entries(self):
        text = 'Mat &SetRows(const V &r, const V &u,\nconst V &f, const V &p); // 0x409f00'
        self.assertEqual(annotated_addresses(text, 'SetRows'), [0x409f00])
        self.assertEqual(annotated_addresses('void TurnX(float x); // 0x47dcd0\nvoid TurnX(float x); // 0x47dd20', 'TurnX'), [0x47dcd0, 0x47dd20])
        self.assertEqual(annotated_addresses('void TurnX(float x); void TurnY(float y); // 0x47dcd0', 'TurnX'), [])

    def test_reference_pointer_and_discarded_scalar_result_abi(self):
        self.assertEqual(classify('?RotationX@Mat@@QAEXM@Z')['address'], 0x47dcd0)
        self.assertEqual(classify('?SetTranslation@Mat@@QAEAAU1@PBM@Z')['vector'], None)
        self.assertEqual(classify('?SetTranslation@Mat@@QAEXABUV@@@Z')['vector'], 'V')
        for symbol in ('?RotationX@Mat@@QAEXN@Z', '?RotationX@Mat@@YGXM@Z',
                       '?SetTranslation@Mat@@QAE?AU1@PBM@Z', '?SetRows@Mat@@QAEXUV@@000@Z'):
            self.assertIsNone(classify(symbol))


if __name__ == '__main__':
    unittest.main()
