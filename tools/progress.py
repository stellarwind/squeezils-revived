"""Count admitted reconstruction functions and publish only README.md.

Usage: bash tools/mkstatus.sh [--dry-run | --no-push]
The default commits and pushes the README; --no-push only writes it locally.
"""
import argparse
import csv
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def git(*args):
    return subprocess.check_output(
        ['git', '-C', str(ROOT), *args], encoding='utf-8', stderr=subprocess.STDOUT
    ).strip()


def rows(path):
    with path.open(encoding='utf-8', newline='') as stream:
        return list(csv.DictReader(stream, delimiter='\t'))


def local_file(value):
    path = (ROOT / value).resolve()
    if not value or not path.is_relative_to(ROOT) or not path.is_file():
        raise ValueError(f'Missing or non-repository evidence/source: {value!r}')
    return path


def render():
    inventory = rows(ROOT / 'progress/functions.tsv')
    addresses = [int(row['address'], 16) for row in inventory]
    if not addresses or len(set(addresses)) != len(addresses):
        raise ValueError('Function inventory must be nonempty with unique addresses')
    known = set(addresses)
    accepted = set()
    for row in rows(ROOT / 'progress/accepted.tsv'):
        address = int(row['address'], 16)
        if address not in known or address in accepted:
            raise ValueError(f'Unknown or duplicate admitted address: {address:06x}')
        source = local_file(row['source'])
        if not source.is_relative_to(ROOT / 'src'):
            raise ValueError(f'Admitted source must be under src/: {row["source"]}')
        local_file(row['evidence'])
        accepted.add(address)
    done, total = len(accepted), len(known)
    filled = 30 * done // total
    bar = '\u2588' * filled + '\u2591' * (30 - filled)
    return (
        '## Current Progress\n\n'
        f'{bar} **{100 * done / total:.1f}%**\n\n'
        f'**{done} / {total}** game functions admitted to the source reconstruction.\n\n'
        'Function coverage against an estimated inventory; this does not measure '
        'playability. Archived implementations, QvLib, CRT and compiler cleanup '
        'funclets are excluded.\n'
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument('--dry-run', action='store_true', help='Print only; no writes or Git actions')
    mode.add_argument('--no-push', action='store_true', help='Write README only; no commit or push')
    args = parser.parse_args()
    content = render()
    if args.dry_run:
        sys.stdout.buffer.write(content.encode('utf-8'))
        return
    readme = ROOT / 'README.md'
    changed = not readme.exists() or readme.read_text(encoding='utf-8') != content
    if args.no_push:
        readme.write_bytes(content.encode('utf-8'))
        print('Updated README locally; no commit or push.')
        return
    if not changed and not git('diff', 'HEAD', '--', 'README.md'):
        print('Progress unchanged; nothing to commit or push.')
        return
    if git('diff', '--cached', '--name-only', '--', 'README.md'):
        raise ValueError('README already staged; finish that change before publishing progress')
    branch = git('symbolic-ref', '--short', 'HEAD')
    remote = git('config', f'branch.{branch}.remote')
    merge = git('config', f'branch.{branch}.merge')
    if remote == '.' or not merge.startswith('refs/heads/'):
        raise ValueError('A remote branch upstream is required')
    git('fetch', remote)
    if git('rev-list', '--left-right', '--count', '@{u}...HEAD').split() != ['0', '0']:
        raise ValueError('HEAD and upstream differ; reconcile them before publishing progress')
    readme.write_bytes(content.encode('utf-8'))
    git('add', '--', 'README.md')
    message = 'Update Current Progress'
    git('commit', '--only', '-m', message, '--', 'README.md')
    result = git('push', remote, f'HEAD:{merge}')
    print(result or 'Pushed Current Progress.')


if __name__ == '__main__':
    try:
        main()
    except (ValueError, KeyError, OSError, subprocess.CalledProcessError) as error:
        detail = getattr(error, 'output', None) or str(error)
        sys.exit(f'mkstatus: {detail}')
