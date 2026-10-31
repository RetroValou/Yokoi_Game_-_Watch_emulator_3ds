"""Run Linux build executables against original synthetic fixtures and rejected packs."""
import os
from pathlib import Path
import struct
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
binary = Path(os.environ.get('YOKOI_BINARY', root/'linux/build/yokoi'))
smoke_binary = Path(os.environ.get('YOKOI_SMOKE_BINARY', root/'linux/build/smoke-test'))
env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
with tempfile.TemporaryDirectory(prefix='yokoi-smoke-') as temporary:
    base = Path(temporary)
    result = subprocess.run([str(binary), '--version'], cwd=base,
                            env=dict(env, SDL_VIDEODRIVER='yokoi-invalid'),
                            capture_output=True, text=True)
    assert result.returncode == 0 and result.stdout == 'Yokoi Linux 2.5\n' and not result.stderr, result
    result = subprocess.run([str(binary), '--help'], cwd=base, env=env,
                            capture_output=True, text=True)
    assert result.returncode == 0 and '--version' in result.stdout and 'Yokoi Linux 2.5' in result.stdout, result
    print('PASS: version and help without a ROM pack or video initialization')
    for mode in ('single', 'vertical', 'horizontal', 'mask', 'multi', 'clock'):
        folder = base / mode
        folder.mkdir()
        pack = folder / 'fixture.ykp'
        subprocess.run(['python3', str(root/'linux/tests/make_fixture.py'), str(pack), mode], check=True)
        subprocess.run([str(smoke_binary), str(pack), str(folder)],
                       env=dict(env, TZ='UTC') if mode == 'clock' else env, check=True)
    reordered = base/'multi/reordered.ykp'
    subprocess.run(['python3', str(root/'linux/tests/make_fixture.py'), str(reordered), 'multi-reordered'], check=True)
    subprocess.run([str(smoke_binary), str(reordered), str(base/'multi'), 'EL_02'], env=env, check=True)
    raw = bytearray((base/'single/fixture.ykp').read_bytes())
    for label, offset, value, expected in (
        ('wrong-platform', 8, 1, 'wrong pack platform'),
        ('old-content', 12, 1, 'rom pack too old'),
    ):
        data = raw.copy()
        struct.pack_into('<I', data, offset, value)
        pack = base/(label+'.ykp')
        pack.write_bytes(data)
        result = subprocess.run([str(binary), str(pack)], env=env, capture_output=True, text=True)
        assert result.returncode == 1 and expected in result.stderr, result
        assert result.stderr.startswith('Yokoi Linux 2.5\n'), result
        print('PASS:', label)
    result = subprocess.run([str(binary), str(base/'missing.ykp')], env=env, capture_output=True, text=True)
    assert result.returncode == 1 and 'open failed' in result.stderr
    assert result.stderr.startswith('Yokoi Linux 2.5\n'), result
    print('PASS: missing pack')
