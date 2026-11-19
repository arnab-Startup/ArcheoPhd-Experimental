import urllib.request
import re
import os
import sys

URL = "https://ees.kuleuven.be/en/geography/projects/14c-palaeolithic/download"

def main():
    headers = {"User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"}
    req = urllib.request.Request(URL, headers=headers)
    try:
        with urllib.request.urlopen(req, timeout=15) as resp:
            html = resp.read().decode("utf-8", errors="ignore")
            print(f"Page fetched successfully ({len(html)} bytes).")
            links = re.findall(r'href=[\'"]([^\'"]+\.(?:xlsx|zip|csv|kmz))[\'"]', html, re.I)
            for l in sorted(set(links)):
                print(f"Found link: {l}")
    except Exception as e:
        print(f"Error fetching {URL}: {e}")

if __name__ == "__main__":
    main()
