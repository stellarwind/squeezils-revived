"""Generate level-entrance saves using the original v1.1 ASCII serializer layout.

The original loader resumes RegionSimulation even for a save made inside a level.
These saves therefore put the player at each level's overworld entrance. Nothing
in the installed INIs or existing saves is changed. See technicalinfo.md for the
original serializer/parser addresses and the fields intentionally synthesized.
"""
import argparse
import configparser
import json
from pathlib import Path
import zipfile

ROOT = Path(__file__).resolve().parents[1]
HEADER = '#SqzSave v1.1 ASCII'


def ini(path):
    data = configparser.ConfigParser(strict=False, interpolation=None,
                                    inline_comment_prefixes=(';',))
    # Win32 INI readers ignore bare non-key lines present in a few stock files.
    lines = [line.strip() for line in path.read_text(encoding='cp1252').splitlines()
             if line.strip().startswith('[') or '=' in line]
    data.read_string('\n'.join(lines), source=str(path))
    return data


def save_bytes(region, location):
    # Inventory's constructor initializes the scalar to -1. Six rows contain
    # three region keys plus the region-completed flag. Unlock them for testing,
    # while keeping the inventory/PlacesCompleted collection empty.
    lines = [HEADER, f'{region} {location}', 'TEST', '-1']
    lines += ['1 1 1 1'] * 6
    lines += ['0 inventory items!']
    return ('\r\n'.join(lines) + '\r\n').encode('ascii')


def write_new(path, content):
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists():
        if path.read_bytes() != content:
            raise FileExistsError(f'Refusing to replace a different existing file: {path}')
    else:
        with path.open('xb') as file:
            file.write(content)


def generate(game, output, install):
    sqz = game / 'SQZ'
    settings = ini(sqz / 'SQZ.INI')
    entries = []
    for region_text, relative in settings['RegionList'].items():
        region = int(region_text)
        region_dir = sqz.joinpath(*relative.split('\\'))
        definition = ini(region_dir / 'REGION' / 'REGION.INI')
        for location_text, section in definition['LocationList'].items():
            if definition.get(section, 'type', fallback='').lower() != 'place':
                continue
            location = int(location_text)
            destination = definition.getint(section, 'destination')
            place_ini = region_dir / f'PLACE{destination:02d}' / 'PLACE.INI'
            if not place_ini.is_file():
                raise ValueError(f'Level definition missing: {place_ini}')
            level = ini(place_ini)
            level_file = place_ini.parent / level.get('Defaults', 'LevelFile')
            if not level_file.is_file():
                raise ValueError(f'Level data missing: {level_file}')
            save_name = f'{region_dir.name.title()}{destination}'
            if not save_name.isascii() or not save_name.isalnum() or len(save_name) > 10:
                raise ValueError(f'Save name exceeds the original dialog limits: {save_name}')
            entries.append(dict(name=save_name, region=region,
                                location=location, place=destination,
                                region_name=region_dir.name, kind='level entrance',
                                definition=str(place_ini.relative_to(sqz)),
                                level_file=str(level_file.relative_to(sqz))))
    for entry in entries:
        content = save_bytes(entry['region'], entry['location'])
        # Independent format/definition checks before writing or installing.
        lines = content.decode('ascii').splitlines()
        assert len(lines) == 11 and lines[0] == HEADER
        assert tuple(map(int, lines[1].split())) == (entry['region'], entry['location'])
        assert all(len(list(map(int, row.split()))) == 4 for row in lines[4:10])
        assert lines[10] == '0 inventory items!'
        write_new(output / 'SAVE' / entry['name'], content)
        if install:
            write_new(sqz / 'SAVE' / entry['name'], content)
    output.mkdir(parents=True, exist_ok=True)
    (output / 'manifest.json').write_text(json.dumps(entries, indent=2) + '\n')
    (output / 'README.txt').write_text(
        'Squeezils testing saves\n\n'
        'Names identify the region and level: River1, Jungle3, Tomb1, etc.\n'
        'Use F1 / Load-Save and choose the level name, then enter the level.\n'
        'The original save loader always resumes the overworld, not mid-level.\n'
        'All six sets of region keys/completion flags are unlocked for testing.\n'
        'Objectives and collectible inventory start fresh; these are synthetic profiles.\n'
        'Copy only the files inside SAVE into your installation\'s SQZ/SAVE folder.\n'
        'Existing saves are never overwritten. Full list: manifest.json.\n'
        'Format checked against original code; loading in-game awaits user verification.\n')
    with zipfile.ZipFile(output / 'squeezils-test-saves.zip', 'w', zipfile.ZIP_DEFLATED) as archive:
        for entry in entries:
            archive.write(output / 'SAVE' / entry['name'], f'SAVE/{entry["name"]}')
        for name in ('README.txt', 'manifest.json'):
            archive.write(output / name, name)
    print(json.dumps(dict(level_saves=sum(e['kind'] == 'level entrance' for e in entries),
                          installed=install, output=str(output)), indent=2))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game', type=Path, default=ROOT / 'build/forward/game')
    parser.add_argument('--output', type=Path, default=ROOT / 'test-saves')
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    generate(args.game.resolve(), args.output.resolve(), args.install)
