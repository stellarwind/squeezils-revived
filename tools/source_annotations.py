"""Associate extern variable names with explicit original addresses in order."""
import re


def annotated(line):
    if 'extern' not in line or '//' not in line:
        return []
    declaration, comment = line.split('//', 1)
    names = re.findall(r'\b(\w+)\s*(?:\[[^\]]*\])?\s*(?=,|;)', declaration)
    addresses = re.findall(r'0x([0-9a-fA-F]{6,8})\b', comment)
    # A bounded contiguous range is explicit evidence only for a declaration
    # consisting solely of ordinary 32-bit scalar variables.
    span = re.search(r'0x([0-9a-fA-F]{6,8})\.\.([0-9a-fA-F]{2,8})\b', comment)
    scalar = re.fullmatch(r'\s*extern\s+(?:float|int|unsigned int|long)\s+'
                          r'\w+(?:\s*,\s*\w+)*\s*;\s*', declaration)
    if span:
        if not scalar or len(addresses) != 1 or not names:
            return []
        first, suffix = span.groups()
        last = int(first[:-len(suffix)] + suffix, 16) if len(suffix) < len(first) else int(suffix, 16)
        start = int(first, 16)
        if last - start != 4 * (len(names) - 1):
            return []
        return [(name, '%08x' % (start + 4 * i)) for i, name in enumerate(names)]
    if len(names) != len(addresses):
        return []
    return [(name, '%08x' % int(address, 16)) for name, address in zip(names, addresses)]
