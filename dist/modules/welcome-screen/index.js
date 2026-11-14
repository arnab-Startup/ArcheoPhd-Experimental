/**
 * ArchaeoPhD — Welcome Screen Module Entry Point
 * 
 * Orchestrates rendering, styles, navigation, and visibility tracking
 * for the workstation welcome portal.
 */

(function () {
  'use strict';

  function ensureMounted() {
    if (document.getElementById('apd-welcome-screen')) return;
    if (window.WelcomeStyles) window.WelcomeStyles.inject();
    if (window.WelcomeTemplate) {
      var screen = window.WelcomeTemplate.create();
      document.body.appendChild(screen);
      if (window.WelcomeNavigation) window.WelcomeNavigation.attachEvents();
    }
  }

  function showWelcome() {
    ensureMounted();
    var screen = document.getElementById('apd-welcome-screen');
    if (screen) screen.classList.remove('hidden');
  }

  function hideWelcome() {
    var screen = document.getElementById('apd-welcome-screen');
    if (screen) screen.classList.add('hidden');
  }

  function checkRoute() {
    var p = window.location.pathname;
    var isWelcome = (p === '/' || p === '/welcome' || p === '/index.html' || p.endsWith('/index.html'));
    if (isWelcome) {
      showWelcome();
    } else {
      hideWelcome();
    }
  }

  function setupRouteListeners() {
    window.addEventListener('popstate', checkRoute);
    window.addEventListener('pushstate', checkRoute);

    document.addEventListener('click', function (e) {
      var logoLink = e.target.closest('a[href="/"], .apd-nav-welcome');
      if (logoLink) {
        e.preventDefault();
        window.history.pushState(null, '', '/');
        showWelcome();
      }
    });

    checkRoute();
  }

  window.archaeophdWelcome = {
    show: showWelcome,
    hide: hideWelcome,
    navigate: function (path) {
      if (window.WelcomeNavigation) window.WelcomeNavigation.navigateTo(path);
      else window.location.href = path;
    }
  };

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', function () {
      ensureMounted();
      setupRouteListeners();
    });
  } else {
    ensureMounted();
    setupRouteListeners();
  }
})();

