"""Check callable CRT owners and reviewed shared cleanup ABIs."""
import json
import pathlib
import unittest

import integration as build
from code_pointer_bindings import bind


class BindingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        report = json.loads((build.OUT / "runtime_alias.json").read_text())
        pairs, cls.evidence, cls.diagnostics, cls.startup = bind(pathlib.Path(report["library"]))
        cls.pairs = dict(pairs)

    def test_floating_hook_is_function_not_function_pointer_variable(self):
        self.assertEqual(self.pairs["_fn_48E930"], "__fpclear")
        proof = next(d for d in self.diagnostics if d["address"] == "0048e930")
        self.assertEqual((proof["target"], proof["size"], proof["equal"]), ("__fpclear", 1, True))

    def test_heap_and_error_entries_have_exact_crt_owners(self):
        expected = {"_fn_4904C0": "_malloc", "_fn_490540": "_free", "_fn_490560": "_realloc",
                    "_fn_48E4D0": "__purecall", "_fn_492E50": "?terminate@@YAXXZ",
                    "_fn_497E00": "?__CxxUnhandledExceptionFilter@@YGJPAU_EXCEPTION_POINTERS@@@Z"}
        for alias, target in expected.items():
            self.assertEqual(self.pairs[alias], target)

    def test_exception_copy_constructor_not_string_constructor(self):
        self.assertEqual(self.pairs["_fn_48EE10"], "??0__non_rtti_object@@QAE@ABV0@@Z")
        self.assertFalse(next(d for d in self.diagnostics if d["target"] == "??0__non_rtti_object@@QAE@PBD@Z")["equal"])

    def test_rebuilt_crt_owns_old_initializer(self):
        self.assertEqual(len(self.startup), 1)
        self.assertEqual((self.startup[0]["address"], self.startup[0]["initializer"]), ("004b9004", "00497dd0"))

    def test_shared_empty_methods_keep_argument_cleanup(self):
        self.assertEqual(self.pairs["_fn_471AF0"], self.pairs["_fn_471B10"])
        self.assertEqual(self.pairs["_fn_471B00"], "?ReportError@Bitmap@@QAEXJ@Z")


if __name__ == "__main__":
    unittest.main()
