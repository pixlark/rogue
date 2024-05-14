import json, sys, mslex, subprocess

d = json.loads(sys.stdin.read())
for v in d:
    items = mslex.split(v["command"])
    c = subprocess.check_output(["cygpath", "-wm", items[0]]).decode().strip()
    v["command"] = mslex.join([c, *items[1:]])
print(json.dumps(d))
