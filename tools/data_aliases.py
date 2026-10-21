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
from source_annotations import annotated

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
            for name, address in annotated(line):
                if name in names:
                    found[name].add(address.upper().lstrip('0'))
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


def validate_pointer_decision(alias, decision, found, targets, sites, source_text):
    """A reviewed type spelling changes neither pointer storage nor address."""
    match = re.fullmatch(r'\?(\w+)@@3PA(?:X|[UV]\w+@@)A', alias)
    if not match or decision['storage'] != 'one mutable 32-bit object pointer':
        raise ValueError('Reviewed pointer alias has incompatible storage: ' + alias)
    name, address = match[1], decision['address']
    if found.get(name) != {address} or len(targets.get(address, [])) != 1:
        raise ValueError('Reviewed pointer address/owner changed: ' + alias)
    if not decision['evidence']:
        raise ValueError('Reviewed pointer has no declaration evidence')
    for source in decision['evidence']:
        declarations = [DECL.search(line) for line in source_text(source).splitlines() if 'extern' in line]
        if not any(m and m[1] == name and m[2].upper().lstrip('0') == address for m in declarations):
            raise ValueError('Reviewed pointer declaration evidence changed: ' + source)
    current_sites = sites.get(alias, [])
    if any(int(site['address'], 16) != int(address, 16) for site in current_sites):
        raise ValueError('Reviewed pointer conflicts with original reference: ' + alias)
    if 'exact_reference_sites' in decision and current_sites != decision['exact_reference_sites']:
        raise ValueError('Reviewed pointer exact-reference evidence changed: ' + alias)


def main():
    by_addr = data_symbols()
    names = wanted()
    found = source_addresses(names)
    pairs, skipped = [], {}
    registry_path = REPO / "tools/data_alias_registry.json"
    registry = json.loads(registry_path.read_text())
    from exact_data_aliases import discover
    all_exact = discover(set())
    registry_sites = all_exact['reference_sites']
    for alias, decision in registry.items():
        validate_pointer_decision(alias, decision, found, by_addr, registry_sites,
            lambda source: (REPO / 'src' / source).read_text(encoding='latin-1'))
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
    # An explicit declaration must never override contradictory machine-code
    # evidence. Keep the diagnostic honest for symbols used at several original
    # addresses until their source-level spellings are separated.
    reference_conflicts = {}
    checked = []
    for alias, target in pairs:
        address = int(re.fullmatch(r'\?[gs]_([0-9A-Fa-f]+)@@\S+', target)[1], 16)
        sites = registry_sites.get(alias, [])
        if any(int(site['address'], 16) != address for site in sites):
            reference_conflicts[alias] = dict(proposed_target=target, exact_reference_sites=sites)
            skipped[alias] = 'original reference contradicts declaration address'
        else:
            checked.append((alias, target))
    pairs = checked
    exact_registry_path = REPO / 'tools/exact_data_registry.json'
    if exact_registry_path.exists():
        exact_registry = json.loads(exact_registry_path.read_text())
        current = discover({alias for alias, _ in pairs} - set(exact_registry))
        (OUT / 'exact-data-candidates.json').write_text(json.dumps(current, indent=2))
        for alias, decision in exact_registry.items():
            if current['accepted'].get(alias) != decision:
                raise ValueError('Exact data proof changed; review registry: ' + alias)
            previous = dict(pairs).get(alias)
            if previous is not None and previous != decision['target']:
                raise ValueError('Annotated and exact data owners disagree: ' + alias)
            if previous is None:
                pairs.append((alias, decision['target']))
        pairs.sort()
    # RTTI descriptors have a dedicated metadata owner. Some cached extracted
    # blocks contain several descriptors; the old single-name annotation regex
    # can otherwise bind the last declaration to the first comment address.
    rtti_registry = REPO / 'tools/rtti_alias_registry.json'
    rtti_names = set(json.loads(rtti_registry.read_text())) if rtti_registry.exists() else set()
    superseded = [(a, t) for a, t in pairs if a in rtti_names]
    pairs = [(a, t) for a, t in pairs if a not in rtti_names]
    write_coff(pairs, OUT / "data_alias.obj")
    (OUT / "data_alias.json").write_text(json.dumps(dict(aliases=pairs, skipped=skipped,
        original_reference_conflicts=reference_conflicts,
        reviewed_registry=registry,
        superseded_descriptor_aliases=superseded,
        exact_registry_sha256=__import__('hashlib').sha256(exact_registry_path.read_bytes()).hexdigest() if exact_registry_path.exists() else None), indent=1))
    print(len(pairs), "aliases;", len(skipped), "skipped")


if __name__ == "__main__":
    main()
