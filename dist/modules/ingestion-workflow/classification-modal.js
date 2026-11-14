/**
 * ArchaeoPhD — Document Classification Modal
 * 
 * Configures Degradation Tier (Class A Modern Offset vs Class B Degraded Letterpress),
 * inspection guardrails, and retroactive claim purge protection.
 */

(function () {
  'use strict';

  function showClassificationModal(sourceId) {
    var existing = document.getElementById('apd-wf-modal-root');
    if (existing) existing.remove();

    if (window.IngestionStyles) window.IngestionStyles.inject();

    var ICONS = window.APD_ICONS || {};
    var backdrop = document.createElement('div');
    backdrop.id = 'apd-wf-modal-root';
    backdrop.className = 'apd-wf-backdrop';

    backdrop.innerHTML = `
      <div class="apd-wf-modal">
        <div class="apd-wf-header">
          <div class="apd-wf-header-left">
            <div class="apd-wf-badge apd-wf-badge-terracotta">
              <span>${ICONS.scale || ''}</span> Source Quality Gating
            </div>
            <h1 class="apd-wf-title">Document Degradation Classification</h1>
            <p class="apd-wf-subtitle" id="apd-wf-class-source-subtitle">
              Loading source details...
            </p>
          </div>
          <button class="apd-wf-close-btn" id="apd-wf-close">&times;</button>
        </div>

        <div class="apd-wf-body" id="apd-wf-class-body">
          <div style="font-size:13px; line-height:1.5;">
            Archaeological document extraction requires strict epistemic categorization. Machine OCR models systematically hallucinate when processing pre-digital letterpress, irregular lead slug types, and aged paper.
          </div>

          <div class="apd-wf-tier-grid">
            <!-- CLASS A CARD -->
            <div class="apd-wf-tier-card" id="apd-wf-tier-a">
              <div class="apd-wf-tier-name">
                <span>Class A: Modern Offset</span>
                <span class="apd-wf-badge apd-wf-badge-emerald" style="margin:0;">High Precision</span>
              </div>
              <p class="apd-wf-tier-desc">
                Clean typography, post-1970 digital or offset lithography, uniform baseline, zero ink bleed or broken lead type.
              </p>
              <div style="font-size:11.5px; font-weight:600; color:hsl(158 64% 35%);">
                Eligible for automated dual-engine extraction.
              </div>
            </div>

            <!-- CLASS B CARD -->
            <div class="apd-wf-tier-card selected" id="apd-wf-tier-b">
              <div class="apd-wf-tier-name">
                <span>Class B: Degraded Letterpress</span>
                <span class="apd-wf-badge apd-wf-badge-amber" style="margin:0;">Hard Gated</span>
              </div>
              <p class="apd-wf-tier-desc">
                Pre-1970 monographs, handset lead type, irregular baselines, uneven ink, bleed-through, historical journals.
              </p>
              <div style="font-size:11.5px; font-weight:600; color:hsl(32 85% 35%);">
                Automated claims strictly locked. Manual transcription only.
              </div>
            </div>
          </div>

          <!-- Class A Confirmation Guardrail -->
          <div id="apd-wf-class-a-guard" style="display:none; background:hsl(38 92% 50% / 0.08); border:1px solid hsl(38 92% 50% / 0.3); border-radius:var(--radius, 8px); padding:14px;">
            <label style="display:flex; align-items:flex-start; gap:10px; cursor:pointer; font-size:12.5px; line-height:1.5;">
              <input type="checkbox" id="apd-wf-clean-offset-check" style="margin-top:3px;" />
              <span>
                <strong>Physical Print Inspection Required:</strong> I have physically inspected the scan and confirm it is clean modern offset printing with uniform typography and zero letterpress distortions or bleed-through.
              </span>
            </label>
          </div>

          <!-- Retroactive Purge Warning for Existing Class A -->
          <div id="apd-wf-reflag-purge-warning" style="display:none;" class="apd-wf-box apd-wf-box-crimson">
            <span class="apd-wf-box-icon">${ICONS.alert || ICONS.warning || ''}</span>
            <div class="apd-wf-box-content">
              <div class="apd-wf-box-title">Retroactive Claim Purge Protection</div>
              Reflagging this document to <strong>Class B</strong> will retroactively purge all automated consensus claims associated with it. Only human manual double-entry claims will be preserved.
            </div>
          </div>

          <div id="apd-wf-class-status"></div>
        </div>

        <div class="apd-wf-footer">
          <button class="apd-wf-btn apd-wf-btn-outline" id="apd-wf-cancel">Cancel</button>
          <button class="apd-wf-btn apd-wf-btn-danger" id="apd-wf-reflag-btn" style="display:none;">
            <span>${ICONS.refresh || ''}</span> Reflag to Class B & Purge Automated Claims
          </button>
          <button class="apd-wf-btn apd-wf-btn-primary" id="apd-wf-save-class-btn">
            Save Classification
          </button>
        </div>
      </div>
    `;

    document.body.appendChild(backdrop);

    function closeModal() { backdrop.remove(); }
    document.getElementById('apd-wf-close').addEventListener('click', closeModal);
    document.getElementById('apd-wf-cancel').addEventListener('click', closeModal);

    var currentSource = null;
    var selectedTier = 'CLASS_B';

    if (window.APD_BRIDGE) {
      window.APD_BRIDGE.callNative('get_sources')
        .then(function (sources) {
          for (var i = 0; sources && i < sources.length; i++) {
            if (sources[i].id === sourceId) {
              currentSource = sources[i];
              break;
            }
          }
          if (currentSource) {
            document.getElementById('apd-wf-class-source-subtitle').textContent = 
              (currentSource.title || 'Untitled') + ' (' + (currentSource.author || 'Unknown') + ', ' + (currentSource.year || 'n.d.') + ')';
            
            if (currentSource.degradation_class === 'CLASS_A') {
              selectTier('CLASS_A');
              document.getElementById('apd-wf-reflag-btn').style.display = 'inline-flex';
              document.getElementById('apd-wf-reflag-purge-warning').style.display = 'flex';
            } else {
              selectTier('CLASS_B');
            }
          }
        })
        .catch(function () {
          document.getElementById('apd-wf-class-source-subtitle').textContent = 'Source ID: ' + sourceId;
        });
    }

    function selectTier(tier) {
      selectedTier = tier;
      var cardA = document.getElementById('apd-wf-tier-a');
      var cardB = document.getElementById('apd-wf-tier-b');
      var guardA = document.getElementById('apd-wf-class-a-guard');
      var saveBtn = document.getElementById('apd-wf-save-class-btn');

      if (tier === 'CLASS_A') {
        cardA.classList.add('selected');
        cardB.classList.remove('selected');
        guardA.style.display = 'block';
        var chk = document.getElementById('apd-wf-clean-offset-check');
        saveBtn.disabled = !chk.checked;
      } else {
        cardB.classList.add('selected');
        cardA.classList.remove('selected');
        guardA.style.display = 'none';
        saveBtn.disabled = false;
      }
    }

    document.getElementById('apd-wf-tier-a').onclick = function () { selectTier('CLASS_A'); };
    document.getElementById('apd-wf-tier-b').onclick = function () { selectTier('CLASS_B'); };

    var offsetCheck = document.getElementById('apd-wf-clean-offset-check');
    if (offsetCheck) {
      offsetCheck.onchange = function () {
        if (selectedTier === 'CLASS_A') {
          document.getElementById('apd-wf-save-class-btn').disabled = !offsetCheck.checked;
        }
      };
    }

    // Save classification
    document.getElementById('apd-wf-save-class-btn').onclick = function () {
      if (selectedTier === 'CLASS_A' && offsetCheck && !offsetCheck.checked) {
        if (window.APD_MODAL) window.APD_MODAL.toast('Please check the physical print confirmation box for Class A.', 'warning');
        return;
      }

      if (window.APD_BRIDGE) {
        window.APD_BRIDGE.callNative('set_source_degradation_class', {
          source_id: sourceId,
          degradation_class: selectedTier
        })
        .then(function () {
          closeModal();
          if (window.APD_MODAL) window.APD_MODAL.toast('Document degradation tier set to ' + selectedTier, 'success');
        })
        .catch(function (err) {
          if (window.APD_MODAL) window.APD_MODAL.toast('Failed to save tier: ' + (err.message || err), 'error');
        });
      }
    };

    // Reflag button
    document.getElementById('apd-wf-reflag-btn').onclick = function () {
      if (confirm('Reflag to Class B and purge automated consensus claims?')) {
        if (window.APD_BRIDGE) {
          window.APD_BRIDGE.callNative('reflag_source_degradation_class', {
            source_id: sourceId,
            new_degradation_class: 'CLASS_B'
          })
          .then(function () {
            closeModal();
            if (window.APD_MODAL) window.APD_MODAL.toast('Source downgraded to Class B. Automated claims purged.', 'warning');
          })
          .catch(function (err) {
            if (window.APD_MODAL) window.APD_MODAL.toast('Reflagging error: ' + (err.message || err), 'error');
          });
        }
      }
    };
  }

  window.ClassificationModal = {
    show: showClassificationModal
  };
})();

