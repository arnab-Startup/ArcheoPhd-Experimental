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

    setActiveId: function (id) {
      if (!id) return;
      localStorage.setItem('archaeo_active_project', id);
      window.dispatchEvent(new CustomEvent('archaeo_project_changed', { detail: { projectId: id } }));
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

    getAllProjects: function () {
      try {
        var projs = JSON.parse(localStorage.getItem('archaeophd_local_projects') || '[]');
        if (Array.isArray(projs)) return projs;
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

