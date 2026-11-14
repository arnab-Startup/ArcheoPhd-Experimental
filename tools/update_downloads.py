import shutil
import zipfile
import hashlib
import os

shutil.copy2('desktop/release/ArchaeoPhD.exe', 'web/downloads/ArchaeoPhD.exe')

with zipfile.ZipFile('web/downloads/ArchaeoPhD-Windows.zip', 'w', zipfile.ZIP_DEFLATED) as zf:
    zf.write('web/downloads/ArchaeoPhD.exe', 'ArchaeoPhD.exe')
    zf.write('web/downloads/WebView2Loader.dll', 'WebView2Loader.dll')

def get_hash(fn):
    h = hashlib.sha256()
    with open(fn, 'rb') as f:
        while True:
            chunk = f.read(65536)
            if not chunk:
                break
            h.update(chunk)
    return h.hexdigest().upper()

files = ['ArchaeoPhD-Setup.exe', 'ArchaeoPhD.exe', 'ArchaeoPhD-Windows.zip']
lines = [f"{get_hash(os.path.join('web/downloads', f))}  {f}" for f in files]

with open('web/downloads/checksums.sha256', 'w', encoding='utf-8') as f:
    f.write('\n'.join(lines) + '\n')

for l in lines:
    print(l)

