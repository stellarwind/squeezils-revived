import unittest
from integration_clusters import split_diagnostic, family_name, classify


class Triage(unittest.TestCase):
    def test_full_symbol_identity_and_display(self):
        name = '?Get@Thing@@QAEPAXH@Z'
        self.assertEqual(split_diagnostic('"public: void * __thiscall Thing::Get(int)"(' + name + ')'),
                         (name, 'public: void * __thiscall Thing::Get(int)'))
        self.assertEqual(split_diagnostic('_fn_401000'), ('_fn_401000', None))

    def test_template_argument_does_not_own_constructor(self):
        self.assertEqual(family_name('ctor', 'public: __thiscall NavProp::NavProp(class Array<int> *)'), 'NavProp')
        self.assertEqual(family_name('ctor', 'public: __thiscall FrameProp::FrameProp(class Table<class String,int> *)'), 'FrameProp')

    def test_nested_template_and_nested_class_owners(self):
        self.assertEqual(family_name('get', 'public: int __thiscall Table<class String,class Array<int> >::Get(int)'),
                         'Table<class String,class Array<int> >')
        self.assertEqual(family_name('ctor', 'public: __thiscall LoadedCompound::List::List(void)'), 'LoadedCompound::List')

    def test_global_data_and_free_functions(self):
        self.assertEqual(family_name('data', 'struct AmbientNameArray gAmbientNames'), 'global:gAmbientNames')
        self.assertEqual(family_name('fn', 'void __cdecl RegisterFunction(class Function *)'), 'global:RegisterFunction')
        self.assertEqual(family_name('?Alloc@@3P6APAXI@ZA', 'void * (__cdecl* Alloc)(unsigned int)'), 'global:Alloc')

    def test_acceptance_has_priority_over_other_generator_rejection(self):
        self.assertEqual(classify([{}], [], ['unproven references'], set(), [], False), 'accepted-awaiting-review')
        self.assertEqual(classify([{}], ['registry'], [], set(), [], False), 'registered-candidate-still-unresolved')

    def test_absent_mapping_is_not_invented_missing_implementation(self):
        self.assertEqual(classify([], [], [], {'00401000'}, [], False), 'missing-mapping')
        self.assertEqual(classify([], [], ['callee lacks unique complete exact-body owner'], set(), [], False), 'unknown')

    def test_data_and_caller_proof_failures_stay_distinct(self):
        reason = ['unproven/conflicting reference, nonzero addend or no unique extracted entry']
        self.assertEqual(classify([], [], reason, set(), [], True), 'data-identity-layout-unproven')
        self.assertEqual(classify([], [], reason, set(), [], False), 'unproven-caller')

    def test_competing_owner_candidates_are_preserved(self):
        owners = [dict(symbol='a', source='A.cpp'), dict(symbol='b', source='B.cpp')]
        self.assertEqual(classify([], [], [], {'00401000'}, owners, False), 'ambiguous-owner-candidates')


if __name__ == '__main__':
    unittest.main()
