#!/usr/bin/env python3
"""Exercise the selected rotation methods with the retained native fixture."""
import argparse
from pathlib import Path
import re
import subprocess
import tempfile


def block(source, marker):
    start = source.index(marker)
    opening = source.index('{', start)
    depth, end = 1, opening + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


parser = argparse.ArgumentParser()
parser.add_argument('--operations-root', required=True, type=Path)
args = parser.parse_args()
ops = args.operations_root.resolve(strict=True)
subprocess.run([str(ops / 'scripts/wow-build-host-guard')], check=True)
work = ops / 'work'
work.mkdir(exist_ok=True)
if not work.resolve().is_relative_to(ops):
    raise SystemExit('work directory resolves outside operations root')
module = Path(__file__).resolve().parents[1]
source = (module / 'src/Rotation.cpp').read_text()
fixture = (module / 'tests/rotation_fixture.cpp').read_text()
config = (module / 'conf/mod_rotation.conf.dist').read_text()
if not re.search(r'^Rotation\.AutoLearn\s*=\s*0\s*$', config, re.MULTILINE):
    raise SystemExit('Rotation.AutoLearn must default to disabled')
if 'GetOption<bool>("Rotation.AutoLearn", false)' not in source:
    raise SystemExit('missing disabled AutoLearn fallback')
variants = {
    'candidate': source,
    'missing-earned-check': source.replace('!info || !p->HasActiveSpell(info->Id)', '!info'),
    'missing-autoshot-check': source.replace(' && c.me->HasActiveSpell(75)', ''),
    'inactive-rank': source.replace('if (p->HasActiveSpell(info->Id))', 'if (p->HasSpell(info->Id))'),
}
with tempfile.TemporaryDirectory(prefix='rotation-fixture-', dir=work) as directory:
    for name, variant in variants.items():
        if name != 'candidate' and variant == source:
            raise SystemExit(f'{name}: mutation did not change source')
        body = variant[variant.index('uint32 MaxRank('):variant.index('bool IsBleedImmune(')]
        body += '\n' + block(variant, 'void PetAttack(') + '\n' + block(variant, 'void EngageRanged(')
        cpp = Path(directory) / f'{name}.cpp'
        binary = Path(directory) / name
        cpp.write_text(fixture.replace('@BODY@', body))
        subprocess.run(['clang++', '-std=c++17', '-Wall', '-Wextra', '-Werror', '-pedantic', '-O0',
                        '-fsanitize=undefined,float-cast-overflow', '-fno-sanitize-recover=all',
                        str(cpp), '-o', str(binary)], check=True, timeout=60)
        result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
        if (result.returncode == 0) != (name == 'candidate'):
            raise SystemExit(f'{name}: unexpected result\n{result.stdout}{result.stderr}')
        print(f'{name}: expected outcome ({result.stdout.strip() or result.stderr.strip()})')
