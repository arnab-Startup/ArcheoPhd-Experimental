import zipfile
import csv
import xml.etree.ElementTree as ET
import os

INPUT_XLSX = "experimental/data/palaeolithic_europe.xlsx"
OUTPUT_CSV = "experimental/data/palaeolithic_europe.csv"

def main():
    if not os.path.exists(INPUT_XLSX):
        print(f"File not found: {INPUT_XLSX}")
        return

    print(f"Reading {INPUT_XLSX}...")
    with zipfile.ZipFile(INPUT_XLSX) as z:
        sst_root = ET.fromstring(z.read("xl/sharedStrings.xml"))
        shared_strings = []
        ns = "{http://schemas.openxmlformats.org/spreadsheetml/2006/main}"
        for si in sst_root.findall(f"{ns}si"):
            t = si.find(f"{ns}t")
            if t is not None:
                shared_strings.append(t.text or "")
            else:
                shared_strings.append("".join(x.text or "" for x in si.findall(f".//{ns}t")))

        sheet2 = z.open("xl/worksheets/sheet2.xml")

        out_fields = [
            "site", "layer_id", "country", "period", "method",
            "age", "std_dev", "labnr", "material", "reliability", "bib"
        ]

        # Column letter mappings from row 7:
        # B: sitename, C: layer_id, F: country, I: cul stage, K: method,
        # L: age, M: "+ - σ", O: labref, P: sample, Q: reliability, S: bibliogr_ref
        col_map = {
            "B": "site",
            "C": "layer_id",
            "F": "country",
            "I": "period",
            "K": "method",
            "L": "age",
            "M": "std_dev",
            "O": "labref",
            "P": "material",
            "Q": "reliability",
            "S": "bib"
        }

        row_idx = 0
        records = []
        for event, elem in ET.iterparse(sheet2):
            if elem.tag.endswith("row"):
                row_idx += 1
                if row_idx >= 8: # Data rows
                    row_data = {}
                    for c in elem.findall(f"{ns}c"):
                        ref = c.attrib.get("r", "")
                        col = "".join(filter(str.isalpha, ref))
                        if col in col_map:
                            t = c.attrib.get("t")
                            v = c.find(f"{ns}v")
                            val = v.text if v is not None else ""
                            if t == "s" and val.isdigit():
                                val = shared_strings[int(val)]
                            row_data[col_map[col]] = val.strip()

                    site = row_data.get("site", "")
                    age_str = row_data.get("age", "")
                    labnr = row_data.get("labref", "")

                    if site and age_str:
                        records.append({
                            "site": site,
                            "layer_id": row_data.get("layer_id", ""),
                            "country": row_data.get("country", ""),
                            "period": row_data.get("period", ""),
                            "method": row_data.get("method", ""),
                            "age": age_str,
                            "std_dev": row_data.get("std_dev", "0"),
                            "labnr": labnr,
                            "material": row_data.get("material", ""),
                            "reliability": row_data.get("reliability", ""),
                            "bib": row_data.get("bib", "")
                        })
                elem.clear()

        print(f"Parsed {len(records)} valid records.")
        with open(OUTPUT_CSV, "w", newline="", encoding="utf-8") as f:
            writer = csv.DictWriter(f, fieldnames=out_fields)
            writer.writeheader()
            for r in records:
                writer.writerow(r)
        print(f"Wrote {len(records)} records to {OUTPUT_CSV}")

if __name__ == "__main__":
    main()
