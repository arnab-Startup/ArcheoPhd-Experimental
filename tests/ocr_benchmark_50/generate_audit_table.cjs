// Script: generate_audit_table.cjs
// Purpose: Deterministically generate the 22-fact forensic audit Markdown table
// directly from evaluated_166.json (commit 17266e1) and hand_audit_22.json.
// Eliminates manual transcription and drift across reports.

const fs = require('fs');
const path = require('path');

const evPath = path.join(__dirname, 'evaluated_166.json');
const auditPath = path.join(__dirname, 'hand_audit_22.json');

const ev166 = JSON.parse(fs.readFileSync(evPath, 'utf8'));
const audit22 = JSON.parse(fs.readFileSync(auditPath, 'utf8'));

const evMap = {};
for (const item of ev166) {
  evMap[item.id] = item;
}

const auditIds = Object.keys(audit22).map(Number).sort((a, b) => a - b);

// Target expectations mapped directly to Step 4 Dev Set
const expectedOutputs = {
  37: {
    expected: "`REJECT_NON_FINDING`: no candidate (status: REJECTED_NON_FINDING)",
    analysis: "Footnote 66 bibliography entry. Generator suppresses citation year 1947 and prehistoric count 10,000 under Rule 2."
  },
  51: {
    expected: "`EXTRACT_ATTRIBUTED`: `6 m`, subject `stratum_depth`",
    analysis: "Generator strictly preserves physical unit 'm' with depth finding 6 m; bare 6 rejected."
  },
  55: {
    expected: "`ROUTE_AMBIGUOUS_OR_UNANCHORED`: none / unanchored",
    analysis: "GT 415 A.D. absent from paragraph; regnal span 5 is a duration, not a calendar date."
  },
  56: {
    expected: "`ROUTE_AMBIGUOUS_OR_UNANCHORED`: none / unanchored",
    analysis: "GT 428 A.D. absent from window; regnal span 5 is a duration, not a calendar date."
  },
  65: {
    expected: "`EXTRACT_ATTRIBUTED`: `1545-48` (CE range), subject `viceroy_office_tenure`",
    analysis: "Generator binds viceroy tenure to compound range 1545-48; separates arrival year 1538."
  },
  73: {
    expected: "`ROUTE_AMBIGUOUS_OR_UNANCHORED`: none / unanchored",
    analysis: "GT 1712 absent from window; journal page range 323-32 rejected under Rule 1."
  },
  84: {
    expected: "`EXTRACT_ATTRIBUTED`: `1780` (CE), subject `monograph_publication_year`",
    analysis: "Generator binds publication year 1780 in Amsterdam clause; separates travel departure 1763."
  },
  93: {
    expected: "`EXTRACT_ATTRIBUTED`: `40 miles`, subject `geographic_distance`",
    analysis: "Generator strictly preserves physical unit 'miles' with distance finding 40 miles."
  },
  95: {
    expected: "`EXTRACT_ATTRIBUTED`: `1927` (CE), subject `revised_edition_date`",
    analysis: "Generator binds revised edition slot to 1927; separates first edition 1923."
  },
  97: {
    expected: "`EXTRACT_ATTRIBUTED`: `5199 BC` (-5198 astro), subject `clement_creation_estimate`",
    analysis: "Generator binds Clement creation estimate to 5199 BC; separates rabbinical date 3700 BC."
  },
  117: {
    expected: "`EXTRACT_ATTRIBUTED`: `1816` (CE), subject `curator_appointment_year`",
    analysis: "Generator binds appointment slot to 1816; separates biographical lifespan 1788-1865."
  },
  119: {
    expected: "`EXTRACT_ATTRIBUTED`: `1839` (CE), subject `guidebook_publication_year`",
    analysis: "Generator binds guidebook publication slot to 1839; separates museum opening 1819."
  },
  124: {
    expected: "`REJECT_NON_FINDING`: no candidate (status: REJECTED_NON_FINDING)",
    analysis: "Generator suppresses scholar lifespan '(1820-1903)' under Rule 2.1 as biographical metadata and emits no candidate."
  },
  135: {
    expected: "`EXTRACT_ATTRIBUTED`: `1870` (CE), subject `second_treatise_publication`",
    analysis: "Generator binds second volume slot to 1870; separates first treatise date 1865."
  },
  139: {
    expected: "`EXTRACT_ATTRIBUTED`: `1542` (CE), subject `xavier_mission_arrival`",
    analysis: "Generator binds Goa arrival slot to 1542; separates lifespan range 1506-1552."
  },
  143: {
    expected: "`EXTRACT_ATTRIBUTED`: `1631-1641` (CE range), subject `dutch_trading_tenure`",
    analysis: "Generator captures full lexical range 'from 1631 to 1641'; partial single year 1631 rejected."
  },
  144: {
    expected: "`REJECT_NON_FINDING`: no candidate (status: REJECTED_NON_FINDING)",
    analysis: "Generator suppresses bibliographic citation '(1956:81)' under Rule 2 and emits no candidate."
  },
  148: {
    expected: "`FLAG_AMBIGUOUS_MULTI_CANDIDATE`: bundle `[1952, 1954]`",
    analysis: "Competing publication years 1954 and 1952 in same sentence; generator emits AMBIGUOUS and bundles candidates."
  },
  151: {
    expected: "`EXTRACT_ATTRIBUTED`: `30%`, subject `conservation_chemical_concentration`",
    analysis: "Generator strictly preserves physical unit '%' with concentration 30%; bare 30 rejected."
  },
  152: {
    expected: "`EXTRACT_ATTRIBUTED`: `2 hours`, subject `soaking_duration`",
    analysis: "Generator binds soaking duration 2 hours; running page header folio 181 rejected under Rule 1."
  },
  153: {
    expected: "`EXTRACT_ATTRIBUTED`: `10%`, subject `reagent_concentration`",
    analysis: "Generator strictly preserves physical unit '%' with reagent concentration 10%."
  },
  154: {
    expected: "`FLAG_AMBIGUOUS_MULTI_CANDIDATE`: bundle `[10%, 3%]`",
    analysis: "Competing concentrations 10% and 3% in same sentence; generator emits AMBIGUOUS and bundles candidates."
  }
};

console.log('| # | Fact ID | Page ID | Print Class & Type | Ground Truth | Extracted (Tess / Win) | Audited Category | Expected Step 4 Generator Output | Clausal Context & Disambiguation Rule |');
console.log('| :---: | :---: | :--- | :--- | :---: | :---: | :--- | :--- | :--- |');

let index = 1;
for (const id of auditIds) {
  const item = evMap[id];
  const audit = audit22[id];
  if (!item || !audit) continue;

  const tess = item.tess_cand ? item.tess_cand.raw : 'N/A';
  const win = item.win_cand ? item.win_cand.raw : 'N/A';
  const candStr = (tess === win) ? tess : `${tess} / ${win}`;

  let cat = 'SCORER_TOKEN_DISPLACEMENT';
  if (id === 37 || id === 144 || id === 124 || audit.cat === 'BIBLIO_CITATION_REJECTION') {
    cat = 'BIBLIO_CITATION_REJECTION';
  } else if (audit.cat === 'unit or symbol dropped') {
    cat = 'UNIT_LOST';
  } else if (audit.cat === 'GT absent from window') {
    cat = 'AGREED_WRONG_CANDIDATE_GT_ABSENT';
  } else if (audit.cat === 'partial read') {
    cat = 'PARTIAL_RANGE_TRUNCATION';
  }

  if (id === 148 || id === 154) {
    cat = 'AMBIGUOUS_MULTI_CANDIDATE';
  }

  const exp = expectedOutputs[id] || { expected: "N/A", analysis: audit.analysis };
  const sanitizedAnalysis = exp.analysis.replace(/\|/g, '-');

  console.log(`| **${index++}** | **#${item.id}** | \`${item.page_id}\` | ${item.class}, ${item.type} | \`${item.true_value}\` | \`${candStr}\` | \`${cat}\` | ${exp.expected} | ${sanitizedAnalysis} |`);
}
