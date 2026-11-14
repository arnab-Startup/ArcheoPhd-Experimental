/**
 * ArchaeoPhD — Gated Document Ingestion Modal
 * 
 * Handles PDF monograph onboarding, lossless archiving, SHA-256
 * cryptographic hashing, and automated classification routing.
 */

(function () {
  'use strict';

  function formatBytes(bytes) {
    if (!bytes || bytes === 0) return '0 B';
    var k = 1024;
    var sizes = ['B', 'KB', 'MB', 'GB'];
    var i = Math.floor(Math.log(bytes) / Math.log(k));
    return (bytes / Math.pow(k, i)).toFixed(1) + ' ' + sizes[i];
  }

  function showIngestionModal(initialData) {
    var existing = document.getElementById('apd-wf-modal-root');
    if (existing) existing.remove();

    if (window.IngestionStyles) window.IngestionStyles.inject();

    initialData = initialData || {};
    var ICONS = window.APD_ICONS || {};
    var backdrop = document.createElement('div');
    backdrop.id = 'apd-wf-modal-root';
    backdrop.className = 'apd-wf-backdrop';

    backdrop.innerHTML = `
      <div class="apd-wf-modal">
        <div class="apd-wf-header">
          <div class="apd-wf-header-left">
            <div class="apd-wf-badge apd-wf-badge-terracotta">
              <span>${ICONS.landmark || ''}</span> ArchaeoPhD Ingestion Engine
            </div>
            <h1 class="apd-wf-title">Gated Document Ingestion</h1>
            <p class="apd-wf-subtitle">
              Onboard excavation reports, monographs, or field journals into the grounded knowledge graph.
            </p>
          </div>
          <button class="apd-wf-close-btn" id="apd-wf-close">&times;</button>
        </div>

        <div class="apd-wf-body" id="apd-wf-ingest-body">
          <div class="apd-wf-box apd-wf-box-amber">
            <span class="apd-wf-box-icon">${ICONS.alert || ICONS.warning || ''}</span>
            <div class="apd-wf-box-content">
              <div class="apd-wf-box-title">Default-Safe Gating Active</div>
              All newly ingested literature defaults strictly to <strong>Class B (Degraded Letterpress)</strong> under an amber <code>UNVERIFIED_ROUGH_SCAN</code> status. Automated entity extraction is locked until physical offset quality is confirmed.
            </div>
          </div>

          <div class="apd-wf-field">
            <label class="apd-wf-label">PDF Document File Path</label>
            <div class="apd-wf-input-row">
              <input type="text" id="apd-wf-file-path" class="apd-wf-input apd-wf-input-mono" 
                placeholder="D:\\Research\\Excavation_Report_Vol1.pdf" 
                value="${initialData.file_path || ''}" />
              <button class="apd-wf-btn apd-wf-btn-secondary" id="apd-wf-browse-btn">
                ${ICONS.folder || ''} Browse...
              </button>
            </div>
          </div>

          <div class="apd-wf-field">
            <label class="apd-wf-label">Document Title</label>
            <input type="text" id="apd-wf-title" class="apd-wf-input" 
              placeholder="Excavation Survey Report: Volume I" 
              value="${initialData.title || ''}" />
          </div>

          <div style="display:grid; grid-template-columns: 2fr 1fr; gap:12px;">
            <div class="apd-wf-field">
              <label class="apd-wf-label">Primary Author / Excavator</label>
              <input type="text" id="apd-wf-author" class="apd-wf-input" 
                placeholder="Primary Author / Researcher" 
                value="${initialData.author || ''}" />
            </div>
            <div class="apd-wf-field">
              <label class="apd-wf-label">Publication Year</label>
              <input type="text" id="apd-wf-year" class="apd-wf-input" 
                placeholder="1960" 
                value="${initialData.year || ''}" />
            </div>
          </div>

          <div id="apd-wf-ingest-status-area"></div>
        </div>

        <div class="apd-wf-footer" id="apd-wf-ingest-footer">
          <button class="apd-wf-btn apd-wf-btn-outline" id="apd-wf-cancel">Cancel</button>
          <button class="apd-wf-btn apd-wf-btn-primary" id="apd-wf-submit-ingest">
            <span>${ICONS.download || ''}</span> Ingest & Gate Document
          </button>
        </div>
      </div>
    `;

    document.body.appendChild(backdrop);

    function closeModal() { backdrop.remove(); }
    document.getElementById('apd-wf-close').addEventListener('click', closeModal);
    document.getElementById('apd-wf-cancel').addEventListener('click', closeModal);

    // Native File Browser via IPC
    document.getElementById('apd-wf-browse-btn').addEventListener('click', function () {
      if (window.APD_BRIDGE) {
        window.APD_BRIDGE.callNative('browse_file', { initial_dir: '' })
          .then(function (res) {
            if (res && !res.cancelled && res.path) {
              document.getElementById('apd-wf-file-path').value = res.path;
              var titleInput = document.getElementById('apd-wf-title');
              if (!titleInput.value) {
                var base = res.path.split(/[\\/]/).pop().replace(/\.pdf$/i, '').replace(/[_-]/g, ' ');
                titleInput.value = base;
              }
            }
          })
          .catch(function (err) {
            console.warn('Native browse_file failed:', err);
          });
      }
    });

    // Ingestion Submission
    document.getElementById('apd-wf-submit-ingest').addEventListener('click', function () {
      var filePath = document.getElementById('apd-wf-file-path').value.trim();
      var title = document.getElementById('apd-wf-title').value.trim();
      var author = document.getElementById('apd-wf-author').value.trim();
      var year = document.getElementById('apd-wf-year').value.trim();

      if (!filePath) {
        if (window.APD_MODAL) window.APD_MODAL.toast('Please enter or browse for a PDF document path.', 'warning');
        return;
      }
      if (!title) {
        if (window.APD_MODAL) window.APD_MODAL.toast('Please provide a document title.', 'warning');
        return;
      }

      var btn = document.getElementById('apd-wf-submit-ingest');
      btn.disabled = true;
      btn.innerHTML = '<span>' + (ICONS.refresh || '') + '</span> Ingesting & Calculating SHA-256...';

      var statusArea = document.getElementById('apd-wf-ingest-status-area');
      statusArea.innerHTML = `
        <div class="apd-wf-box apd-wf-box-muted">
          <span class="apd-wf-box-icon">${ICONS.refresh || ''}</span>
          <div class="apd-wf-box-content">
            Verifying cryptographic integrity, applying lossless gzip compression, and registering source in native database...
          </div>
        </div>
      `;

      if (window.APD_BRIDGE) {
        window.APD_BRIDGE.callNative('ingest_document', {
          file_path: filePath,
          title: title,
          author: author,
          year: year
        })
        .then(function (result) {
          btn.style.display = 'none';
          var ratio = result.original_bytes > 0 
            ? ((1 - (result.compressed_bytes / result.original_bytes)) * 100).toFixed(1) + '%' 
            : '0%';

          statusArea.innerHTML = `
            <div class="apd-wf-box apd-wf-box-emerald">
              <span class="apd-wf-box-icon">${ICONS.check || ''}</span>
              <div class="apd-wf-box-content">
                <div class="apd-wf-box-title">Document Successfully Ingested & Losslessly Archived</div>
                Source ID: <code>${result.source_id}</code>
              </div>
            </div>

            <div style="background:hsl(var(--muted, 38 20% 94%) / 0.5); border:1px solid hsl(var(--border, 36 16% 87%)); border-radius:var(--radius, 8px); padding:14px 16px; font-size:12px; display:flex; flex-direction:column; gap:8px;">
              <div style="display:flex; justify-content:space-between; align-items:center;">
                <span style="font-weight:600;">${ICONS.shieldCheck || ''} SHA-256 Lossless Hash:</span>
                <button class="apd-wf-btn apd-wf-btn-outline" style="padding:2px 8px; font-size:11px;" id="apd-wf-copy-hash">${ICONS.copy || ''} Copy</button>
              </div>
              <div style="font-family:var(--font-mono, monospace); word-break:break-all; color:hsl(var(--primary, 18 58% 28%)); background:hsl(var(--card, 40 33% 99%)); padding:8px 10px; border-radius:4px; border:1px solid hsl(var(--border, 36 16% 87%));">
                ${result.sha256}
              </div>

              <div style="display:grid; grid-template-columns: 1fr 1fr 1fr; gap:8px; margin-top:4px;">
                <div><strong>Original:</strong> ${formatBytes(result.original_bytes)}</div>
                <div><strong>Compressed:</strong> ${formatBytes(result.compressed_bytes)}</div>
                <div><strong>Compression:</strong> ${ratio} saved</div>
              </div>
            </div>
          `;

          var copyBtn = document.getElementById('apd-wf-copy-hash');
          if (copyBtn) {
            copyBtn.onclick = function () {
              navigator.clipboard.writeText(result.sha256);
              copyBtn.innerHTML = (ICONS.check || '') + ' Copied!';
            };
          }

          var footer = document.getElementById('apd-wf-ingest-footer');
          footer.innerHTML = `
            <button class="apd-wf-btn apd-wf-btn-outline" id="apd-wf-finish-btn">Close</button>
            <button class="apd-wf-btn apd-wf-btn-primary" id="apd-wf-classify-next-btn">
              Configure Degradation Quality &rarr;
            </button>
          `;
          document.getElementById('apd-wf-finish-btn').onclick = closeModal;
          document.getElementById('apd-wf-classify-next-btn').onclick = function () {
            closeModal();
            if (window.ClassificationModal) {
              window.ClassificationModal.show(result.source_id);
            }
          };
        })
        .catch(function (err) {
          btn.disabled = false;
          btn.innerHTML = '<span>' + (ICONS.download || '') + '</span> Ingest & Gate Document';
          if (window.APD_MODAL) {
            window.APD_MODAL.toast('Ingestion failed: ' + (err.message || err), 'error');
          }
        });
      }
    });
  }

  window.IngestionModal = {
    show: showIngestionModal
  };
})();

