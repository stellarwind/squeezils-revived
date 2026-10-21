"""Verify all retained symbols, payload and relocations in an EH projection."""
import json
import struct

import integration as build
from project_eh import parse


def main():
    before = (build.REPO / "data/data.obj").read_bytes()
    after = (build.OUT / "containers/data_eh.obj").read_bytes()
    report = json.loads((build.OUT / "containers/eh-projection.json").read_text())
    old_symbols, old_sections = parse(before)
    new_symbols, new_sections = parse(after)
    dropped = set(report["removed_symbols"])
    by_name = {s["name"]: s for s in new_symbols.values()}
    assert not dropped.intersection(by_name), "Discarded symbol remains in output"
    spans = list(report["removed_objects"].values())

    def removed(si, value):
        return any(section == si and lo <= value < hi for section, lo, hi in spans)

    def shift(si, value):
        return value - sum(hi - lo for section, lo, hi in spans if section == si and hi <= value)

    assert len(old_sections) == len(new_sections)
    relocated = 0
    for si, old in old_sections.items():
        new = new_sections[si]
        if not old["offset"]:
            assert new == old
            continue
        # Independently walk each retained original byte in order. This includes
        # unnamed trailing records inside the extractor's larger aggregates.
        expected = bytes(before[old["offset"] + i] for i in range(old["size"]) if not removed(si, i))
        actual = after[new["offset"]:new["offset"] + new["size"]]
        assert expected == actual, "Retained payload changed"
        expected_relocations = []
        for offset, index, typ in old["relocations"]:
            if removed(si, offset):
                continue
            target = old_symbols[index]["name"]
            assert target not in dropped
            expected_relocations.append((shift(si, offset), target, typ))
        actual_relocations = [(offset, new_symbols[index]["name"], typ)
                              for offset, index, typ in new["relocations"]]
        assert expected_relocations == actual_relocations, "Retained relocation changed"
        relocated += len(actual_relocations)
    for old in old_symbols.values():
        if old["name"] in dropped:
            continue
        new = by_name[old["name"]]
        expected = shift(old["section"], old["value"]) if old["section"] > 0 else old["value"]
        assert (new["section"], new["storage"], new["value"], new["aux"]) == (
            old["section"], old["storage"], expected, old["aux"]), old["name"]
    print("PASS: all retained payload, symbols and %d relocations preserved; %d dead symbols removed" %
          (relocated, len(dropped)))


if __name__ == "__main__":
    main()
