"""Strict target-row grammar shared by contract with TargetRows.pm.

Rows are expression, hexadecimal address, label, and optional positive decimal
size or auto/ehend/cc. Never join physical lines implicitly.
"""
import re


def parse_target_row(line):
    fields = line.split()
    if not fields or fields[0].startswith('#'):
        return None
    if len(fields) not in (3, 4):
        raise ValueError('expected expression address label [size] on one line')
    expression, address, label = fields[:3]
    if not re.fullmatch(r'(?:0[xX])?[0-9a-fA-F]{1,8}', address):
        raise ValueError('invalid hexadecimal target address')
    if len(fields) == 4 and not re.fullmatch(r'(?:[1-9][0-9]*|auto|ehend|cc)', fields[3]):
        raise ValueError('invalid target size')
    return fields
