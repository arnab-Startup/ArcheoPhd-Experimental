/**
 * ArchaeoPhD — Verification Queue & Discrepancy Resolver
 * 
 * Manages human verification of dual-engine OCR discrepancies,
 * optical scan crops, and epistemic anti-anchoring lockouts.
 */

(function () {
  'use strict';

  function showVerificationQueueModal(sourceIdFilter) {
    var existing = document.getElementById('apd-wf-modal-root');
    if (existing) existing.remove();

    if (window.IngestionStyles) window.IngestionStyles.inject();

    var ICONS = window.APD_ICONS || {};
    var backdrop = document.createElement('div');
    backdrop.id = 'apd-wf-modal-root';
    backdrop.className = 'apd-wf-backdrop';

    backdrop.innerHTML = `
      <div class="apd-wf-modal apd-wf-modal-wide">
        <div class="apd-wf-header">
          <div class="apd-wf-header-left">
            <div class="apd-wf-badge apd-wf-badge-terracotta">
              <span>${ICONS.clipboard || ''}</span> Human Discrepancy Queue
            </div>
            <h1 class="apd-wf-title">Anti-Anchoring Discrepancy Verification</h1>
            <p class="apd-wf-subtitle">
              Dual-engine discrepancies (Windows OCR vs. VLM consensus) require verified visual resolution against optical physical crops.
            </p>
          </div>
          <button class="apd-wf-close-btn" id="apd-wf-close">&times;</button>
        </div>

        <div class="apd-wf-body" id="apd-wf-vqueue-body">
          <div style="font-size:13px; color:hsl(var(--muted-foreground));">
            Loading verification items from native storage...
          </div>
        </div>

        <div class="apd-wf-footer">
          <button class="apd-wf-btn apd-wf-btn-outline" id="apd-wf-refresh-queue">
            ${ICONS.refresh || ''} Refresh Queue
          </button>
          <button class="apd-wf-btn apd-wf-btn-secondary" id="apd-wf-close-footer">
            Close
          </button>
        </div>
      </div>
    `;

    document.body.appendChild(backdrop);

    function closeModal() { backdrop.remove(); }
    document.getElementById('apd-wf-close').addEventListener('click', closeModal);
    document.getElementById('apd-wf-close-footer').addEventListener('click', closeModal);
    document.getElementById('apd-wf-refresh-queue').addEventListener('click', loadQueue);

    function loadQueue() {
      var body = document.getElementById('apd-wf-vqueue-body');
      body.innerHTML = '<div style="padding:20px; font-size:13px; color:hsl(var(--muted-foreground));">Loading queue...</div>';

      if (!window.APD_BRIDGE) return;
      window.APD_BRIDGE.callNative('get_verification_queue', { source_id: sourceIdFilter || '' })
        .then(function (items) {
          if (!items || items.length === 0) {
            body.innerHTML = `
              <div class="apd-wf-box apd-wf-box-emerald">
                <span class="apd-wf-box-icon">${ICONS.check || ''}</span>
                <div class="apd-wf-box-content">
                  <div class="apd-wf-box-title">Verification Queue Empty</div>
                  No unresolved discrepancies in the knowledge graph. All quantitative claims are grounded with consensus or manual transcription.
                </div>
              </div>
            `;
            return;
          }

          body.innerHTML = '';
          items.forEach(function (item) {
            var card = renderVerificationCard(item);
            body.appendChild(card);
          });
        })
        .catch(function (err) {
          body.innerHTML = `
            <div class="apd-wf-box apd-wf-box-crimson">
              <span class="apd-wf-box-icon">${ICONS.alert || ICONS.warning || ''}</span>
              <div class="apd-wf-box-content">
                Failed to load verification queue: ${err.message || err}
              </div>
            </div>
          `;
        });
    }

    function renderVerificationCard(item) {
      var card = document.createElement('div');
      card.className = 'apd-wf-vcard';
      card.id = 'vitem-card-' + item.id;

      var hasCrop = Boolean(item.crop_image_path && item.crop_image_path.trim().length > 0);
      var isResolved = item.status && item.status !== 'PENDING';

      var cropHtml = '';
      var cleanCropSrc = item.crop_image_path || '';
      if (cleanCropSrc.startsWith('/') || cleanCropSrc.startsWith('\\')) {
        cleanCropSrc = cleanCropSrc.replace(/^[/\\]+/, '');
      }
      if (hasCrop) {
        cropHtml = `
          <div class="apd-wf-crop-box" id="crop-box-${item.id}">
            <div class="apd-wf-crop-label">${ICONS.search || ''} Physical Scan Crop (Direct Evidence)</div>
            <img src="${cleanCropSrc}" class="apd-wf-crop-img" id="crop-img-${item.id}" alt="Optical Scan Crop" />
          </div>
        `;
      } else {
        cropHtml = `
          <div class="apd-wf-box apd-wf-box-amber" id="crop-box-${item.id}">
            <span class="apd-wf-box-icon">${ICONS.lock || ''}</span>
            <div class="apd-wf-box-content">
              <div class="apd-wf-box-title">ANTI-ANCHORING LOCKOUT ACTIVE</div>
              Visual optical crop is missing for this discrepancy item. In accordance with ArchaeoPhD epistemic invariants, candidate resolution is strictly locked without visual evidence to prevent cognitive anchoring. You may only reject this item.
            </div>
          </div>
        `;
      }

      card.innerHTML = `
        <div style="display:flex; justify-content:space-between; align-items:flex-start;">
          <div>
            <div style="display:flex; align-items:center; gap:8px;">
              <span class="apd-wf-badge ${isResolved ? 'apd-wf-badge-emerald' : 'apd-wf-badge-amber'}" style="margin:0;">
                ${item.status}
              </span>
              <span style="font-size:12px; font-family:var(--font-mono, monospace); color:hsl(var(--muted-foreground));">
                ID: ${item.id} | Page ${item.page_number || 1}
              </span>
            </div>
            <div style="font-size:13px; font-weight:600; margin-top:4px;">
              Context: <em>"${item.context_text || 'No context snippet available'}"</em>
            </div>
          </div>
          <div style="font-size:11.5px; color:hsl(var(--muted-foreground)); text-align:right;">
            Field: <code>${item.field_type}</code>
          </div>
        </div>

        ${cropHtml}

        <div class="apd-wf-candidates-grid">
          <div class="apd-wf-candidate-box">
            <div class="apd-wf-candidate-title">Candidate A (Windows OCR)</div>
            <div class="apd-wf-candidate-val">${item.candidate_a || '—'}</div>
          </div>
          <div class="apd-wf-candidate-box">
            <div class="apd-wf-candidate-title">Candidate B (VLM Consensus)</div>
            <div class="apd-wf-candidate-val">${item.candidate_b || '—'}</div>
          </div>
        </div>

        <div style="font-size:12px; color:hsl(var(--muted-foreground));">
          <strong>Audit Note:</strong> ${item.audit_note || 'Discrepancy detected across dual extraction engines.'}
        </div>

        <!-- Inline Manual Override Input -->
        <div id="override-box-${item.id}" style="display:none; margin-top:6px; flex-direction:column; gap:6px;">
          <label style="font-size:12px; font-weight:600;">Manual Ground Truth (Type exact reading from crop above):</label>
          <div class="apd-wf-input-row">
            <input type="text" id="override-input-${item.id}" class="apd-wf-input" placeholder="Enter true value..." />
            <button class="apd-wf-btn apd-wf-btn-primary" id="override-submit-${item.id}">Submit Override</button>
          </div>
        </div>

        <div style="display:flex; justify-content:flex-end; gap:8px; margin-top:4px;">
          <button class="apd-wf-btn apd-wf-btn-danger" id="btn-reject-${item.id}" ${isResolved ? 'disabled' : ''}>
            ${ICONS.x || ''} Reject Item
          </button>
          <button class="apd-wf-btn apd-wf-btn-secondary" id="btn-override-${item.id}" ${!hasCrop || isResolved ? 'disabled' : ''}>
            ${ICONS.transcribe || ''} Manual Override...
          </button>
          <button class="apd-wf-btn apd-wf-btn-outline" id="btn-choose-b-${item.id}" ${!hasCrop || isResolved ? 'disabled' : ''}>
            Select Candidate B
          </button>
          <button class="apd-wf-btn apd-wf-btn-primary" id="btn-choose-a-${item.id}" ${!hasCrop || isResolved ? 'disabled' : ''}>
            Select Candidate A
          </button>
        </div>
      `;

      function resolveItem(decision, val) {
        if (!window.APD_BRIDGE) return;
        window.APD_BRIDGE.callNative('resolve_verification_candidate', {
          item_id: item.id,
          decision: decision,
          resolved_value: val
        })
        .then(function () {
          loadQueue();
          if (window.APD_MODAL) window.APD_MODAL.toast('Item resolved as ' + decision, 'success');
        })
        .catch(function (err) {
          if (window.APD_MODAL) window.APD_MODAL.toast('Resolution failed: ' + (err.message || err), 'error');
        });
      }

      var btnA = card.querySelector('#btn-choose-a-' + item.id);
      var btnB = card.querySelector('#btn-choose-b-' + item.id);
      var btnRej = card.querySelector('#btn-reject-' + item.id);
      var btnOver = card.querySelector('#btn-override-' + item.id);
      var boxOver = card.querySelector('#override-box-' + item.id);
      var inpOver = card.querySelector('#override-input-' + item.id);
      var subOver = card.querySelector('#override-submit-' + item.id);

      if (btnA) btnA.onclick = function () { resolveItem('ACCEPTED_A', item.candidate_a); };
      if (btnB) btnB.onclick = function () { resolveItem('ACCEPTED_B', item.candidate_b); };
      if (btnRej) btnRej.onclick = function () { resolveItem('REJECTED', ''); };
      if (btnOver && boxOver) {
        btnOver.onclick = function () {
          boxOver.style.display = boxOver.style.display === 'none' ? 'flex' : 'none';
        };
      }
      if (subOver && inpOver) {
        subOver.onclick = function () {
          var v = inpOver.value.trim();
          if (!v) {
            if (window.APD_MODAL) window.APD_MODAL.toast('Please enter the manual ground truth reading.', 'warning');
            return;
          }
          resolveItem('OVERRIDDEN', v);
        };
      }

      return card;
    }

    loadQueue();
  }

  window.VerificationQueueModal = {
    show: showVerificationQueueModal
  };
})();

