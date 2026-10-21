"""Retire undefined COFF rows used exclusively by discarded function sections.

Keep symbol indices stable (including weak-external auxiliary indices). Retired
rows become private absolute bookkeeping markers, never exported definitions.
"""
import struct

from project_eh import parse


def retire(original, projected, removed_sections):
    before, sections = parse(original)
    after, kept_sections = parse(projected)
    discarded = {index for si in removed_sections
                 for _, index, _ in sections[si]['relocations']}
    retained = {index for sec in kept_sections.values()
                for _, index, _ in sec['relocations']}
    # Weak externals can refer to another symbol through their auxiliary row.
    for symbol in after.values():
        if symbol['storage'] == 105:
            if symbol['aux'] != 1:
                raise ValueError('Unsupported weak-external auxiliary layout')
            retained.add(struct.unpack_from('<I', projected, symbol['position'] + 18)[0])
    output = bytearray(projected)
    retired = []
    for index in sorted(discarded - retained):
        symbol = before[index]
        if symbol['storage'] != 2 or symbol['section'] != 0 or symbol['value']:
            continue  # Preserve common storage and all original definitions.
        if symbol['aux']:
            raise ValueError('Undefined external has unexpected auxiliary records')
        current = after[index]
        if current != symbol:
            raise ValueError('Projection changed candidate symbol row')
        struct.pack_into('<8sIhHBB', output, symbol['position'],
                         b'.sqzdead', 0, -1, 0, 3, 0)
        retired.append(symbol['name'])
    return output, retired
