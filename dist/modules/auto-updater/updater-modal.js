/**
 * ArchaeoPhD — Auto-Updater Modal (Compatibility Forwarder)
 * Direct 1-click update is handled seamlessly in-place by index.js.
 */
(function () {
  'use strict';
  window.APD_UPDATER_MODAL = {
    showToast: function (msg) {
      if (window.archaeophdUpdater && typeof window.archaeophdUpdater.update === 'function') {
        var toast = document.createElement('div');
        toast.className = 'apd-updater-toast visible';
        toast.textContent = msg;
        document.body.appendChild(toast);
        setTimeout(function () { if (toast.parentNode) toast.parentNode.removeChild(toast); }, 2500);
      }
    },
    showUpdateModal: function (info, cb) {
      // Direct 1-click update without modal clutter
      if (window.archaeophdUpdater && typeof window.archaeophdUpdater.update === 'function') {
        window.archaeophdUpdater.update();
      }
    }
  };
})();
