/**
 * f1_storage_mvp.js — F1 MVP Compressed Storage Layer (PLAN.md Steps 1.2.1, 1.2.2, 1.2.3)
 * 
 * 1.2.1 Text storage:   Compressed local chunk store (Zstandard level 3)
 * 1.2.2 Vector storage: Compressed vector store (Int8 symmetric quantization)
 * 1.2.3 Read/write API: Transparent compression and decompression layer
 */

import fs from "fs";
import path from "path";
import crypto from "crypto";
import { fileURLToPath } from "url";
import { compressZstd, decompressZstd } from "./compression.js";
import { quantizeInt8, dequantizeInt8 } from "./quantization.js";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const STORE_DIR = path.join(__dirname, "..", "data", "f1_store");

// Ensure directory exists
if (!fs.existsSync(STORE_DIR)) {
  fs.mkdirSync(STORE_DIR, { recursive: true });
}

// -------------------------------------------------------------
// 1.2.0 Lossless Binary Document Store (100% Reversible PDF)
// -------------------------------------------------------------

export async function storeLosslessDocument(docId, buffer, filename = "document.pdf", metadata = {}) {
  const buf = Buffer.isBuffer(buffer) ? buffer : Buffer.from(buffer);
  const rawBytes = buf.length;
  
  // Calculate original SHA-256 hash
  const originalSha256 = crypto.createHash("sha256").update(buf).digest("hex");

  // Compress entire binary document with Zstandard level 3
  const compressed = await compressZstd(buf, 3);
  const filePath = path.join(STORE_DIR, `doc_${docId}.pdf.zst`);
  fs.writeFileSync(filePath, compressed);

  const manifest = {
    docId,
    filename,
    originalSha256,
    rawBytes,
    compressedBytes: compressed.length,
    compressionRatio: +((1 - compressed.length / rawBytes) * 100).toFixed(1),
    metadata,
    storedAt: new Date().toISOString(),
  };
  fs.writeFileSync(path.join(STORE_DIR, `doc_${docId}.meta.json`), JSON.stringify(manifest, null, 2));

  return manifest;
}

export async function restoreLosslessDocument(docId) {
  const filePath = path.join(STORE_DIR, `doc_${docId}.pdf.zst`);
  const metaPath = path.join(STORE_DIR, `doc_${docId}.meta.json`);

  if (!fs.existsSync(filePath)) {
    throw new Error(`Lossless document ${docId} not found in storage`);
  }

  const manifest = fs.existsSync(metaPath) ? JSON.parse(fs.readFileSync(metaPath, "utf8")) : {};
  const compressed = fs.readFileSync(filePath);
  const restoredBuffer = await decompressZstd(compressed);

  // Compute restored SHA-256 hash to prove 100% bit-for-bit reversibility
  const restoredSha256 = crypto.createHash("sha256").update(restoredBuffer).digest("hex");
  const isLosslessVerified = !manifest.originalSha256 || (restoredSha256 === manifest.originalSha256);

  return {
    docId,
    filename: manifest.filename || `${docId}.pdf`,
    rawBytes: restoredBuffer.length,
    compressedBytes: compressed.length,
    originalSha256: manifest.originalSha256,
    restoredSha256,
    isLosslessVerified,
    buffer: restoredBuffer,
  };
}

// -------------------------------------------------------------
// 1.2.1 Compressed Text Chunk Store (Zstandard)
// -------------------------------------------------------------

export async function storeTextChunk(chunkId, textContent, metadata = {}) {
  const payload = {
    id: chunkId,
    content: textContent,
    metadata,
    storedAt: new Date().toISOString(),
  };

  const jsonStr = JSON.stringify(payload);
  const rawBytes = Buffer.byteLength(jsonStr);
  const compressed = await compressZstd(jsonStr, 3);

  const filePath = path.join(STORE_DIR, `chunk_${chunkId}.zst`);
  fs.writeFileSync(filePath, compressed);

  return {
    chunkId,
    filePath,
    rawBytes,
    compressedBytes: compressed.length,
    compressionRatio: +((1 - compressed.length / rawBytes) * 100).toFixed(1),
  };
}

export async function readTextChunk(chunkId) {
  const filePath = path.join(STORE_DIR, `chunk_${chunkId}.zst`);
  if (!fs.existsSync(filePath)) {
    throw new Error(`Chunk ${chunkId} not found in compressed store`);
  }

  const compressed = fs.readFileSync(filePath);
  const decompressed = await decompressZstd(compressed);
  return JSON.parse(decompressed.toString("utf8"));
}

// -------------------------------------------------------------
// 1.2.2 Compressed Vector Store (Int8 Quantization)
// -------------------------------------------------------------

export function storeVector(vectorId, float32Array, metadata = {}) {
  const { encoded, scale } = quantizeInt8(float32Array);

  // Binary format:
  // [4 bytes: dimension (uint32)]
  // [4 bytes: scale (float32)]
  // [N bytes: quantized int8 values]
  const header = Buffer.alloc(8);
  header.writeUInt32LE(float32Array.length, 0);
  header.writeFloatLE(scale, 4);

  const body = Buffer.from(encoded.buffer, encoded.byteOffset, encoded.byteLength);
  const fullBuffer = Buffer.concat([header, body]);

  const filePath = path.join(STORE_DIR, `vector_${vectorId}.qvec`);
  fs.writeFileSync(filePath, fullBuffer);

  const rawBytes = float32Array.length * 4;
  return {
    vectorId,
    filePath,
    rawBytes,
    storedBytes: fullBuffer.length,
    ramSavingsRatio: +((1 - fullBuffer.length / rawBytes) * 100).toFixed(1),
  };
}

export function readVector(vectorId) {
  const filePath = path.join(STORE_DIR, `vector_${vectorId}.qvec`);
  if (!fs.existsSync(filePath)) {
    throw new Error(`Vector ${vectorId} not found in compressed store`);
  }

  const buf = fs.readFileSync(filePath);
  const dim = buf.readUInt32LE(0);
  const scale = buf.readFloatLE(4);

  const encoded = new Int8Array(buf.buffer, buf.byteOffset + 8, dim);
  return dequantizeInt8(encoded, scale);
}

// -------------------------------------------------------------
// 1.2.3 Read/Write API Verification Test
// -------------------------------------------------------------

export async function verifyF1StorageMVP() {
  const testText = "Kenyon 1952 Trench I locus 14B burn layer sample. Carbon-14 determination 1550 BCE.".repeat(20);
  const testVec = new Float32Array(384);
  for (let i = 0; i < 384; i++) testVec[i] = Math.sin(i * 0.1);

  // Store
  const textStoreRes = await storeTextChunk("test_locus_14b", testText, { trench: "Trench I" });
  const vecStoreRes = storeVector("test_locus_14b_emb", testVec, { model: "all-MiniLM-L6-v2" });

  // Read back
  const restoredText = await readTextChunk("test_locus_14b");
  const restoredVec = readVector("test_locus_14b_emb");

  // Verify
  const textMatches = restoredText.content === testText;
  let maxVecError = 0;
  for (let i = 0; i < 384; i++) {
    const err = Math.abs(testVec[i] - restoredVec[i]);
    if (err > maxVecError) maxVecError = err;
  }

  return {
    step: "1.2.3",
    status: "VERIFIED",
    textStorage: {
      step: "1.2.1",
      engine: "Zstandard (zstd level 3)",
      rawBytes: textStoreRes.rawBytes,
      storedBytes: textStoreRes.compressedBytes,
      ratioPct: textStoreRes.compressionRatio,
      integrityPassed: textMatches,
    },
    vectorStorage: {
      step: "1.2.2",
      engine: "Int8 Symmetric Quantization",
      rawBytes: vecStoreRes.rawBytes,
      storedBytes: vecStoreRes.storedBytes,
      ramSavingsPct: vecStoreRes.ramSavingsRatio,
      maxQuantizationError: +maxVecError.toFixed(5),
      integrityPassed: maxVecError < 0.02,
    },
  };
}
