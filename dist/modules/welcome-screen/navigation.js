/**
 * ArchaeoPhD — Welcome Screen Navigation & Event Handlers
 * 
 * Manages SPA routing and click events for the welcome workstation portal.
 */

(function () {
  'use strict';

  function navigateTo(path) {
    try {
      sessionStorage.setItem('archaeo_workspace_entered', 'true');
    } catch (e) {}
    var screen = document.getElementById('apd-welcome-screen');
    if (screen) {
      screen.classList.add('hidden');
    }

    // 1. Primary: Use React Router navigator directly (Zero network, Zero reload)
    if (typeof window.__appNavigate === 'function') {
      try {
        window.__appNavigate(path);
        return;
      } catch (e) {}
    }
    if (window.__historyNavigator && typeof window.__historyNavigator.push === 'function') {
      try {
        window.__historyNavigator.push(path);
        return;
      } catch (e) {}
    }

    // 2. Secondary: Find matching navigation link in React sidebar DOM and click it
    var matchingLink = document.querySelector('aside a[href="' + path + '"], a[href="' + path + '"]');
    if (matchingLink) {
      try {
        matchingLink.click();
        return;
      } catch (e) {}
    }

    // 3. Fallback: retry with __appNavigate once router is mounted
    var attempts = 0;
    var pollTimer = setInterval(function() {
      attempts++;
      if (typeof window.__appNavigate === 'function') {
        clearInterval(pollTimer);
        try { window.__appNavigate(path); } catch(e) {}
      } else if (attempts >= 10) {
        clearInterval(pollTimer);
        try {
          var link = document.querySelector('aside a[href="' + path + '"], a[href="' + path + '"]');
          if (link) link.click();
        } catch(e) {}
      }
    }, 50);
  }

  function attachEvents() {
    function openPersonalDashboard() {
      try {
        localStorage.setItem('archaeo_active_project', 'default');
      } catch (e) {}
      navigateTo('/dashboard');
    }

    var btnEnter = document.getElementById('apd-w-enter-workspace');
    if (btnEnter) {
      btnEnter.onclick = openPersonalDashboard;
    }

    var emblem = document.getElementById('apd-w-emblem');
    if (emblem) {
      emblem.onclick = openPersonalDashboard;
    }

    var cardDemo = document.getElementById('apd-card-demo');
    if (cardDemo) {
      cardDemo.onclick = function () { navigateTo('/demo'); };
    }

    var cardStart = document.getElementById('apd-card-start');
    if (cardStart) {
      cardStart.onclick = function () {
        try { localStorage.setItem('archaeo_active_project', 'default'); } catch (e) {}
        navigateTo('/projects/new');
      };
    }

    var cardWorkspace = document.getElementById('apd-card-workspace');
    if (cardWorkspace) {
      cardWorkspace.onclick = function () {
        try { localStorage.setItem('archaeo_active_project', 'default'); } catch (e) {}
        navigateTo('/projects');
      };
    }

    var cardAiResearch = document.getElementById('apd-card-ai-research');
    if (cardAiResearch) {
      cardAiResearch.onclick = function () {
        var projs = [];
        try {
          projs = JSON.parse(localStorage.getItem('archaeophd_local_projects') || '[]');
        } catch (e) {}
        var realProjs = Array.isArray(projs) ? projs.filter(function (p) { return p && p.id && p.id !== 'default' && p.id !== 'null'; }) : [];
        if (realProjs.length > 0) {
          var cur = localStorage.getItem('archaeo_active_project');
          var curValid = cur && cur !== 'default' && realProjs.some(function (p) { return p.id === cur; });
          if (!curValid) {
            localStorage.setItem('archaeo_active_project', realProjs[0].id);
          }
          navigateTo('/ai-research');
        } else {
          navigateTo('/demo');
        }
      };
    }

    var cardSignin = document.getElementById('apd-card-signin');
    if (cardSignin) {
      cardSignin.onclick = function () {
        try { localStorage.setItem('archaeo_active_project', 'default'); } catch (e) {}
        navigateTo('/projects');
      };
    }

    var ribbonBtns = document.querySelectorAll('.apd-w-ribbon-btn');
    ribbonBtns.forEach(function (btn) {
      btn.onclick = function () {
        var route = btn.getAttribute('data-route') || '/dashboard';
        var projectScopedRoutes = ['/thesis', '/ai-research', '/evidence-graph', '/sites', '/chronology'];
        var isProjectScoped = projectScopedRoutes.some(function(r) { return route.indexOf(r) !== -1; });

        if (isProjectScoped) {
          var projs = [];
          try {
            projs = JSON.parse(localStorage.getItem('archaeophd_local_projects') || '[]');
          } catch(e) {}
          var realProjs = Array.isArray(projs) ? projs.filter(function(p) { return p && p.id && p.id !== 'default' && p.id !== 'null'; }) : [];
          if (realProjs.length > 0) {
            var cur = localStorage.getItem('archaeo_active_project');
            var curValid = cur && cur !== 'default' && realProjs.some(function(p) { return p.id === cur; });
            if (!curValid) {
              localStorage.setItem('archaeo_active_project', realProjs[0].id);
            }
          } else {
            // No projects created yet: guide user to project creation first
            navigateTo('/projects/new');
            return;
          }
        }
        navigateTo(route);
      };
    });
  }

  window.WelcomeNavigation = {
    navigateTo: navigateTo,
    attachEvents: attachEvents
  };
})();

