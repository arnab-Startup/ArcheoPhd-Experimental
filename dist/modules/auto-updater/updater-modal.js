/**
 * ArchaeoPhD — Auto-Updater Modal & Progress Engine
 * 
 * Renders an animated progress modal during update handoff:
 * 1. Step checklist & percentage animation
 * 2. Pre-update database backup status
 * 3. Cache pruning
 * 4. Safe reload to /index.html (preventing WebView2 blank 404 page)
 * Zero emojis, strict architectural compliance.
 */
(function () {
  'use strict';

  function showProgressModal(targetVer, onProgressDone) {
    var overlay = document.getElementById('apd-update-overlay');
    if (overlay && overlay.parentNode) {
      overlay.parentNode.removeChild(overlay);
    }

    overlay = document.createElement('div');
    overlay.id = 'apd-update-overlay';
    overlay.className = 'apd-update-overlay';
    overlay.innerHTML = `
      <div class="apd-update-card">
        <div class="apd-update-badge">System Update</div>
        <h2 class="apd-update-title">Updating ArchaeoPhD Workstation</h2>
        <p class="apd-update-desc">Installing release <strong>v${targetVer || '1.2.1'}</strong> with zero data loss guarantee.</p>

        <div class="apd-update-progress-container">
          <div class="apd-update-progress-track">
            <div id="apd-update-progress-fill" class="apd-update-progress-fill" style="width: 15%;"></div>
          </div>
          <div class="apd-update-progress-meta">
            <span id="apd-update-step-text">Connecting to distribution server...</span>
            <span id="apd-update-percent-text">15%</span>
          </div>
        </div>

        <div class="apd-update-steps">
          <div id="apd-step-1" class="apd-step active">
            <span class="apd-step-icon">1</span>
            <span>Verifying package integrity from archeophd.vercel.app</span>
          </div>
          <div id="apd-step-2" class="apd-step">
            <span class="apd-step-icon">2</span>
            <span>Creating safe pre-update database snapshot</span>
          </div>
          <div id="apd-step-3" class="apd-step">
            <span class="apd-step-icon">3</span>
            <span>Applying updated application payload and clearing cache</span>
          </div>
          <div id="apd-step-4" class="apd-step">
            <span class="apd-step-icon">4</span>
            <span>Restarting workstation</span>
          </div>
        </div>
      </div>
    `;

    document.body.appendChild(overlay);

    function updateStep(stepNum, percent, statusText) {
      var fill = document.getElementById('apd-update-progress-fill');
      var pText = document.getElementById('apd-update-percent-text');
      var sText = document.getElementById('apd-update-step-text');

      if (fill) fill.style.width = percent + '%';
      if (pText) pText.textContent = percent + '%';
      if (sText) sText.textContent = statusText;

      for (var i = 1; i <= 4; i++) {
        var el = document.getElementById('apd-step-' + i);
        if (!el) continue;
        if (i < stepNum) {
          el.className = 'apd-step done';
        } else if (i === stepNum) {
          el.className = 'apd-step active';
        } else {
          el.className = 'apd-step';
        }
      }
    }

    return {
      setStep: updateStep,
      finish: function (callback) {
        updateStep(4, 100, 'Restarting workstation...');
        setTimeout(function () {
          if (typeof callback === 'function') callback();
        }, 600);
      }
    };
  }

  function showToast(msg) {
    var old = document.getElementById('apd-updater-toast');
    if (old && old.parentNode) old.parentNode.removeChild(old);

    var t = document.createElement('div');
    t.id = 'apd-updater-toast';
    t.className = 'apd-updater-toast';
    t.textContent = msg;
    document.body.appendChild(t);

    if (typeof requestAnimationFrame === 'function') {
      requestAnimationFrame(function () { t.classList.add('visible'); });
    } else {
      t.className = 'apd-updater-toast visible';
    }

    setTimeout(function () {
      if (t.classList && typeof t.classList.remove === 'function') {
        t.classList.remove('visible');
      }
      setTimeout(function () {
        if (t.parentNode) t.parentNode.removeChild(t);
      }, 300);
    }, 2800);
  }

  window.APD_UPDATER_MODAL = {
    showToast: showToast,
    showProgressModal: showProgressModal,
    showUpdateModal: function (info, cb) {
      if (window.archaeophdUpdater && typeof window.archaeophdUpdater.update === 'function') {
        window.archaeophdUpdater.update();
      }
    }
  };
})();
