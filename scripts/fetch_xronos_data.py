import urllib.request
import urllib.parse
import json
import csv
import sys
import os

ISO_COUNTRIES = ["CH", "DE", "IL", "GB"]
OUTPUT_CSV = os.path.join(os.path.dirname(__file__), "..", "data", "xronos_10k.csv")

def fetch_country_data(country_code):
    url = f"https://xronos.ch/api/v1/data?query_country={urllib.parse.quote(country_code)}"
    headers = {"User-Agent": "ArchaeoPhD-EpistemicEngine/1.0"}
    req = urllib.request.Request(url, headers=headers)
    print(f"Fetching XRONOS data for {country_code}...")
    try:
        with urllib.request.urlopen(req, timeout=30) as resp:
            if resp.status == 200:
                data = json.loads(resp.read().decode("utf-8"))
                if data:
                    print(f"  Retrieved {len(data)} records for {country_code}")
                    return data
    except Exception as e:
        print(f"  Error fetching {country_code}: {e}")
    return []

def main():
    all_measurements = []
    for c in ISO_COUNTRIES:
        records = fetch_country_data(c)
        for item in records:
            if isinstance(item, dict) and "measurement" in item:
                all_measurements.append(item["measurement"])
            elif isinstance(item, dict):
                all_measurements.append(item)

        print(f"  Current cumulative measurements: {len(all_measurements)}")
        if len(all_measurements) >= 12000:
            print("Reached target record count (> 10,000), stopping fetch.")
            break

    if not all_measurements:
        print("No measurements retrieved.")
        sys.exit(1)

    fields = ["labnr", "site", "feature", "stratum", "bp", "std", "material", "species", "country", "lat", "lng", "period"]
    
    os.makedirs(os.path.dirname(OUTPUT_CSV), exist_ok=True)
    valid_rows = 0
    with open(OUTPUT_CSV, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=fields, extrasaction="ignore")
        writer.writeheader()
        
        for m in all_measurements:
            labnr = str(m.get("labnr", "") or "").strip()
            bp_val = m.get("bp")
            std_val = m.get("std")
            
            if not labnr or bp_val is None or str(bp_val).strip() in ("", "None", "NA", "null"):
                continue

            try:
                bp_float = float(bp_val)
                std_float = float(std_val) if std_val not in (None, "", "None", "NA", "null") else 40.0
            except (ValueError, TypeError):
                continue

            # Period extraction
            period_str = ""
            periods = m.get("periods")
            if isinstance(periods, list) and periods:
                first_p = periods[0]
                if isinstance(first_p, dict):
                    period_str = first_p.get("periode", "")
                elif isinstance(first_p, str):
                    period_str = first_p

            row = {
                "labnr": labnr,
                "site": str(m.get("site", "") or "").strip(),
                "feature": str(m.get("feature", "") or "").strip(),
                "stratum": str(m.get("feature_type", "") or "").strip(),
                "bp": f"{bp_float:.1f}",
                "std": f"{std_float:.1f}",
                "material": str(m.get("material", "") or "").strip(),
                "species": str(m.get("species", "") or "").strip(),
                "country": str(m.get("country", "") or "").strip(),
                "lat": str(m.get("lat", "") or "").strip(),
                "lng": str(m.get("lng", "") or "").strip(),
                "period": period_str
            }
            writer.writerow(row)
            valid_rows += 1

    print(f"Successfully saved {valid_rows} clean real-world radiocarbon rows to {OUTPUT_CSV}")

if __name__ == "__main__":
    main()
