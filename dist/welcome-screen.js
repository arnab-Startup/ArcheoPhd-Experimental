/**
 * ArchaeoPhD — Welcome Screen (Compatibility Forwarder)
 * 
 * Re-routes calls to the modular architecture in dist/modules/welcome-screen/.
 */

(function () {
  'use strict';

  if (!window.archaeophdWelcome) {
    window.archaeophdWelcome = {
      show: function () {
        var screen = document.getElementById('apd-welcome-screen');
        if (screen) screen.classList.remove('hidden');
      },
      hide: function () {
        var screen = document.getElementById('apd-welcome-screen');
        if (screen) screen.classList.add('hidden');
      },
      navigate: function (path) {
        if (window.WelcomeNavigation) window.WelcomeNavigation.navigateTo(path);
        else window.location.href = path;
      }
    };
  }
})();
