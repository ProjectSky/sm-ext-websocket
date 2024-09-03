# vim: set sts=2 ts=8 sw=2 tw=99 et:
import sys
from ambuild2 import run

# Simple extensions do not need to modify this file.

ambuild_version = getattr(run, 'CURRENT_API', '2.1')
if ambuild_version.startswith('2.1'):
    sys.stderr.write("AMBuild 2.2 or higher is required; please update\n")
    sys.exit(1)

parser = run.BuildParser(sourcePath=sys.path[0], api='2.2')

parser.options.add_argument('--sm-path', type=str, dest='sm_path', default=None,
                       help='Path to SourceMod')
parser.options.add_argument('--enable-debug', action='store_const', const='1', dest='debug',
                       help='Enable debugging symbols')
parser.options.add_argument('--enable-optimize', action='store_const', const='1', dest='opt',
                       help='Enable optimization')
parser.options.add_argument('--targets', type=str, dest='targets', default=None,
                       help='Override the target architecture (use commas to separate multiple targets).')

parser.Configure()