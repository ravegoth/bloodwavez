#!/usr/bin/env python3
import sys, subprocess as s, os

def die(msg):
    print(f"ERROR: {msg}")
    sys.exit(1)

# verify inside a git repo
if not os.path.isdir('.git'):
    die('Not a git repository')

# combine all args and parse by separator
args = ' '.join(sys.argv[1:])
if '##' not in args:
    die('Usage: push msg ## branch')
msg, br = map(str.strip, args.split('##', 1))
if not msg or not br:
    die('Empty commit message or branch')

# protect main branches
if br in ('main', 'master'):
    if input(f"Push to protected branch '{br}'? Type YES to confirm: ") != 'YES':
        die('Aborted')

# confirmation
print_status = lambda t: sys.stdout.write(t) or sys.stdout.flush()
print(f"commit message: {msg}\nbranch: {br}")
try:
    input('Press ENTER to confirm...')
except KeyboardInterrupt:
    die('Aborted')

# execute git commands
for cmd in (['git','add','.'], ['git','commit','-m', msg], ['git','push','-u','origin', br]):
    try:
        s.run(cmd, check=True)
    except Exception as e:
        die(e)
