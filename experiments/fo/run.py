#!/usr/bin/env python3
"""Compile-and-run check for lib/emitc.cub: a first-order program is printed by the machine as a
list of bytes, decoded here, compiled with gcc, run, and its output compared with eval-fo's."""
import re, subprocess, sys, tempfile, os
HERE = os.path.dirname(os.path.abspath(__file__)); ROOT = os.path.dirname(os.path.dirname(HERE))
CUB = os.path.join(ROOT, 'build', 'cubical')
LIBS = [os.path.join(ROOT, 'lib', f) for f in ('prelude.cub', 'code.cub', 'fo.cub', 'match.cub', 'emitc.cub', 'check.cub', 'specialize.cub')]
PROGS = {
 # name: (definitions as .cub text building a Lst of (g , (arity , body)), expected output)
 'fact': ("""defs = lcons (#0 , (#0 , fcall #1 (lcons (flit #5) lnil)))
       (lcons (#1 , (#1 , fo-fact)) lnil)""", "120"),
 'switch': ("""defs = lcons (#0 , (#0 , fswitch (fcon #7 #1 (lcons (flit #9) (lcons (flit #30) lnil)))
                                 (lcons (#0 , (#0 , flit #0)) (lcons (#1 , (#2 , fprim P-add (lcons (fvar #1) (lcons (fvar #0) lnil)))) lnil)))) lnil""", "39"),
 # a code specialized by the machine (lib/specialize.cub), then printed and compiled: 2^n as a loop
 'specialized': ("""lam-body f = case f of flam b → b; _ → f
defs = lcons (#0 , (#0 , fcall #1 (lcons (flit #6) lnil)))
       (lcons (#1 , (#1 , lam-body (specialize (Word → Word) lnil c5))) lnil)""", "64"),
 # a closure capturing a let-bound value, lifted to its own definition and applied through the dispatcher
 'closure': ("""defs = case lift #1 t-clo of lifted t2 ds k → lcons (#0 , (#0 , t2)) ds""", "15"),
 'let-if': ("""defs = lcons (#0 , (#0 , flet (flit #3) (fif (fprim P-lt (lcons (fvar #0) (lcons (flit #5) lnil))) (fprim P-mul (lcons (fvar #0) (lcons (fvar #0) lnil))) (flit #0)))) lnil""", "9"),
}
def run(name, defs, expect):
    with tempfile.TemporaryDirectory() as d:
        cub = os.path.join(d, name + '.cub')
        open(cub, 'w').write(defs + "\n#show emit-prog defs\n")
        out = subprocess.run([CUB] + LIBS + [cub], capture_output=True, text=True, env=dict(os.environ, CUBICAL_NOCACHE='1')).stdout
        m = re.search(r'emit-prog defs\s*=\s*(.*?)\s*\[\d+ steps', out, re.S)
        if not m: print(name, 'FAIL: no reading'); print(out[-500:]); return False
        bs = bytes(int(x) for x in re.findall(r'#(\d+)', m.group(1)))
        c = os.path.join(d, name + '.c'); open(c, 'wb').write(bs)
        exe = os.path.join(d, name)
        cc = subprocess.run(['gcc', '-O2', '-w', '-o', exe, c], capture_output=True, text=True)
        if cc.returncode: print(name, 'FAIL: gcc'); print(cc.stderr[:800]); print(bs.decode()); return False
        got = subprocess.run([exe], capture_output=True, text=True).stdout.strip()
        ok = got == expect
        print(name, 'ok' if ok else f'FAIL: got {got}, expected {expect}')
        if not ok: print(bs.decode())
        return ok
if __name__ == '__main__':
    sys.exit(0 if all(run(n, *PROGS[n]) for n in PROGS) else 1)
