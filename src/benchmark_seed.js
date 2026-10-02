/**
 * benchmark_seed.js — Canonical Archaeological Benchmark Corpus (Tell es-Sultan)
 * Mirrors desktop/engine/src/benchmark_seed.hpp
 * Seeds sites, strata (with Harris cycle), artifacts, claims, evidence, sources, notes
 */

import * as storage from "./storage.js";

export function seedBenchmarkCorpus(projectId = "default") {
  // Sites
  storage.upsert("sites", {
    id: "site-jericho",
    project_id: projectId,
    site_name: "Tell es-Sultan (Jericho)",
    country: "Palestine",
    region: "Jordan Valley / Southern Levant",
    latitude: 31.871,
    longitude: 35.444,
    elevation: -258.0,
    period: "Middle Bronze Age IIB / Late Bronze",
    site_type: "Fortified Tell Settlement",
    excavation_history: "Garstang (1930–1936), Kenyon (1952–1958), Marchetti & Nigro (1997–present)",
    aliases: ["Jericho", "Tell es-Sultan", "Ariha", "Ancient Jericho"],
  });

  storage.upsert("sites", {
    id: "site-megiddo",
    project_id: projectId,
    site_name: "Megiddo (Tel Megiddo)",
    country: "Israel",
    region: "Jezreel Valley",
    latitude: 32.585,
    longitude: 35.184,
    period: "Middle Bronze / Late Bronze",
    site_type: "Royal City Mound",
  });

  storage.upsert("sites", {
    id: "site-hazor",
    project_id: projectId,
    site_name: "Hazor (Tel Hazor)",
    country: "Israel",
    region: "Upper Galilee",
    latitude: 33.017,
    longitude: 35.568,
    period: "Bronze Age / Iron Age",
    site_type: "Upper and Lower City",
  });

  // Strata with Stratigraphic Cycle (Unit 402 above 317 AND Unit 317 above 402)
  storage.upsert("strata", {
    id: "stratum-jericho-ivb",
    project_id: projectId,
    site_id: "site-jericho",
    stratum_name: "Stratum IV, Phase b (Destruction Horizon)",
    locus_numbers: ["Locus 402", "Trench C Unit 17"],
    sediment_type: "Dense ash layer, 0.4m depth, collapsed mudbrick",
    chronological_bounds: "1550–1400 BCE (Contested)",
    date_start_bce: -1550,
    date_end_bce: -1400,
  });

  storage.upsert("strata", {
    id: "unit-402",
    project_id: projectId,
    site_id: "site-jericho",
    stratum_name: "Stratigraphic Unit 402 (Destruction Rubble)",
    harris_above: ["unit-317"],
  });

  storage.upsert("strata", {
    id: "unit-317",
    project_id: projectId,
    site_id: "site-jericho",
    stratum_name: "Stratigraphic Unit 317 (Floor Fill Horizon)",
    harris_above: ["unit-402"], // Cycles back to 402!
  });

  // Claims
  storage.upsert("claims", {
    id: "claim-kenyon-1978",
    project_id: projectId,
    claim_text: "City IV was destroyed by the Egyptians ca. 1550 BCE at the end of MB II.",
    scholar_name: "Kathleen Kenyon",
    topic: "Chronology",
    chapter: "Chapter 4: Stratigraphic Phasing",
    status: "Verified",
    site_ids: ["site-jericho"],
  });

  storage.upsert("claims", {
    id: "claim-wood-1990",
    project_id: projectId,
    claim_text: "City IV fell ca. 1400 BCE at the end of the Late Bronze I period.",
    scholar_name: "Bryant Wood",
    topic: "Chronology",
    chapter: "Chapter 4: Stratigraphic Phasing",
    status: "Contested",
    site_ids: ["site-jericho"],
  });

  storage.upsert("claims", {
    id: "claim-simultaneous-collapse",
    project_id: projectId,
    claim_text: "Regional evidence demonstrates simultaneous collapse of urban settlements across the Southern Levant.",
    scholar_name: "Comparative Regional Survey",
    topic: "Regional Synchrony",
    chapter: "Chapter 6: Synthesis",
    status: "Contested",
    site_ids: ["site-jericho", "site-megiddo", "site-hazor"],
  });

  // Evidence
  storage.upsert("evidence", {
    id: "ev-grain-jars-domestic",
    project_id: projectId,
    claim_id: "claim-kenyon-1978",
    evidence_text: "Six large storage jars filled with charred grain found in domestic storerooms.",
    evidence_type: "supporting",
    physical_entity_id: "grain-jars-locus-14",
  });

  storage.upsert("evidence", {
    id: "ev-grain-jars-siege",
    project_id: projectId,
    claim_id: "claim-wood-1990",
    evidence_text: "Charred grain jars indicate short siege right after harvest season, contrary to standard Egyptian campaign patterns.",
    evidence_type: "contradicting",
    physical_entity_id: "grain-jars-locus-14",
  });

  return { status: "success", message: "Canonical Tell es-Sultan benchmark corpus seeded." };
}
