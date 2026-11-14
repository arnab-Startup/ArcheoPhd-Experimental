/**
 * ArchaeoPhD — Thesis Studio (Compatibility Forwarder)
 * 
 * Re-routes calls to the modular architecture in dist/modules/thesis-studio/.
 */

(function () {
  'use strict';

  if (!window.archaeophdThesis) {
    window.archaeophdThesis = {
      init: function () {
        if (window.ThesisStudio) window.ThesisStudio.init();
      },
      mountIfPresent: function () {
        if (window.ThesisStudio) window.ThesisStudio.mountIfPresent();
      }
    };
  }
})();
