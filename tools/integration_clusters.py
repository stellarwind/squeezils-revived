"""Evidence-based triage; this report never approves or emits aliases."""
import argparse
import collections
import hashlib
import json
import pathlib
import re
import integration as build
from project_eh import parse


def split_diagnostic(diagnostic):
    match = re.fullmatch(r'"(.*)"\(([^()]+)\)', diagnostic)
    return (match[2], match[1]) if match else (diagnostic, None)


def family_name(symbol, display):
    """Read the owner before arguments, respecting template nesting."""
    if not display:
        return 'code-pointers' if re.fullmatch(r'_fn_[0-9a-fA-F]+', symbol) else 'unknown:' + symbol
    depth, spaces, scopes = 0, [], []
    end = len(display)
    for i, char in enumerate(display):
        if char == '<':
            depth += 1
        elif char == '>' and depth:
            depth -= 1
        elif not depth:
            if char == '(':
                end = i
                break
            if char == ' ':
                spaces.append(i)
            if display[i:i + 2] == '::':
                scopes.append(i)
    limit = scopes[-1] if scopes else end
    start = max((i + 1 for i in spaces if i < limit), default=0)
    entity = display[start:limit].strip()
    if scopes:
        return entity
    # A global function-pointer declaration has parentheses before its name.
    # Its decorated global entity remains unambiguous and excludes argument types.
    global_name = re.match(r'^\?([^@]+)@@', symbol)
    return 'global:' + (global_name[1] if global_name else entity or symbol)


def classify(accepted, registered, reasons, addresses, owners, is_data):
    if accepted:
        return 'registered-candidate-still-unresolved' if registered else 'accepted-awaiting-review'
    text = ' '.join(reasons).lower()
    if 'abi' in text or 'calling convention' in text or 'requires cdecl' in text:
        return 'conflicting-declaration-abi'
    if is_data and ('conflict' in text or 'addend' in text or 'extracted' in text):
        return 'data-identity-layout-unproven'
    if 'unproven reference' in text or 'unproven/conflicting' in text:
        return 'unproven-caller'
    if 'missing explicit' in text:
        return 'missing-mapping'
    if len(addresses) > 1:
        return 'conflicting-original-addresses'
    if addresses and not owners:
        return 'missing-mapping'
    if len({(o['symbol'], o['source']) for o in owners}) > 1:
        return 'ambiguous-owner-candidates'
    if owners:
        return 'owner-proof-incomplete'
    return 'unknown'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--run-id')
    args = parser.parse_args()
    paths = set()
    def read(path):
        paths.add(path)
        return json.loads(path.read_text())
    unresolved = read(build.OUT / 'containers/unresolved-grouped.json')
    diagnostic = read(build.OUT / 'containers/partial-link-summary.json')
    if not diagnostic['diagnostic_pass_completed']:
        raise ValueError('Triage requires a completed diagnostic link')
    external = read(build.OUT / 'external-references.json')
    units = read(build.OUT / 'units.json')
    ownership = read(build.OUT / 'ownership.json')
    exact = read(build.OUT / 'exact-call-candidates.json')
    call_addresses = {n: {s['address'] for s in sites} for n, sites in exact['call_sites'].items()}
    sources = {u['object'].replace('\\', '/').split('/Repo/')[-1]: u['source'] for u in units}
    registered = collections.defaultdict(list)
    for path in sorted((build.REPO / 'tools').glob('*registry.json')):
        registry = read(path)
        # EH retirement and field-data policy registries are not alias maps.
        if isinstance(registry, dict):
            for symbol in registry:
                if symbol.startswith(('?', '_')):
                    registered[symbol].append(path.name)
    discoveries = collections.defaultdict(list)
    for path in sorted(build.OUT.glob('*candidates.json')):
        report = read(path)
        accepted = report.get('accepted', report if all(k.startswith('?') for k in report) else {})
        if isinstance(accepted, dict):
            for symbol, decision in accepted.items():
                if isinstance(decision, dict) and 'target' in decision:
                    discoveries[symbol].append(dict(generator=path.name, accepted=True, decision=decision))
                else:
                    discoveries[symbol].append(dict(generator=path.name, accepted=False,
                        reason='Discovery hint only; no accepted ownership proof', hint=decision))
        for key in ('rejected', 'withheld'):
            for symbol, reason in report.get(key, {}).items():
                discoveries[symbol].append(dict(generator=path.name, accepted=False, reason=reason))
    sites = collections.defaultdict(list)
    for unit in units:
        if unit['returncode']:
            raise ValueError('Triage requires complete compiled inventory')
        path = pathlib.Path(unit['object'])
        raw = path.read_bytes()
        paths.add(path)
        symbols, sections = parse(raw)
        for section, data in sections.items():
            for offset, index, kind in data['relocations']:
                target = symbols[index]
                if target['section'] == 0 and target['storage'] == 2:
                    sites[target['name']].append(dict(source=unit['source'], section=section,
                                                     offset=offset, relocation_type=kind))
    rows = {}
    for text, messages in unresolved.items():
        symbol, display = split_diagnostic(text)
        row = rows.setdefault(symbol, dict(symbol=symbol, display=display, diagnostics=[], messages=0))
        row['diagnostics'].append(text)
        row['messages'] += messages
    groups = collections.defaultdict(list)
    pending = []
    for symbol, row in sorted(rows.items()):
        evidence = discoveries[symbol]
        accepted = [e for e in evidence if e['accepted']]
        addresses = set(ownership['symbol_addresses'].get(symbol, []))
        for e in accepted:
            address = e['decision'].get('address')
            if address is not None:
                addresses.add('%08x' % address if isinstance(address, int) else address)
        addresses.update(call_addresses.get(symbol, ()))
        owners = [dict(address=address, **o) for address in sorted(addresses)
                  for o in ownership['addresses'].get(address, [])]
        reasons = [str(e['reason']) for e in evidence if not e['accepted']]
        is_data = bool(re.search(r'@@[23]', symbol))
        family = family_name(symbol, row['display'])
        row.update(implementation_family=family, original_addresses=sorted(addresses),
                   candidate_owners=owners, registries=registered[symbol], discovery=evidence,
                   blocker=classify(accepted, registered[symbol], reasons, addresses, owners, is_data),
                   referring_sources=sorted({sources.get(obj.replace('\\', '/'), obj)
                                             for obj, _ in external.get(symbol, [])}),
                   compiled_relocation_sites=sites[symbol])
        groups[family].append(row)
        if accepted and not registered[symbol]:
            pending.append(dict(symbol=symbol, generators=sorted(e['generator'] for e in accepted),
                                candidates=[e['decision'] for e in accepted]))
    if len(rows) != diagnostic['unique_unresolved_messages']:
        raise ValueError('Unique decorated names do not reconcile with diagnostic total')
    if sum(r['messages'] for r in rows.values()) != diagnostic['linker_codes'].get('LNK2001', 0):
        raise ValueError('Message count does not reconcile with completed diagnostic')
    clusters = {family: dict(unique_symbols=len(values), messages=sum(r['messages'] for r in values),
                            affected_sources=len({s for r in values for s in r['referring_sources']}),
                            blockers=dict(collections.Counter(r['blocker'] for r in values)),
                            symbols=sorted(values, key=lambda r: (-r['messages'], r['symbol'])))
                for family, values in sorted(groups.items())}
    # Outside a pipeline, only attach its run ID if every consumed generated
    # input and registry still agrees with that completed run's provenance.
    run_id = args.run_id
    hashes = {str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(paths)}
    if not run_id:
        completed = json.loads((build.OUT / 'pipeline-summary.json').read_text())
        if not completed['diagnostic_pass_completed']:
            raise ValueError('Standalone triage requires a completed pipeline')
        for path in paths:
            if path.is_relative_to(build.OUT):
                expected = completed['generated_hashes'].get(path.relative_to(build.OUT).as_posix())
                # Windows pipeline stores relative paths with native separators.
                expected = expected or completed['generated_hashes'].get(str(path.relative_to(build.OUT)))
            else:
                expected = completed['input_hashes'].get(str(path))
            if expected != hashes[str(path)]:
                raise ValueError('Triage input differs from completed pipeline: ' + str(path))
        run_id = completed['run_id']
    report = dict(schema_version=2, run_id=run_id,
                  warning='Candidate owners and blockers are triage, not approved bindings. '
                          'Compiled relocation sites include sites later retired by projection. '
                          'Unknown evidence stays explicit; messages are not relocation counts.',
                  input_sha256=hashes,
                  unique_symbols=len(rows), messages=sum(r['messages'] for r in rows.values()),
                  blockers=dict(collections.Counter(r['blocker'] for r in rows.values())),
                  accepted_unregistered=pending, clusters=clusters,
                  ranked_families=sorted(clusters, key=lambda k: (-clusters[k]['unique_symbols'],
                                                                -clusters[k]['affected_sources'], k)))
    (build.OUT / 'owner-clusters.json').write_text(json.dumps(report, indent=2))
    print(json.dumps({k: report[k] for k in ('unique_symbols', 'messages', 'blockers')}, indent=2))
    print('Pending accepted names:', len(pending))
    print('Largest owning families:', ', '.join(report['ranked_families'][:12]))


if __name__ == '__main__':
    main()
