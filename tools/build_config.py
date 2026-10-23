"""Shared paths and prerequisite checks for building and testing the patch."""
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / 'build' / 'forward'
STAGE = BUILD / 'game'
GAME_FILES = ROOT.parent / 'SqueezilsGameFiles'
GAME_ARCHIVE = 'Squeezils_Win_EN_v042b_updated.zip'


def add_game_files_argument(parser):
    parser.add_argument('--game-files', type=Path, default=GAME_FILES, metavar='PATH',
                        help='original files directory (default: ../SqueezilsGameFiles)')


def require_files(paths, hint=''):
    missing = [str(path) for path in paths if not Path(path).is_file()]
    if missing:
        raise SystemExit('Missing required files:\n  ' + '\n  '.join(missing)
                         + ('\n' + hint if hint else ''))


def toolchain():
    if os.name != 'nt':
        raise SystemExit('Building and testing require Windows.')
    if sys.version_info < (3, 9):
        raise SystemExit('Python 3.9 or newer is required.')
    compiler = shutil.which('gcc')
    if not compiler:
        raise SystemExit("GCC not found. Add your 32-bit MinGW-w64 bin directory to PATH.")
    gcc = Path(compiler)
    try:
        target = subprocess.check_output([gcc, '-dumpmachine'], text=True).strip()
    except (OSError, subprocess.CalledProcessError) as error:
        raise SystemExit('Cannot run GCC: ' + str(error)) from error
    if target != 'i686-w64-mingw32':
        raise SystemExit('32-bit MinGW-w64 GCC is required; gcc -dumpmachine returned ' + target)
    prefix = gcc.parent.parent
    headers = ('ddraw.h', 'd3d.h', 'd3drm.h', 'd3drmobj.h')
    for sdk in (prefix / target / 'include', prefix / 'include'):
        if all((sdk / name).is_file() for name in headers):
            return gcc, sdk
    raise SystemExit('DirectX headers not found in this MinGW-w64 installation: ' + str(prefix))


def system_ddraw():
    # A 32-bit Python process accesses the x86 DLL through System32 redirection.
    folder = 'SysWOW64' if sys.maxsize > 2**32 else 'System32'
    path = Path(os.environ['WINDIR']) / folder / 'ddraw.dll'
    require_files([path])
    return path
