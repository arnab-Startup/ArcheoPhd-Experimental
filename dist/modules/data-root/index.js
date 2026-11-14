/**
 * ArchaeoPhD — Data Root Module Entry Point
 * 
 * Orchestrates native storage detection, first-launch setup wizard,
 * external SSD disconnect protection, and settings relocation.
 */

(function () {
  'use strict';

  function refreshStatus() {
    if (!window.RootStore) return;
    window.RootStore.fetchStatus()
      .then(function (status) {
        var rootSpan = document.getElementById('apd-sb-dataroot');
        if (rootSpan && status.data_root) {
          rootSpan.textContent = 'Data Root: ' + status.data_root;
        }

        if (!status.configured) {
          if (window.RootDialog) {
            window.RootDialog.showFirstLaunchModal(status, refreshStatus);
          }
        } else if (status.is_missing) {
          if (window.RootDialog) {
            window.RootDialog.showMissingRootModal(status, refreshStatus);
          }
        } else {
          tryInjectSettings();
        }
      })
      .catch(function (err) {
        console.warn('[ArchaeoPhD DataRoot] Could not query data root status:', err);
      });
  }

  function tryInjectSettings() {
    if (!window.RootStore || !window.DataRootSettingsCard) return;
    var status = window.RootStore.getStatus();
    if (status) {
      window.DataRootSettingsCard.inject(status, function () {
        if (window.RootDialog) {
          window.RootDialog.showFirstLaunchModal(status, refreshStatus);
        }
      });
    }
  }

  // SPA navigation observer for /settings
  var observer = new MutationObserver(function () {
    tryInjectSettings();
  });
  if (document.documentElement) {
    observer.observe(document.documentElement, { childList: true, subtree: true });
  }

  window.archaeophdDataRoot = {
    openPicker: function () {
      var status = window.RootStore ? window.RootStore.getStatus() : null;
      if (status && window.RootDialog) {
        window.RootDialog.showFirstLaunchModal(status, refreshStatus);
      } else {
        refreshStatus();
      }
    },
    getStatus: function () {
      return window.RootStore ? window.RootStore.getStatus() : null;
    },
    refresh: refreshStatus
  };

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', refreshStatus);
  } else {
    refreshStatus();
  }
})();

