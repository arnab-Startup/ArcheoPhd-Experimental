/**
 * models.js — Data structures
 * Node.js equivalent of desktop/engine/src/models.hpp
 * Same fields, same structure — so data moves cleanly between layers
 */

export function makeSite(data = {}) {
  return {
    id:                 data.id                 ?? "",
    project_id:         data.project_id         ?? "default",
    site_name:          data.site_name           ?? "",
    country:            data.country             ?? "",
    region:             data.region              ?? "",
    latitude:           data.latitude            ?? 0.0,
    longitude:          data.longitude           ?? 0.0,
    elevation:          data.elevation           ?? 0.0,
    period:             data.period              ?? "",
    site_type:          data.site_type           ?? "",
    excavation_history: data.excavation_history  ?? "",
    aliases:            data.aliases             ?? [],
    created_date:       data.created_date        ?? new Date().toISOString(),
  };
}

export function makeStratum(data = {}) {
  return {
    id:                   data.id                   ?? "",
    project_id:           data.project_id           ?? "default",
    site_id:              data.site_id              ?? "",
    stratum_name:         data.stratum_name         ?? "",
    phase:                data.phase                ?? "",
    locus_numbers:        data.locus_numbers        ?? [],
    sediment_type:        data.sediment_type        ?? "",
    chronological_bounds: data.chronological_bounds ?? "",
    date_start_bce:       data.date_start_bce       ?? 0,
    date_end_bce:         data.date_end_bce         ?? 0,
    harris_above:         data.harris_above         ?? [],
    harris_below:         data.harris_below         ?? [],
    harris_cut_by:        data.harris_cut_by        ?? [],
    created_date:         data.created_date         ?? new Date().toISOString(),
  };
}

export function makeArtifact(data = {}) {
  return {
    id:             data.id             ?? "",
    project_id:     data.project_id     ?? "default",
    artifact_name:  data.artifact_name  ?? "",
    category:       data.category       ?? "Ceramic",
    material:       data.material       ?? "",
    period:         data.period         ?? "",
    date_range:     data.date_range     ?? "",
    site_ids:       data.site_ids       ?? [],
    stratum_id:     data.stratum_id     ?? "",
    locus_findspot: data.locus_findspot ?? "",
    typology:       data.typology       ?? "",
    created_date:   data.created_date   ?? new Date().toISOString(),
  };
}

export function makeClaim(data = {}) {
  return {
    id:               data.id               ?? "",
    project_id:       data.project_id       ?? "default",
    claim_text:       data.claim_text       ?? "",
    scholar_name:     data.scholar_name     ?? "",
    source_id:        data.source_id        ?? "",
    publication_year: data.publication_year ?? "",
    page_ref:         data.page_ref         ?? "",
    chapter:          data.chapter          ?? "",
    status:           data.status           ?? "Contested",
    topic:            data.topic            ?? "",
    site_ids:         data.site_ids         ?? [],
    strata_ids:       data.strata_ids       ?? [],
    created_date:     data.created_date     ?? new Date().toISOString(),
  };
}

export function makeEvidenceLink(data = {}) {
  return {
    id:                   data.id                   ?? "",
    project_id:           data.project_id           ?? "default",
    claim_id:             data.claim_id             ?? "",
    evidence_text:        data.evidence_text        ?? "",
    evidence_type:        data.evidence_type        ?? "supporting",
    physical_entity_type: data.physical_entity_type ?? "",
    physical_entity_id:   data.physical_entity_id   ?? "",
    source_ids:           data.source_ids           ?? [],
    date_info:            data.date_info            ?? "",
    created_date:         data.created_date         ?? new Date().toISOString(),
  };
}

export function makeSource(data = {}) {
  return {
    id:           data.id           ?? "",
    project_id:   data.project_id   ?? "default",
    title:        data.title        ?? "",
    author:       data.author       ?? "",
    year:         data.year         ?? "",
    publication:  data.publication  ?? "",
    journal:      data.journal      ?? "",
    pages:        data.pages        ?? "",
    source_type:  data.source_type  ?? "Monograph",
    file_path:    data.file_path    ?? "",
    created_date: data.created_date ?? new Date().toISOString(),
  };
}

export function makeNote(data = {}) {
  return {
    id:           data.id           ?? "",
    project_id:   data.project_id   ?? "default",
    title:        data.title        ?? "",
    content:      data.content      ?? "",
    tags:         data.tags         ?? [],
    updated_date: data.updated_date ?? new Date().toISOString(),
    created_date: data.created_date ?? new Date().toISOString(),
  };
}
