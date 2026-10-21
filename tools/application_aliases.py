"""Reviewed Application interfaces, including distinct set/clear flag entries."""
import json
import struct
import integration as build
from code_pointer_bindings import original_reader
from surface_aliases import discover as verify

OWNERS = {
    0x43bc70: ('?Value@Control43bc70@@QAEMXZ', 'Tiny4/F43bc70.cpp'),
    0x43bd20: ('?Value@Control43bd20@@QAEMXZ', 'Tiny4/F43bd20.cpp'),
    0x43bc10: ('?TakeX@Pad@@QAEMXZ', 'Misc10/Small.cpp'),
    0x43bc40: ('?TakeY@Pad@@QAEMXZ', 'Misc10/Small.cpp'),
    0x43a740: ('?Initialize@InputInitializer@@QAEHPAUListenerTable@@@Z', 'Input/Initialize.cpp'),
    0x43a900: ('?Cleanup@H43a900@@QAEHXZ', 'Tiny1/Tiny1.cpp'),
    0x43c970: ('?Reset@Clock@@QAEXXZ', 'Misc18/Flush.cpp'),
    0x43b980: ('?SetActive@Obj@@QAEXH@Z', 'Fn43b980/F.cpp'),
    0x43aa10: ('?f_43aa10@L@@QAEXXZ', 'Leaves/Leaves.cpp'),
    0x43aa20: ('?f_43aa20@L@@QAEXXZ', 'Leaves/Leaves.cpp'),
}


def bindings():
    result = {}
    specs = [
        ('ForwardAxis', 'QAEMXZ', 0x43bc70, 'float', None, None),
        ('TurnAxis', 'QAEMXZ', 0x43bd20, 'float', None, None),
        ('ForwardTap', 'QAEMXZ', 0x43bc10, 'float', None, None),
        ('TurnTap', 'QAEMXZ', 0x43bc40, 'float', None, None),
        ('Initialize', 'QAEHPAUWindowCallbacks@@@Z', 0x43a740, 'int', 'WindowCallbacks', 'pointer'),
        ('Cleanup', 'QAEXXZ', 0x43a900, 'void', None, None),
        ('ResetClock', 'QAEXXZ', 0x43c970, 'void', None, None),
        ('FlushInput', 'QAEXXZ', 0x43c970, 'void', None, None),
        ('SetInputMode', 'QAEXH@Z', 0x43b980, 'void', 'int', 'value'),
        ('SetFlag', 'QAEXXZ', 0x43aa10, 'void', None, None),
        ('ClearFlag', 'QAEXXZ', 0x43aa20, 'void', None, None),
    ]
    for method, suffix, address, returned, argument, mode in specs:
        abi = 'thiscall ECX=this unchanged; no stack argument; '
        abi += 'float in x87 ST0' if returned == 'float' else 'return ignored'
        if argument:
            abi = 'thiscall ECX=this unchanged; one four-byte argument; RET 4; ' + ('int EAX' if returned == 'int' else 'return ignored')
        result['?' + method + '@Application@@' + suffix] = dict(
            typename='Application', method=method, address=address, returned=returned,
            argument=argument, argument_mode=mode, argument_const=False, return_bytes=None,
            family='application', abi=abi,
            layout='controls +8/+10; axis timers +3c/+50, taps +40/+54; disabled +74, active +78; '
                   'mode +98; set/clear flag +b4. WindowCallbacks first 0x80 bytes are 16 callback/context pairs; '
                   'same Add entry as ListenerTable. Cleanup int result is deliberately ignored by void callers.')
    return result


def discover():
    report = verify(bindings(), OWNERS)
    read = original_reader()
    for decision in report['accepted'].values():
        decision['reason'] = ('Reviewed distinct application entries, declarations and layouts; all selectable '
                              'canonical definitions match complete original code with symbolic absolute operands. '
                              'Float returns retain ST0; set and clear at +b4 are separate names.')
    for method, instruction, destination, source in (
            ('ClearFlag', 0x417405, 0x43aa20, 'Sim2/RegionInit.cpp'),
            ('SetFlag', 0x43963c, 0x43aa10, 'Sim2/PlaceStart.cpp')):
        body = read(instruction, 5)
        if body[0] != 0xe8 or instruction + 5 + struct.unpack('<i', body[1:])[0] != destination:
            raise ValueError('Reviewed original application flag call changed: ' + method)
        alias = '?' + method + '@Application@@QAEXXZ'
        if alias in report['accepted']:
            report['accepted'][alias]['reviewed_original_call'] = dict(
                instruction='%08x' % instruction, destination='%08x' % destination, source=source)
    return report


if __name__ == '__main__':
    result = discover()
    (build.OUT / 'application-candidates.json').write_text(json.dumps(result, indent=2))
    print('Application interfaces:', len(result['accepted']), 'accepted;', len(result['rejected']), 'withheld')
