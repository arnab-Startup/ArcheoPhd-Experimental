/**
 * storage.js — Local file-based storage
 * Node.js equivalent of desktop/engine/src/storage.hpp
 * Stores all data as compressed JSON in the data/ directory
 */

import { readFileSync, writeFileSync, existsSync, mkdirSync } from "fs";
import { join, dirname } from "path";
import { fileURLToPath } from "url";
import { compressGzip, decompressGzip } from "./compression.js";

const __dirname = dirname(fileURLToPath(import.meta.url));
const DATA_DIR  = join(__dirname, "..", "data");

// Ensure data directory exists
mkdirSync(DATA_DIR, { recursive: true });

// ── File helpers ─────────────────────────────

function statePath() { return join(DATA_DIR, "relational_state.json"); }

function loadState() {
  if (!existsSync(statePath())) return {};
  try {
    const raw = readFileSync(statePath());
    return JSON.parse(raw.toString("utf8"));
  } catch { return {}; }
}

function saveState(state) {
  writeFileSync(statePath(), JSON.stringify(state, null, 2), "utf8");
}

// ── In-memory store (same tables as C++ engine) ──

let _state = loadState();

function getTable(name) { return _state[name] ?? {}; }
function setTable(name, map) { _state[name] = map; saveState(_state); }

// ── Public API (mirrors NativeStorage methods) ──

export function getAll(table, project_id = "") {
  const map = getTable(table);
  const rows = Object.values(map);
  return project_id ? rows.filter(r => r.project_id === project_id) : rows;
}

export function getOne(table, id) {
  return getTable(table)[id] ?? null;
}

export function upsert(table, item) {
  const map = getTable(table);
  map[item.id] = item;
  setTable(table, map);
  return item;
}

export function remove(table, id) {
  const map = getTable(table);
  delete map[id];
  setTable(table, map);
}

export function count(table) {
  return Object.keys(getTable(table)).length;
}
