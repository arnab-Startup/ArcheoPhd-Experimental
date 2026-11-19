import urllib.request
import csv
import os
import sys

URL_MED = "https://raw.githubusercontent.com/zoometh/neonet/main/inst/extdata/id00140_doc_elencoc14.tsv"
URL_ATL = "https://raw.githubusercontent.com/zoometh/neonet/main/inst/extdata/id00164_doc_elencoc14.tsv"
URL_ABERRANT = "https://raw.githubusercontent.com/zoometh/neonet/main/inst/extdata/c14_aberrant_dates.tsv"
OUTPUT_CSV = os.path.join(os.path.dirname(__file__), "..", "data", "neonet_curated.csv")

def download_tsv(url):
    print(f"Downloading from {url}...")
    headers = {"User-Agent": "ArchaeoPhD-EpistemicEngine/1.0"}
    req = urllib.request.Request(url, headers=headers)
    try:
        with urllib.request.urlopen(req, timeout=30) as resp:
            text = resp.read().decode("utf-8")
            reader = csv.DictReader(text.splitlines(), delimiter="\t")
            rows = list(reader)
            print(f"  Parsed {len(rows)} records.")
            return rows
    except Exception as e:
        print(f"  Failed to download {url}: {e}")
        return []

def main():
    rows_med = download_tsv(URL_MED)
    rows_atl = download_tsv(URL_ATL)
    rows_aberrant = download_tsv(URL_ABERRANT)

    all_raw = rows_med + rows_atl
    print(f"Total raw records fetched: {len(all_raw)}")

    os.makedirs(os.path.dirname(OUTPUT_CSV), exist_ok=True)
    out_fields = ["site", "country", "period", "phase_code", "labnr", "bp", "std", "material", "species", "tpq", "taq", "bib"]

    # Build aberrant lab lookup
    aberrant_labs = {r.get("LabCode", "").strip(): r for r in rows_aberrant if r.get("LabCode")}

    seen_signatures = set()
    valid_count = 0

    with open(OUTPUT_CSV, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=out_fields)
        writer.writeheader()

        for r in all_raw:
            site = r.get("SiteName", "").strip().strip('"')
            country = r.get("Country", "").strip().strip('"')
            period = r.get("Period", "").strip().strip('"')
            phase = r.get("PhaseCode", "").strip().strip('"')
            labnr = r.get("LabCode", "").strip().strip('"')
            bp = r.get("C14Age", "").strip().strip('"')
            std = r.get("C14SD", "").strip().strip('"')
            mat = r.get("Material", "").strip().strip('"')
            species = r.get("MaterialSpecies", "").strip().strip('"')
            tpq = r.get("tpq", "").strip().strip('"')
            taq = r.get("taq", "").strip().strip('"')
            bib = r.get("bib", "").strip().strip('"')

            if not labnr or not bp or bp in ("n/a", "NA", "null", ""):
                continue

            try:
                bp_f = float(bp)
                std_f = float(std) if std not in ("n/a", "NA", "") else 40.0
            except ValueError:
                continue

            sig = f"{labnr}_{site}_{phase}"
            if sig in seen_signatures:
                continue
            seen_signatures.add(sig)

            writer.writerow({
                "site": site,
                "country": country,
                "period": period,
                "phase_code": phase,
                "labnr": labnr,
                "bp": f"{bp_f:.1f}",
                "std": f"{std_f:.1f}",
                "material": mat,
                "species": species,
                "tpq": tpq,
                "taq": taq,
                "bib": bib
            })
            valid_count += 1

    print(f"Saved {valid_count} unique NeoNet determinations to {OUTPUT_CSV}")

if __name__ == "__main__":
    main()
