/**
 * ArchaeoPhD — Archival Scan Manual Transcription
 * 
 * Implements strict Zero Pre-Fill Epistemic Invariant for Class B documents.
 * All form fields start completely blank — machine guesses are forbidden.
 */

(function () {
  'use strict';

  function showManualTranscriptionModal(sourceId, initialPage) {
    var existing = document.getElementById('apd-wf-modal-root');
    if (existing) existing.remove();

    if (window.IngestionStyles) window.IngestionStyles.inject();

    initialPage = initialPage || 1;
    var ICONS = window.APD_ICONS || {};
    var backdrop = document.createElement('div');
    backdrop.id = 'apd-wf-modal-root';
    backdrop.className = 'apd-wf-backdrop';

    backdrop.innerHTML = `
      <div class="apd-wf-modal apd-wf-modal-wide" style="height:92vh; max-width:1250px;">
        <div class="apd-wf-header">
          <div class="apd-wf-header-left">
            <div class="apd-wf-badge apd-wf-badge-amber">
              <span>${ICONS.transcribe || ''}</span> Class B Manual Double-Entry
            </div>
            <h1 class="apd-wf-title">Archival Scan Transcription</h1>
            <p class="apd-wf-subtitle" id="apd-wf-trans-subtitle">
              Strict Zero Pre-Fill Enforced — A blank field is safer than a plausible-looking wrong one.
            </p>
          </div>
          <button class="apd-wf-close-btn" id="apd-wf-close">&times;</button>
        </div>

        <div class="apd-wf-split">
          <!-- LEFT PANE: Scan Viewer -->
          <div class="apd-wf-split-left">
            <div class="apd-wf-scan-viewport" id="apd-wf-scan-viewport">
              <div class="apd-wf-scan-canvas" id="apd-wf-scan-canvas">
                <div style="font-family:var(--font-mono, monospace); font-size:10px; color:#999; margin-bottom:12px; border-bottom:1px solid #ddd; padding-bottom:4px;">
                  ARCHIVAL SCAN PREVIEW • PAGE ${initialPage}
                </div>
                <h3 style="margin-top:0; font-family:Georgia, serif; font-size:16px;">TRENCH B: STRATIGRAPHIC RECORD</h3>
                <p>
                  At a depth of 2.40 metres below surface datum, a dense deposit of burnt mudbrick and collapsed timber was encountered overlying Locus 402. The ash horizon measured approximately 40 centimetres in thickness.
                </p>
                <p>
                  Diagnostic pottery recovered within this stratum included wheel-made burnished red slip bowls and fragments of storage pithoi conforming to Middle Bronze Age IIB typological horizons.
                </p>
                <div style="margin-top:16px; border:1px dashed #bbb; padding:10px; background:#f5f0e6; font-size:12px;">
                  <em>[Physical Optical Note: Letterpress numeral "40" printed with damaged lead slug causing hairline ink bridge resembling "400". Transcription requires human verification.]</em>
                </div>
              </div>
            </div>

            <div class="apd-wf-scan-toolbar">
              <div style="display:flex; align-items:center; gap:8px;">
                <button class="apd-wf-btn apd-wf-btn-outline" style="padding:2px 8px; color:#eee;" id="apd-wf-prev-page">◀ Prev</button>
                <span>Page <strong id="apd-wf-cur-page">${initialPage}</strong> of 42</span>
                <button class="apd-wf-btn apd-wf-btn-outline" style="padding:2px 8px; color:#eee;" id="apd-wf-next-page">Next ▶</button>
              </div>
              <div style="font-size:11px;">
                Zoom: 100% | Archival Scan Layer
              </div>
            </div>
          </div>

          <!-- RIGHT PANE: Blank Transcription Form -->
          <div class="apd-wf-split-right">
            <div class="apd-wf-body" style="padding:20px 24px;">
              <!-- Zero Pre-Fill Banner -->
              <div class="apd-wf-box apd-wf-box-amber">
                <span class="apd-wf-box-icon">${ICONS.shieldCheck || ''}</span>
                <div class="apd-wf-box-content">
                  <div class="apd-wf-box-title">Zero Pre-Fill Epistemic Invariant</div>
                  In accordance with scientific rigor, machine OCR guesses are <strong>prohibited</strong> from pre-populating fields. Transcribe facts directly from the physical scan on the left.
                </div>
              </div>

              <div style="display:flex; justify-content:space-between; align-items:center;">
                <span style="font-weight:700; font-size:13px;">Extracted Archaeological Facts:</span>
                <button class="apd-wf-btn apd-wf-btn-outline" style="padding:4px 10px; font-size:12px;" id="apd-wf-add-fact-btn">
                  + Add Fact Row
                </button>
              </div>

              <!-- Fact Rows Container -->
              <div id="apd-wf-facts-container" style="display:flex; flex-direction:column; gap:10px;">
              </div>

              <div id="apd-wf-trans-status"></div>
            </div>

            <div class="apd-wf-footer" style="margin-top:auto;">
              <button class="apd-wf-btn apd-wf-btn-outline" id="apd-wf-trans-cancel">Cancel</button>
              <button class="apd-wf-btn apd-wf-btn-primary" id="apd-wf-trans-commit">
                <span>${ICONS.check || ''}</span> Commit Verified Transcription
              </button>
            </div>
          </div>
        </div>
      </div>
    `;

    document.body.appendChild(backdrop);

    function closeModal() { backdrop.remove(); }
    document.getElementById('apd-wf-close').addEventListener('click', closeModal);
    document.getElementById('apd-wf-trans-cancel').addEventListener('click', closeModal);

    var curPage = initialPage;
    var container = document.getElementById('apd-wf-facts-container');

    function addFactRow(entityName, value, type) {
      var row = document.createElement('div');
      row.className = 'apd-wf-fact-row';
      row.innerHTML = `
        <input type="text" class="apd-wf-input fact-name" placeholder="Entity (e.g. Ash Layer)" value="${entityName || ''}" />
        <input type="text" class="apd-wf-input fact-val" placeholder="Value (e.g. 40 cm thickness)" value="${value || ''}" />
        <select class="apd-wf-input fact-type" style="padding:8px 4px;">
          <option value="measurement" ${type === 'measurement' ? 'selected' : ''}>Measurement</option>
          <option value="stratum" ${type === 'stratum' ? 'selected' : ''}>Stratum</option>
          <option value="locus" ${type === 'locus' ? 'selected' : ''}>Locus</option>
          <option value="artifact" ${type === 'artifact' ? 'selected' : ''}>Artifact</option>
          <option value="date" ${type === 'date' ? 'selected' : ''}>Date / Period</option>
        </select>
        <button class="apd-wf-btn apd-wf-btn-outline" style="padding:6px; color:#e11d48;" title="Remove row">${ICONS.x || 'X'}</button>
      `;

      row.querySelector('button').addEventListener('click', function () {
        row.remove();
      });

      container.appendChild(row);
    }

    function loadTemplate(page) {
      container.innerHTML = '<div style="font-size:12px; color:hsl(var(--muted-foreground));">Loading schema...</div>';
      if (!window.APD_BRIDGE) return;
      window.APD_BRIDGE.callNative('get_transcription_template', { source_id: sourceId, page_number: page })
        .then(function (res) {
          container.innerHTML = '';
          if (res && res.fields && res.fields.length > 0) {
            addFactRow('', '', 'measurement');
            addFactRow('', '', 'stratum');
          } else {
            addFactRow('', '', 'measurement');
          }
        })
        .catch(function () {
          container.innerHTML = '';
          addFactRow('', '', 'measurement');
        });
    }

    document.getElementById('apd-wf-add-fact-btn').addEventListener('click', function () {
      addFactRow('', '', 'measurement');
    });

    document.getElementById('apd-wf-prev-page').addEventListener('click', function () {
      if (curPage > 1) {
        curPage--;
        document.getElementById('apd-wf-cur-page').textContent = curPage;
        loadTemplate(curPage);
      }
    });

    document.getElementById('apd-wf-next-page').addEventListener('click', function () {
      curPage++;
      document.getElementById('apd-wf-cur-page').textContent = curPage;
      loadTemplate(curPage);
    });

    document.getElementById('apd-wf-trans-commit').addEventListener('click', function () {
      var rows = container.querySelectorAll('.apd-wf-fact-row');
      var facts = [];
      rows.forEach(function (r) {
        var entity = r.querySelector('.fact-name').value.trim();
        var val = r.querySelector('.fact-val').value.trim();
        var type = r.querySelector('.fact-type').value;
        if (entity && val) {
          facts.push({
            entity_name: entity,
            value: val,
            type: type
          });
        }
      });

      if (facts.length === 0) {
        if (window.APD_MODAL) window.APD_MODAL.toast('Please enter at least one verified archaeological fact.', 'warning');
        return;
      }

      var statusDiv = document.getElementById('apd-wf-trans-status');
      statusDiv.innerHTML = '<div style="font-size:12px; color:hsl(var(--muted-foreground));">Committing verified claims to storage...</div>';

      if (window.APD_BRIDGE) {
        window.APD_BRIDGE.callNative('submit_manual_transcription', {
          source_id: sourceId,
          page_number: curPage,
          facts: facts
        })
        .then(function () {
          closeModal();
          if (window.APD_MODAL) window.APD_MODAL.toast('Transcription committed successfully for Page ' + curPage, 'success');
        })
        .catch(function (err) {
          statusDiv.innerHTML = '<div style="color:red; font-size:12px;">Error committing transcription: ' + (err.message || err) + '</div>';
        });
      }
    });

    loadTemplate(curPage);
  }

  window.ManualTranscriptionModal = {
    show: showManualTranscriptionModal
  };
})();

