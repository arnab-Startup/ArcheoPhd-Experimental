import shutil
import os

src = r"C:\Users\User\.gemini\antigravity\brain\c90e8e75-ee37-4548-8eca-40a3091e680b\archeology_desktop_wallpaper_1793996399691.jpg"

targets = [
    r"desktop\dist\assets\welcome-clean-bg.jpg",
    r"desktop\dist\assets\welcome-bg.jpg",
    r"desktop\dist\assets\welcome-backdrop.jpg"
]

for t in targets:
    shutil.copyfile(src, t)
    print("Copied to", t, "size:", os.path.getsize(t))

print("All targets updated successfully!")
