/**
 * ArchaeoPhD — Native Data Root Manager (Compatibility Forwarder)
 * 
 * Re-routes calls to the modular architecture in dist/modules/data-root/.
 */

(function () {
  'use strict';

  if (!window.archaeophdDataRoot) {
    window.archaeophdDataRoot = {
      openPicker: function () {
        if (window.RootStore && window.RootDialog) {
          var status = window.RootStore.getStatus();
          window.RootDialog.showFirstLaunchModal(status, function () {
            if (window.archaeophdDataRoot.refresh) window.archaeophdDataRoot.refresh();
          });
        }
      },
      getStatus: function () {
        return window.RootStore ? window.RootStore.getStatus() : null;
      },
      refresh: function () {
        if (window.RootStore) return window.RootStore.fetchStatus();
      }
    };
  }
})();
