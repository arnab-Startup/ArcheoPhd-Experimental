/**
 * ArchaeoPhD — Auto-Updater Modal & Workflow
 * 
 * Manages the user-facing 1-click update dialog, progress indicator,
 * and seamless in-place restart handoff. Strictly zero emojis.
 */

(function () {
  'use strict';

  var activeModal = null;
  var toastTimeout = null;

  var ICONS = window.APD_ICONS || {
    sparkles: '<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="m12 3-1.912 5.813a2 2 0 0 1-1.275 1.275L3 12l5.813 1.912a2 2 0 0 1 1.275 1.275L12 21l1.912-5.813a2 2 0 0 1 1.275-1.275L21 12l-5.813-1.912a2 2 0 0 1-1.275-1.275L12 3Z"/></svg>',
    shieldCheck: '<svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/><path d="m9 12 2 2 4-4"/></svg>',
    checkCircle: '<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 11.08V12a10 10 0 1 1-5.93-9.14"/><polyline points="22 4 12 14.01 9 11.01"/></svg>',
    download: '<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="7 10 12 15 17 10"/><line x1="12" x2="12" y1="15" y2="3"/></svg>',
    x: '<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="18" x2="6" y1="6" y2="18"/><line x1="6" x2="18" y1="6" y2="18"/></svg>'
  };

  function showToast(message, isSuccess) {
    if (toastTimeout) clearTimeout(toastTimeout);
    var oldToast = document.getElementById('apd-updater-toast');
    if (oldToast && oldToast.parentNode) {
      oldToast.parentNode.removeChild(oldToast);
    }

    var toast = document.createElement('div');
    toast.id = 'apd-updater-toast';
    toast.className = 'apd-updater-toast';
    var iconHtml = isSuccess
      ? `<span style="color: #34d399; display: flex;">${ICONS.checkCircle || ''}</span>`
      : `<span style="color: #e2b35a; display: flex;">${ICONS.sparkles || ''}</span>`;

    toast.innerHTML = `${iconHtml}<span>${message}</span>`;
    document.body.appendChild(toast);

    requestAnimationFrame(function () {
      toast.classList.add('visible');
    });

    toastTimeout = setTimeout(function () {
      toast.classList.remove('visible');
      setTimeout(function () {
        if (toast.parentNode) toast.parentNode.removeChild(toast);
      }, 300);
    }, 3800);
  }

  function showUpdateModal(updateInfo, onCompleted) {
    if (activeModal && activeModal.parentNode) {
      activeModal.parentNode.removeChild(activeModal);
    }

    var backdrop = document.createElement('div');
    backdrop.className = 'apd-updater-backdrop';

    var currentVer = updateInfo.current_version || '1.0.0';
    var targetVer = updateInfo.latest_version || '1.2.0';
    var rawHighlights = updateInfo.highlights || [
      'Enhanced citation graph linking and evidence retrieval',
      'Faster excavation report indexing with background optimization',
      'IntCal20 radiocarbon calibration and timeline conflict detection'
    ];

    var highlightsList = rawHighlights.slice(0, 4).map(function (item) {
      return `<li>${item}</li>`;
    }).join('');

    backdrop.innerHTML = `
      <div class="apd-updater-card" role="dialog" aria-modal="true" aria-labelledby="apd-updater-title">
        <div class="apd-updater-header">
          <div class="apd-updater-title-wrap">
            <div class="apd-updater-icon-badge">
              ${ICONS.sparkles}
            </div>
            <div>
              <h3 id="apd-updater-title" class="apd-updater-title">ArchaeoPhD Update Ready</h3>
              <p class="apd-updater-subtitle">One-click seamless workspace upgrade</p>
            </div>
          </div>
          <button id="apd-updater-close-x" class="apd-updater-close" title="Close">
            ${ICONS.x}
          </button>
        </div>

        <div class="apd-updater-body">
          <div class="apd-version-pill-row">
            <span class="apd-ver-pill current">Current: v${currentVer}</span>
            <span style="color: #a89f91; font-size: 12px;">&rarr;</span>
            <span class="apd-ver-pill new">Available: v${targetVer}</span>
          </div>

          <div class="apd-updater-reassurance">
            <span style="color: #e2b35a; flex-shrink: 0; display: flex; margin-top: 1px;">
              ${ICONS.shieldCheck}
            </span>
            <div>
              <strong>100% Zero Data Loss Guarantee:</strong> Your monographs, excavation field catalogs, thesis drafts, and notes are completely safe and will remain untouched.
            </div>
          </div>

          <div style="font-size: 12.5px; font-weight: 600; color: #ffffff; margin-bottom: 6px;">
            What is included in this update:
          </div>
          <ul class="apd-updater-highlights">
            ${highlightsList}
          </ul>

          <div id="apd-updater-progress-area" class="apd-updater-progress-wrap">
            <div class="apd-progress-track">
              <div id="apd-progress-bar" class="apd-progress-fill indeterminate"></div>
            </div>
            <div class="apd-progress-label">
              <span id="apd-progress-status">Creating pre-update safety backup...</span>
              <span id="apd-progress-percent">Please wait</span>
            </div>
          </div>
        </div>

        <div class="apd-updater-footer">
          <button id="apd-updater-cancel-btn" type="button" class="apd-btn-update-cancel">
            Remind Me Later
          </button>
          <button id="apd-updater-confirm-btn" type="button" class="apd-btn-update-primary">
            ${ICONS.download}
            <span>Update &amp; Restart</span>
          </button>
        </div>
      </div>
    `;

    document.body.appendChild(backdrop);
    activeModal = backdrop;

    requestAnimationFrame(function () {
      backdrop.classList.add('open');
    });

    function closeModal() {
      backdrop.classList.remove('open');
      setTimeout(function () {
        if (backdrop.parentNode) backdrop.parentNode.removeChild(backdrop);
        activeModal = null;
      }, 250);
    }

    var closeBtn = backdrop.querySelector('#apd-updater-close-x');
    var cancelBtn = backdrop.querySelector('#apd-updater-cancel-btn');
    var confirmBtn = backdrop.querySelector('#apd-updater-confirm-btn');
    var progressArea = backdrop.querySelector('#apd-updater-progress-area');
    var progressStatus = backdrop.querySelector('#apd-progress-status');
    var progressPercent = backdrop.querySelector('#apd-progress-percent');

    if (closeBtn) closeBtn.addEventListener('click', closeModal);
    if (cancelBtn) cancelBtn.addEventListener('click', closeModal);

    if (confirmBtn) {
      confirmBtn.addEventListener('click', function () {
        confirmBtn.disabled = true;
        cancelBtn.disabled = true;
        closeBtn.style.display = 'none';
        progressArea.style.display = 'block';

        progressStatus.textContent = 'Creating pre-update research snapshot...';
        progressPercent.textContent = 'Step 1/3';

        var bridge = window.APD_BRIDGE;
        var applyPromise;

        if (bridge && typeof bridge.call === 'function') {
          applyPromise = bridge.call('apply_update', {
            target_version: targetVer,
            update_model: false
          });
        } else {
          // Fallback simulation for browser development preview
          applyPromise = new Promise(function (resolve) {
            setTimeout(function () {
              resolve({
                success: true,
                to_version: targetVer,
                message: 'Update applied successfully in local environment.'
              });
            }, 1200);
          });
        }

        setTimeout(function () {
          progressStatus.textContent = 'Optimizing workstation cache & files...';
          progressPercent.textContent = 'Step 2/3';
        }, 600);

        applyPromise
          .then(function (result) {
            progressStatus.textContent = 'Update complete! Restarting workspace...';
            progressPercent.textContent = 'Done';
            var fill = document.getElementById('apd-progress-bar');
            if (fill) {
              fill.classList.remove('indeterminate');
              fill.style.width = '100%';
            }

            try {
              localStorage.setItem('archaeophd_app_version', targetVer);
            } catch (e) {}

            setTimeout(function () {
              closeModal();
              if (typeof onCompleted === 'function') {
                onCompleted(targetVer);
              }
              window.location.reload();
            }, 1400);
          })
          .catch(function (err) {
            console.error('[AutoUpdater] Failed to apply update:', err);
            progressStatus.textContent = 'Update paused: ' + (err.message || 'Engine busy. Retry anytime.');
            confirmBtn.disabled = false;
            cancelBtn.disabled = false;
            closeBtn.style.display = 'flex';
          });
      });
    }
  }

  window.APD_UPDATER_MODAL = {
    showToast: showToast,
    showUpdateModal: showUpdateModal
  };
})();
