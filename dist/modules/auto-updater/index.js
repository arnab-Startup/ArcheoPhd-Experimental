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
    refresh: '<svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 12a9 9 0 0 0-9-9 9.75 9.75 0 0 0-6.74 2.74L3 8"/><path d="M3 3v5h5"/><path d="M3 12a9 9 0 0 0 9 9 9.75 9.75 0 0 0 6.74-2.74L21 16"/><path d="M16 21h5v-5"/></svg>'
  };

  async function checkForUpdates(manualTrigger) {
    var storedVer = null;
    try {
      storedVer = localStorage.getItem('archaeophd_app_version');
    } catch (e) {}

    var bridge = window.APD_BRIDGE;
    var res = null;

    if (bridge && typeof bridge.call === 'function') {
      try {
        res = await bridge.call('check_for_updates', {});
      } catch (err) {
        console.warn('[AutoUpdater] IPC check_for_updates query returned:', err);
      }
    }

    if (!res) {
      // Fallback metadata for standalone browser preview
      res = {
        current_version: storedVer || '1.0.0',
        update_available: storedVer !== '1.2.0',
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

    // If local storage indicates already updated to latest version
    if (storedVer && storedVer === res.latest_version) {
      res.update_available = false;
      res.current_version = storedVer;
    }

    updateState.checked = true;
    updateState.updateAvailable = !!res.update_available;
    updateState.currentVersion = res.current_version || '1.0.0';
    updateState.latestVersion = res.latest_version || '1.2.0';
    updateState.info = res;

    injectHeaderButton();

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
  }

  function injectHeaderButton() {
    var existingBtn = document.getElementById('apd-header-update-btn');
    if (existingBtn) {
      updateButtonAppearance(existingBtn);
      return;
    }

    // Target the top right header container:
    // React layout renders: header > div.ml-auto.flex.items-center.gap-2
    var headerRight = document.querySelector('header div.ml-auto') ||
                      document.querySelector('header .flex.items-center.gap-2') ||
                      document.querySelector('header');

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
    if (updateState.updateAvailable) {
      btn.className = 'apd-header-update-btn has-update';
      btn.title = 'New version ready! Click to update ArchaeoPhD.';
      btn.innerHTML = `
        <span class="apd-update-dot"></span>
        <span style="display:inline-flex; align-items:center;">${ICONS.download}</span>
        <span>Update Available</span>
      `;
    } else {
      btn.className = 'apd-header-update-btn up-to-date';
      btn.title = 'ArchaeoPhD is up to date. Click to check again.';
      btn.innerHTML = `
        <span style="display:inline-flex; align-items:center; color:#10b981;">${ICONS.checkCircle}</span>
        <span>v${updateState.currentVersion}</span>
      `;
    }
  }

  // MutationObserver to ensure button survives React client-side route changes
  var observer = new MutationObserver(function () {
    if (!document.getElementById('apd-header-update-btn')) {
      injectHeaderButton();
    }
  });

  if (document.documentElement) {
    observer.observe(document.documentElement, { childList: true, subtree: true });
  }

  window.archaeophdUpdater = {
    check: function () { checkForUpdates(true); },
    getState: function () { return Object.assign({}, updateState); }
  };

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', function () {
      checkForUpdates(false);
    });
  } else {
    checkForUpdates(false);
  }
})();
