/**
 * ArchaeoPhD — Welcome Screen Navigation & Event Handlers
 * 
 * Manages SPA routing and click events for the welcome workstation portal.
 */

(function () {
  'use strict';

  function navigateTo(path) {
    var screen = document.getElementById('apd-welcome-screen');
    if (screen) {
      screen.classList.add('hidden');
    }
    try {
      window.history.pushState(null, '', path);
      window.dispatchEvent(new PopStateEvent('popstate'));
    } catch (e) {
      window.location.href = path;
    }
  }

  function attachEvents() {
    var btnEnter = document.getElementById('apd-w-enter-workspace');
    if (btnEnter) {
      btnEnter.onclick = function () { navigateTo('/dashboard'); };
    }

    var emblem = document.getElementById('apd-w-emblem');
    if (emblem) {
      emblem.onclick = function () { navigateTo('/dashboard'); };
    }

    var cardDemo = document.getElementById('apd-card-demo');
    if (cardDemo) {
      cardDemo.onclick = function () { navigateTo('/demo'); };
    }

    var cardStart = document.getElementById('apd-card-start');
    if (cardStart) {
      cardStart.onclick = function () { navigateTo('/projects/new'); };
    }

    var cardWorkspace = document.getElementById('apd-card-workspace');
    if (cardWorkspace) {
      cardWorkspace.onclick = function () { navigateTo('/dashboard'); };
    }

    var cardSignin = document.getElementById('apd-card-signin');
    if (cardSignin) {
      cardSignin.onclick = function () { navigateTo('/dashboard'); };
    }

    var ribbonBtns = document.querySelectorAll('.apd-w-ribbon-btn');
    ribbonBtns.forEach(function (btn) {
      btn.onclick = function () {
        var route = btn.getAttribute('data-route') || '/dashboard';
        navigateTo(route);
      };
    });
  }

  window.WelcomeNavigation = {
    navigateTo: navigateTo,
    attachEvents: attachEvents
  };
})();

