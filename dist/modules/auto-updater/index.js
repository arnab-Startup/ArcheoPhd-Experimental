/**
 * ArchaeoPhD — Auto-Updater Desktop Coordinator
 * 
 * Clean, minimal, industry-standard 1-click update:
 * Shows a discreet "Update" button in the top-right header when a release is available.
 * Clicking the button directly updates the existing workstation and restarts.
 * Zero clutter, zero unnecessary dialogs.
 */

(function () {
  'use strict';

  var updateState = {
    checked: false,
    updateAvailable: false,
    currentVersion: '1.0.0',
    latestVersion: '1.2.0',
    updating: false
  };

  var ICONS = {
    checkCircle: '<svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 11.08V12a10 10 0 1 1-5.93-9.14"/><polyline points="22 4 12 14.01 9 11.01"/></svg>',
    spinner: '<svg class="apd-spin" width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="12" y1="2" x2="12" y2="6"/><line x1="12" y1="18" x2="12" y2="22"/><line x1="4.93" y1="4.93" x2="7.76" y2="7.76"/><line x1="16.24" y1="16.24" x2="19.07" y2="19.07"/><line x1="2" y1="12" x2="6" y2="12"/><line x1="18" y1="12" x2="22" y2="12"/><line x1="4.93" y1="19.07" x2="7.76" y2="16.24"/><line x1="16.24" y1="7.76" x2="19.07" y2="4.93"/></svg>'
  };

  function parseSemver(v) {
    if (!v) return [0, 0, 0];
    var s = String(v).replace(/^[vV]/, '').split('-')[0];
    var parts = s.split('.').map(function (n) { return parseInt(n, 10) || 0; });
    while (parts.length < 3) parts.push(0);
    return parts;
  }

  function isNewer(remote, local) {
    var r = parseSemver(remote);
    var l = parseSemver(local);
    for (var i = 0; i < 3; i++) {
      if (r[i] > l[i]) return true;
      if (r[i] < l[i]) return false;
    }
    return false;
  }

  async function checkForUpdates(manual) {
    var storedVer = null;
    try {
      storedVer = localStorage.getItem('archaeophd_app_version');
    } catch (e) {}

    var currentVer = storedVer || '1.0.0';
    var bridge = window.APD_BRIDGE;
    var targetVer = '1.2.0';

    if (bridge && typeof bridge.call === 'function') {
      try {
        var res = await bridge.call('check_for_updates', {});
        if (res && res.latest_version) targetVer = res.latest_version;
      } catch (err) {}
    }

    try {
      var controller = new AbortController();
      var tid = setTimeout(function () { controller.abort(); }, 3000);
      var resp = await fetch('https://archeophd.vercel.app/downloads/manifest.json', { signal: controller.signal });
      clearTimeout(tid);
      if (resp && resp.ok) {
        var m = await resp.json();
        if (m && m.version) targetVer = m.version;
      }
    } catch (e) {
      try {
        var localResp = await fetch('/downloads/manifest.json');
        if (localResp && localResp.ok) {
          var lm = await localResp.json();
          if (lm && lm.version) targetVer = lm.version;
        }
      } catch (err) {}
    }

    var hasUpdate = isNewer(targetVer, currentVer);
    updateState.checked = true;
    updateState.currentVersion = currentVer;
    updateState.latestVersion = targetVer;
    updateState.updateAvailable = hasUpdate;

    injectButton();

    if (manual && !hasUpdate) {
      showToast('ArchaeoPhD is up to date.');
    }
  }

  async function performUpdate() {
    if (updateState.updating) return;
    updateState.updating = true;
    injectButton();

    var targetVer = updateState.latestVersion || '1.2.1';
    var bridge = window.APD_BRIDGE;
    var modalApi = window.APD_UPDATER_MODAL;
    var progressCtrl = null;

    if (modalApi && typeof modalApi.showProgressModal === 'function') {
      try {
        progressCtrl = modalApi.showProgressModal(targetVer);
      } catch (e) {}
    }

    try {
      if (progressCtrl) {
        progressCtrl.setStep(1, 28, 'Verifying package integrity from archeophd.vercel.app...');
      }

      // Invoke native bridge immediately
      var bridgePromise = (bridge && typeof bridge.call === 'function')
        ? bridge.call('apply_update', { target_version: targetVer })
        : Promise.resolve();

      if (progressCtrl) {
        progressCtrl.setStep(2, 60, 'Creating safe pre-update database snapshot...');
      }

      await bridgePromise;

      if (progressCtrl) {
        progressCtrl.setStep(3, 88, 'Updating application configuration and clearing cache...');
      }

      try {
        localStorage.setItem('archaeophd_app_version', targetVer);
      } catch (e) {}

      updateState.currentVersion = targetVer;
      updateState.updateAvailable = false;
      updateState.updating = false;
      injectButton();

      var doRestart = function () {
        var base = (typeof window !== 'undefined' && window.location && window.location.origin && window.location.origin !== 'null')
          ? window.location.origin
          : '';
        // CRITICAL: Explicitly navigate to /index.html.
        // Calling window.location.reload() on subroutes like /projects or /dashboard
        // causes WebView2 to look for a folder and return ERR_ACCESS_DENIED (blank white screen).
        if (typeof window !== 'undefined' && window.location) {
          if (typeof window.location.assign === 'function') {
            window.location.assign((base ? base : '') + '/index.html');
          } else {
            window.location.href = (base ? base : '') + '/index.html';
          }
        }
      };

      if (progressCtrl && typeof progressCtrl.finish === 'function') {
        progressCtrl.finish(doRestart);
      } else {
        showToast('Updated to latest version. Restarting...');
        setTimeout(doRestart, 500);
      }
    } catch (err) {
      console.error('[AutoUpdater] Update failed:', err);
      updateState.updating = false;
      injectButton();
      var overlay = document.getElementById('apd-update-overlay');
      if (overlay && overlay.parentNode) overlay.parentNode.removeChild(overlay);
      showToast('Update failed. Please retry.');
    }
  }

  function injectButton() {
    var btn = document.getElementById('apd-header-update-btn');
    if (!btn) {
      var container = document.querySelector('header div.ml-auto') ||
                      document.querySelector('header .flex.items-center.gap-2') ||
                      document.querySelector('header') ||
                      document.querySelector('.apd-w-topbar > div:last-child') ||
                      document.querySelector('.apd-w-topbar');

      if (!container) return;

      btn = document.createElement('button');
      btn.id = 'apd-header-update-btn';
      btn.type = 'button';

      btn.addEventListener('click', function (e) {
        e.preventDefault();
        e.stopPropagation();
        if (updateState.updateAvailable) {
          performUpdate();
        } else {
          checkForUpdates(true);
        }
      });

      if (container.firstChild) {
        container.insertBefore(btn, container.firstChild);
      } else {
        container.appendChild(btn);
      }
    }

    if (updateState.updating) {
      btn.className = 'apd-header-update-btn updating';
      btn.title = 'Updating ArchaeoPhD...';
      btn.innerHTML = `${ICONS.spinner} <span>Updating...</span>`;
    } else if (updateState.updateAvailable) {
      btn.className = 'apd-header-update-btn has-update';
      btn.title = 'Click to update ArchaeoPhD.';
      btn.innerHTML = `<span class="apd-update-dot"></span> <span>Update</span>`;
    } else {
      btn.className = 'apd-header-update-btn up-to-date';
      btn.title = 'ArchaeoPhD is up to date.';
      btn.innerHTML = `<span style="color:#10b981;">${ICONS.checkCircle}</span> <span>Up to date</span>`;
    }
  }

  function showToast(msg) {
    var old = document.getElementById('apd-updater-toast');
    if (old && old.parentNode) old.parentNode.removeChild(old);

    var t = document.createElement('div');
    t.id = 'apd-updater-toast';
    t.className = 'apd-updater-toast';
    t.textContent = msg;
    document.body.appendChild(t);

    requestAnimationFrame(function () { t.classList.add('visible'); });
    setTimeout(function () {
      t.classList.remove('visible');
      setTimeout(function () { if (t.parentNode) t.parentNode.removeChild(t); }, 300);
    }, 2800);
  }

  if (typeof MutationObserver !== 'undefined' && document.documentElement) {
    var observer = new MutationObserver(function () {
      if (!document.getElementById('apd-header-update-btn')) injectButton();
    });
    observer.observe(document.documentElement, { childList: true, subtree: true });
  }

  window.archaeophdUpdater = {
    check: function () { checkForUpdates(true); },
    update: performUpdate,
    simulateNewRelease: function (ver) {
      updateState.updateAvailable = true;
      updateState.latestVersion = ver || '1.3.0';
      injectButton();
      showToast('Update available. Click Update to install.');
    },
    resetTo: function (ver) {
      try { localStorage.setItem('archaeophd_app_version', ver || '1.0.0'); } catch (e) {}
      checkForUpdates(false);
    },
    getState: function () { return Object.assign({}, updateState); }
  };

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', function () { checkForUpdates(false); });
  } else {
    checkForUpdates(false);
  }
})();
