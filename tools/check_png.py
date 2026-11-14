from PIL import Image
import os

for name in ['release_icon.png', 'installed_icon.png']:
    path = os.path.join('desktop', name)
    if os.path.exists(path):
        # Read header or check size
        print(name, "size:", os.path.getsize(path))

