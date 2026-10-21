"""Alias source-level global names to data.obj's address-named symbols.

data.obj (data/extract.py) names every global by address (g_4BADB8, s_4BF950).
Sources use readable names with the address in an `extern ... // 0x4badb8`
comment. For each unresolved data name whose declarations agree on exactly one
address present in data.obj, emit a COFF object holding a weak external
(IMAGE_SYM_CLASS_WEAK_EXTERNAL) from the readable mangled name to the data
symbol. The linker uses the alias only if nothing defines the name. No bytes
are copied and nothing is forced. Writes build/integration/data_alias.obj and
data_alias.json (aliases made, plus names skipped and why).

Run with flagsearch/env.sh sourced (needs dumpbin).
"""
import collections, json, os, pathlib, re, struct, subprocess

REPO = pathlib.Path(__file__).resolve().parents[1]
OUT = REPO / "build" / "integration"
DECL = re.compile(r'\b(\w+)(?:\[[^\]]*\])?\s*(?:=[^;]*)?;\s*//.*?0x([0-9a-fA-F]{6})')


def data_symbols():
    text = subprocess.run(["dumpbin", "-symbols", str(REPO / "data" / "data.obj")],
                          capture_output=True, text=True).stdout
    by_addr = collections.defaultdict(list)
    for line in text.splitlines():
        if "External" not in line or "SECT" not in line:
            continue
        m = re.search(r'\| (\?(\w)_([0-9A-F]+)@@\S+)', line)
        if m:
            by_addr[m.group(3).lstrip("0")].append(m.group(1))
    return by_addr


def wanted():
    # Current loose-object references are independent of previous link/alias runs.
    grouped = json.loads((OUT / "external-references.json").read_text())
    names = collections.defaultdict(set)
    for key in grouped:
        m = re.fullmatch(r'(\?(\w+)@@3\S*)', key)
        if m:
            names[m.group(2)].add(m.group(1))
    return names


def source_addresses(names):
    found = collections.defaultdict(set)
    for path in (REPO / "src").rglob("*"):
        if path.suffix not in (".h", ".cpp"):
            continue
        for line in path.read_text(encoding="latin-1").splitlines():
            if "extern" not in line:
                continue
            m = DECL.search(line)
            if m and m.group(1) in names:
                found[m.group(1)].add(m.group(2).upper().lstrip("0"))
    return found


def write_coff(pairs, path):
    if any(alias == target for alias, target in pairs):
        raise ValueError('Identity weak aliases are invalid')
    if len({alias for alias, _ in pairs}) != len(pairs):
        raise ValueError('Duplicate weak alias definitions')
    strtab = bytearray(4)
    symbols = bytearray()
    index = {}

    def name_field(name):
        raw = name.encode()
        if len(raw) <= 8:
            return raw.ljust(8, b"\0")
        off = len(strtab)
        strtab.extend(raw + b"\0")
        return struct.pack("<II", 0, off)

    def add(name, cls, aux=b""):
        index[name] = len(symbols) // 18
        symbols.extend(name_field(name) + struct.pack("<IhHBB", 0, 0, 0, cls, 1 if aux else 0))
        symbols.extend(aux)

    for alias, target in pairs:
        if target not in index:
            add(target, 2)
        add(alias, 105, struct.pack("<II10x", index[target], 3))
    count = len(symbols) // 18
    strtab[0:4] = struct.pack("<I", len(strtab))
    header = struct.pack("<HHIIIHH", 0x14C, 0, 0, 20, count, 0, 0)
    path.write_bytes(header + symbols + strtab)


def main():
    by_addr = data_symbols()
    names = wanted()
    found = source_addresses(names)
    pairs, skipped = [], {}
    registry_path = REPO / "tools/data_alias_registry.json"
    registry = json.loads(registry_path.read_text())
    for name, decorated in sorted(names.items()):
        if len(decorated) != 1:
            if all(alias in registry and registry[alias]["address"] in found.get(name, set())
                   for alias in decorated) and len(found.get(name, set())) == 1:
                for alias in sorted(decorated):
                    decision = registry[alias]
                    for source in decision["evidence"]:
                        if not (REPO / "src" / source).is_file():
                            raise ValueError("Missing reviewed data evidence: " + source)
                    targets = by_addr.get(decision["address"])
                    if not targets or len(targets) != 1:
                        raise ValueError("No unique reviewed data owner: " + alias)
                    pairs.append((alias, targets[0]))
                continue
            skipped[name] = "incompatible decorated declarations: " + ",".join(sorted(decorated))
            continue
        mangled = next(iter(decorated))
        addrs = found.get(name, set())
        if len(addrs) != 1:
            skipped[name] = "no address" if not addrs else "ambiguous " + ",".join(sorted(addrs))
            continue
        targets = by_addr.get(next(iter(addrs)))
        if not targets or len(targets) != 1:
            skipped[name] = "no unique data.obj symbol"
            continue
        pairs.append((mangled, targets[0]))
    exact_registry_path = REPO / 'tools/exact_data_registry.json'
    if exact_registry_path.exists():
        from exact_data_aliases import discover
        current = discover({alias for alias, _ in pairs})
        (OUT / 'exact-data-candidates.json').write_text(json.dumps(current, indent=2))
        for alias, decision in json.loads(exact_registry_path.read_text()).items():
            if current['accepted'].get(alias) != decision:
                raise ValueError('Exact data proof changed; review registry: ' + alias)
            pairs.append((alias, decision['target']))
        pairs.sort()
    write_coff(pairs, OUT / "data_alias.obj")
    (OUT / "data_alias.json").write_text(json.dumps(dict(aliases=pairs, skipped=skipped,
        reviewed_registry=registry,
        exact_registry_sha256=__import__('hashlib').sha256(exact_registry_path.read_bytes()).hexdigest() if exact_registry_path.exists() else None), indent=1))
    print(len(pairs), "aliases;", len(skipped), "skipped")


if __name__ == "__main__":
    main()
