/**
 * ArchaeoPhD — Auto-Updater Desktop Coordinator
 * 
 * Injects the industry-standard top-right update button into the workstation
 * header, listens for updates, and coordinates 1-click seamless upgrades.
 * Strictly zero emojis.
 */

(function () {
  'use strict';

  var updateState = {
    checked: false,
    updateAvailable: false,
    currentVersion: '1.0.0',
    latestVersion: '1.2.0',
    info: null
  };

  var ICONS = window.APD_ICONS || {
    sparkles: '<svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="m12 3-1.912 5.813a2 2 0 0 1-1.275 1.275L3 12l5.813 1.912a2 2 0 0 1 1.275 1.275L12 21l1.912-5.813a2 2 0 0 1 1.275-1.275L21 12l-5.813-1.912a2 2 0 0 1-1.275-1.275L12 3Z"/></svg>',
    download: '<svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="7 10 12 15 17 10"/><line x1="12" x2="12" y1="15" y2="3"/></svg>',
    checkCircle: '<svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 11.08V12a10 10 0 1 1-5.93-9.14"/><polyline points="22 4 12 14.01 9 11.01"/></svg>',
    refresh: '<svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 12a9 9 0 0 0-9-9 9.75 9.75 0 0 0-6.74 2.74L3 8"/><path d="M3 3v5h5"/><path d="M3 12a9 9 0 0 0 9 9 9.75 9.75 0 0 0 6.74-2.74L21 16"/><path d="M16 21h5v-5"/></svg>',
    arrowRight: '<svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="5" y1="12" x2="19" y2="12"/><polyline points="12 5 19 12 12 19"/></svg>'
  };

  function parseSemver(v) {
    if (!v) return [0, 0, 0];
    var s = String(v).replace(/^[vV]/, '').split('-')[0];
    var parts = s.split('.').map(function (n) { return parseInt(n, 10) || 0; });
    while (parts.length < 3) parts.push(0);
    return parts;
  }

  function isNewerVersion(remote, local) {
    var r = parseSemver(remote);
    var l = parseSemver(local);
    for (var i = 0; i < 3; i++) {
      if (r[i] > l[i]) return true;
      if (r[i] < l[i]) return false;
    }
    return false;
  }

  async function checkForUpdates(manualTrigger) {
    var storedVer = null;
    try {
      storedVer = localStorage.getItem('archaeophd_app_version');
    } catch (e) {}

    var currentVer = storedVer || '1.0.0';
    var bridge = window.APD_BRIDGE;
    var res = null;

    if (bridge && typeof bridge.call === 'function') {
      try {
        res = await bridge.call('check_for_updates', {});
      } catch (err) {
        console.warn('[AutoUpdater] IPC check_for_updates returned:', err);
      }
    }

    // Attempt online manifest check if available
    try {
      var controller = new AbortController();
      var timeoutId = setTimeout(function () { controller.abort(); }, 1200);
      var manifestUrl = '/downloads/manifest.json';
      var resp = await fetch(manifestUrl, { signal: controller.signal });
      clearTimeout(timeoutId);
      if (resp && resp.ok) {
        var manifest = await resp.json();
        if (manifest && manifest.version) {
          if (!res) res = {};
          res.latest_version = manifest.version;
          res.release_date = manifest.build_date || res.release_date || '2026-11-10';
          if (manifest.version_history && manifest.version_history[0]) {
            res.title = manifest.version_history[0].title || res.title;
            res.highlights = manifest.version_history[0].highlights || res.highlights;
          }
        }
      }
    } catch (e) {
      // Offline or local fallback
    }

    if (!res) {
      // Default fallback metadata for standalone browser preview
      res = {
        current_version: currentVer,
        latest_version: '1.2.0',
        release_date: '2026-11-10',
        title: 'Phase 3 Optimization & Full Engine Verification',
        highlights: [
          'Enhanced citation graph linking and evidence retrieval',
          'Faster excavation report indexing with background optimization',
          'IntCal20 radiocarbon calibration and timeline conflict detection'
        ]
      };
    }

    var targetLatest = res.latest_version || '1.2.0';
    var hasUpdate = isNewerVersion(targetLatest, currentVer);

    res.current_version = currentVer;
    res.latest_version = targetLatest;
    res.update_available = hasUpdate;

    updateState.checked = true;
    updateState.updateAvailable = hasUpdate;
    updateState.currentVersion = currentVer;
    updateState.latestVersion = targetLatest;
    updateState.info = res;

    injectHeaderButton();
    updateSettingsCard();

    if (manualTrigger) {
      if (updateState.updateAvailable) {
        if (window.APD_UPDATER_MODAL) {
          window.APD_UPDATER_MODAL.showUpdateModal(res, onUpdateFinished);
        }
      } else {
        if (window.APD_UPDATER_MODAL) {
          window.APD_UPDATER_MODAL.showToast(
            'ArchaeoPhD is up to date (v' + updateState.currentVersion + '). You have the latest version.',
            true
          );
        }
      }
    }
  }

  function onUpdateFinished(newVersion) {
    updateState.currentVersion = newVersion;
    updateState.updateAvailable = false;
    injectHeaderButton();
    updateSettingsCard();
  }

  function injectHeaderButton() {
    var existingBtn = document.getElementById('apd-header-update-btn');
    if (existingBtn) {
      updateButtonAppearance(existingBtn);
      return;
    }

    // Target the top right container:
    // 1. Workspace header: header > div.ml-auto.flex.items-center.gap-2
    // 2. Welcome screen topbar: .apd-w-topbar > div:last-child
    var headerRight = document.querySelector('header div.ml-auto') ||
                      document.querySelector('header .flex.items-center.gap-2') ||
                      document.querySelector('header') ||
                      document.querySelector('.apd-w-topbar > div:last-child') ||
                      document.querySelector('.apd-w-topbar');

    if (!headerRight) return;

    var btn = document.createElement('button');
    btn.id = 'apd-header-update-btn';
    btn.type = 'button';
    updateButtonAppearance(btn);

    btn.addEventListener('click', function (e) {
      e.preventDefault();
      e.stopPropagation();

      if (updateState.updateAvailable && updateState.info) {
        if (window.APD_UPDATER_MODAL) {
          window.APD_UPDATER_MODAL.showUpdateModal(updateState.info, onUpdateFinished);
        }
      } else {
        checkForUpdates(true);
      }
    });

    // Insert as the first item in the top-right group so it precedes notifications and profile
    if (headerRight.firstChild) {
      headerRight.insertBefore(btn, headerRight.firstChild);
    } else {
      headerRight.appendChild(btn);
    }
  }

  function updateButtonAppearance(btn) {
    if (!btn) return;
    if (updateState.updateAvailable) {
      btn.className = 'apd-header-update-btn has-update';
      btn.title = 'ArchaeoPhD v' + updateState.latestVersion + ' is available! Click to update in-place.';
      btn.innerHTML = `
        <span class="apd-update-dot"></span>
        <span style="display:inline-flex; align-items:center;">${ICONS.download}</span>
        <span>Update to v${updateState.latestVersion}</span>
      `;
    } else {
      btn.className = 'apd-header-update-btn up-to-date';
      btn.title = 'ArchaeoPhD is up to date (v' + updateState.currentVersion + '). Click to check for updates.';
      btn.innerHTML = `
        <span style="display:inline-flex; align-items:center; color:#10b981;">${ICONS.checkCircle}</span>
        <span>v${updateState.currentVersion}</span>
      `;
    }
  }

  function updateSettingsCard() {
    var isSettings = window.location.pathname.indexOf('/settings') !== -1;
    if (!isSettings) return;

    var container = document.querySelector('.space-y-6') || document.querySelector('main') || document.getElementById('root');
    if (!container) return;

    var existingCard = document.getElementById('apd-settings-update-card');
    if (existingCard) {
      existingCard.parentNode.removeChild(existingCard);
    }

    var card = document.createElement('div');
    card.id = 'apd-settings-update-card';
    card.className = 'apd-settings-update-card';

    var statusBadge = updateState.updateAvailable
      ? `<span style="display:inline-flex; align-items:center; gap:6px; color:#fce7cf; background:rgba(148,70,42,0.25); border:1px solid rgba(200,155,60,0.45); padding:4px 10px; border-radius:9999px; font-size:11.5px; font-weight:600;"><span class="apd-update-dot"></span> Update Ready (v${updateState.latestVersion})</span>`
      : `<span style="display:inline-flex; align-items:center; gap:6px; color:#10b981; background:rgba(16,185,129,0.08); border:1px solid rgba(16,185,129,0.2); padding:4px 10px; border-radius:9999px; font-size:11.5px; font-weight:500;"><span style="width:6px; height:6px; border-radius:50%; background:currentColor;"></span> Up to Date (v${updateState.currentVersion})</span>`;

    card.innerHTML = `
      <div style="display:flex; justify-content:space-between; align-items:flex-start; gap:16px; margin-bottom:14px;">
        <div>
          <div style="display:flex; align-items:center; gap:8px; margin-bottom:4px;">
            <span style="color:#d4a359; display:flex; align-items:center;">${ICONS.sparkles}</span>
            <h3 style="margin:0; font-size:14px; font-weight:600; color:#ffffff; letter-spacing:-0.01em;">
              Workstation Releases &amp; Updates
            </h3>
          </div>
          <p style="margin:0; font-size:13px; color:#a89f91; line-height:1.4;">
            In-place atomic updates with 100% Zero-Data-Loss guarantee for your research vault.
          </p>
        </div>
        <div style="flex-shrink:0;">${statusBadge}</div>
      </div>

      <div style="display:flex; flex-wrap:wrap; align-items:center; gap:10px; background:rgba(255,255,255,0.03); border:1px solid rgba(212,163,89,0.2); border-radius:8px; padding:12px 14px; margin-bottom:14px;">
        <div style="flex:1; min-width:200px;">
          <div style="font-size:12px; color:#a89f91;">Installed Version</div>
          <div style="font-size:14px; font-weight:600; color:#ffffff; font-family:monospace;">v${updateState.currentVersion}</div>
        </div>
        <div style="display:flex; align-items:center; gap:8px;">
          <button id="apd-settings-check-btn" type="button" class="apd-settings-btn apd-settings-btn-secondary">
            ${ICONS.refresh} Check for Updates
          </button>
          ${updateState.updateAvailable ? `
            <button id="apd-settings-update-now-btn" type="button" class="apd-settings-btn apd-settings-btn-primary">
              ${ICONS.download} Update to v${updateState.latestVersion}
            </button>
          ` : ''}
        </div>
      </div>

      <!-- Developer & QA Update Simulation Controls -->
      <div style="border-top:1px dashed rgba(255,255,255,0.08); padding-top:12px; display:flex; align-items:center; justify-content:space-between; flex-wrap:wrap; gap:10px;">
        <span style="font-size:11.5px; color:#78716c;">
          Interactive Testing Controls:
        </span>
        <div style="display:flex; gap:8px;">
          <button id="apd-test-simulate-release-btn" type="button" class="apd-settings-btn apd-settings-btn-secondary" style="font-size:11px; padding:4px 10px;">
            Simulate New Release (v1.3.0)
          </button>
          <button id="apd-test-reset-btn" type="button" class="apd-settings-btn apd-settings-btn-secondary" style="font-size:11px; padding:4px 10px;">
            Reset to v1.0.0
          </button>
        </div>
      </div>
    `;

    container.appendChild(card);

    var checkBtn = document.getElementById('apd-settings-check-btn');
    if (checkBtn) checkBtn.addEventListener('click', function () { checkForUpdates(true); });

    var updateNowBtn = document.getElementById('apd-settings-update-now-btn');
    if (updateNowBtn) {
      updateNowBtn.addEventListener('click', function () {
        if (window.APD_UPDATER_MODAL && updateState.info) {
          window.APD_UPDATER_MODAL.showUpdateModal(updateState.info, onUpdateFinished);
        }
      });
    }

    var simBtn = document.getElementById('apd-test-simulate-release-btn');
    if (simBtn) {
      simBtn.addEventListener('click', function () {
        window.archaeophdUpdater.simulateNewRelease('1.3.0');
      });
    }

    var resetBtn = document.getElementById('apd-test-reset-btn');
    if (resetBtn) {
      resetBtn.addEventListener('click', function () {
        window.archaeophdUpdater.resetTo('1.0.0');
      });
    }
  }

  // MutationObserver to ensure button survives React client-side route changes
  if (typeof MutationObserver !== 'undefined' && document.documentElement) {
    var observer = new MutationObserver(function () {
      if (!document.getElementById('apd-header-update-btn')) {
        injectHeaderButton();
      }
      if (window.location.pathname.indexOf('/settings') !== -1 && !document.getElementById('apd-settings-update-card')) {
        updateSettingsCard();
      }
    });

    observer.observe(document.documentElement, { childList: true, subtree: true });
  }

  window.archaeophdUpdater = {
    check: function () { checkForUpdates(true); },
    getState: function () { return Object.assign({}, updateState); },
    simulateNewRelease: function (newVer, customInfo) {
      var targetVer = newVer || '1.3.0';
      var info = customInfo || {
        current_version: updateState.currentVersion,
        latest_version: targetVer,
        release_date: '2026-11-15',
        title: 'Archaeological Field Synthesis & Multi-Layer Stratigraphy',
        highlights: [
          'Automatic site alignment across contradictory trench diaries',
          'Instant 1-click in-place update engine with atomic rollback safety',
          'IntCal20 Bayesian radiocarbon multi-phase calibration'
        ]
      };
      updateState.updateAvailable = true;
      updateState.latestVersion = targetVer;
      updateState.info = info;
      injectHeaderButton();
      updateSettingsCard();
      if (window.APD_UPDATER_MODAL) {
        window.APD_UPDATER_MODAL.showToast(
          'Simulated release ready! Click the top-right button to update to v' + targetVer + '.',
          false
        );
      }
    },
    resetTo: function (ver) {
      var target = ver || '1.0.0';
      try {
        localStorage.setItem('archaeophd_app_version', target);
      } catch (e) {}
      checkForUpdates(false);
      if (window.APD_UPDATER_MODAL) {
        window.APD_UPDATER_MODAL.showToast(
          'Workstation reset to v' + target + '. Click top-right button to test update.',
          true
        );
      }
    },
    openModal: function () {
      if (window.APD_UPDATER_MODAL && updateState.info) {
        window.APD_UPDATER_MODAL.showUpdateModal(updateState.info, onUpdateFinished);
      }
    }
  };

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', function () {
      checkForUpdates(false);
    });
  } else {
    checkForUpdates(false);
  }
})();