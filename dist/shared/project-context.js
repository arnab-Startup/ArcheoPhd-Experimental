/**
 * ArchaeoPhD — Shared Project Context Manager
 * 
 * Centralized active project resolution, titles, and local storage keys.
 */

(function () {
  'use strict';

  var APD_PROJECT = {
    getActiveId: function () {
      return localStorage.getItem('archaeo_active_project') || 'default';
    },

    getActiveProjectId: function () {
      return this.getActiveId();
    },

    setActiveId: function (id) {
      if (!id) return;
      localStorage.setItem('archaeo_active_project', id);
      window.dispatchEvent(new CustomEvent('archaeo_project_changed', { detail: { projectId: id } }));
    },

    setActiveProjectId: function (id) {
      return this.setActiveId(id);
    },

    getActiveTitle: function () {
      try {
        var projs = JSON.parse(localStorage.getItem('archaeophd_local_projects') || '[]');
        var curId = this.getActiveId();
        var found = projs.find(function(p) { return p.id === curId; });
        if (found && (found.title || found.name)) return found.title || found.name;
      } catch(e) {}
      return "Archaeological Project";
    },

    getActiveProjectTitle: function () {
      return this.getActiveTitle();
    },

    getAllProjects: function () {
      try {
        var projs = JSON.parse(localStorage.getItem('archaeophd_local_projects') || '[]');
        if (Array.isArray(projs)) {
          var hasDemo = projs.some(function(p) { return p && (p.is_demo || p.id === 'demo-southern-levant'); });
          if (!hasDemo) {
            projs.unshift({
              id: 'demo-southern-levant',
              title: 'Excavations at Tell es-Sultan (Jericho) & Southern Levant Horizons',
              name: 'Excavations at Tell es-Sultan (Jericho) & Southern Levant Horizons',
              research_period: 'Early to Middle Bronze Age (ca. 2500 - 1550 BCE)',
              research_region: 'Southern Levant (Jordan Rift Valley)',
              research_question: 'Stratigraphic synchronization of Bronze Age destruction layers and ceramic typologies.',
              keywords: ['Stratigraphy', 'Radiocarbon Dating', 'Kenyon Trench', 'Pottery Typology'],
              is_demo: true,
              is_system_demo: true,
              created_date: '2026-01-01T00:00:00.000Z'
            });
            localStorage.setItem('archaeophd_local_projects', JSON.stringify(projs));
          }
          return projs;
        }
      } catch(e) {}
      return [];
    },

    listen: function (callback) {
      if (typeof callback === 'function') {
        window.addEventListener('archaeo_project_changed', function (e) {
          callback(e.detail ? e.detail.projectId : null);
        });
      }
    }
  };

  window.APD_PROJECT = APD_PROJECT;
})();

