/**
 * ArchaeoPhD — Thesis Studio Draft Store
 * 
 * Manages chapter outline matrix, per-chapter draft HTML, review status,
 * and text-to-HTML conversion.
 */

(function () {
  'use strict';

  var DEFAULT_CHAPTERS = [
    'Introduction',
    'Literature Review',
    'Methodology',
    'Chapter 1',
    'Chapter 2',
    'Chapter 3',
    'Discussion',
    'Conclusion'
  ];

  var DraftStore = {
    getChapters: function (projectId) {
      try {
        var saved = localStorage.getItem('archaeophd_thesis_chapters_' + projectId);
        if (saved) {
          var arr = JSON.parse(saved);
          if (Array.isArray(arr) && arr.length > 0) return arr;
        }
      } catch(e) {}
      return DEFAULT_CHAPTERS.slice();
    },

    saveChapters: function (projectId, chapters) {
      try {
        localStorage.setItem('archaeophd_thesis_chapters_' + projectId, JSON.stringify(chapters));
      } catch(e) {}
    },

    getDraft: function (projectId, chapter) {
      try {
        return localStorage.getItem('archaeophd_thesis_draft_' + projectId + '_' + encodeURIComponent(chapter)) || '';
      } catch(e) {
        return '';
      }
    },

    saveDraft: function (projectId, chapter, content) {
      try {
        localStorage.setItem('archaeophd_thesis_draft_' + projectId + '_' + encodeURIComponent(chapter), content);
      } catch(e) {}
    },

    deleteDraft: function (projectId, chapter) {
      try {
        localStorage.removeItem('archaeophd_thesis_draft_' + projectId + '_' + encodeURIComponent(chapter));
        localStorage.removeItem('archaeophd_thesis_status_' + projectId + '_' + encodeURIComponent(chapter));
      } catch(e) {}
    },

    getStatus: function (projectId, chapter) {
      try {
        return localStorage.getItem('archaeophd_thesis_status_' + projectId + '_' + encodeURIComponent(chapter)) || 'drafting';
      } catch(e) {
        return 'drafting';
      }
    },

    saveStatus: function (projectId, chapter, status) {
      try {
        localStorage.setItem('archaeophd_thesis_status_' + projectId + '_' + encodeURIComponent(chapter), status);
      } catch(e) {}
    },

    textToHtml: function (text, chapter) {
      if (!text || !text.trim()) {
        return '<h1>' + chapter + '</h1><p>Begin drafting your archaeological analysis, stratigraphy, locus calibrations, and empirical discussion here...</p>';
      }
      if (text.indexOf('<p>') !== -1 || text.indexOf('<h1>') !== -1 || text.indexOf('<div>') !== -1 || text.indexOf('<figure>') !== -1) {
        return text;
      }
      var lines = text.split('\n');
      var html = '';
      var inList = false;
      lines.forEach(function(l) {
        var trimmed = l.trim();
        if (!trimmed) {
          if (inList) { html += '</ul>'; inList = false; }
          return;
        }
        if (trimmed.startsWith('# ')) {
          if (inList) { html += '</ul>'; inList = false; }
          html += '<h1>' + trimmed.slice(2) + '</h1>';
        } else if (trimmed.startsWith('## ')) {
          if (inList) { html += '</ul>'; inList = false; }
          html += '<h2>' + trimmed.slice(3) + '</h2>';
        } else if (trimmed.startsWith('### ')) {
          if (inList) { html += '</ul>'; inList = false; }
          html += '<h3>' + trimmed.slice(4) + '</h3>';
        } else if (trimmed.startsWith('> ')) {
          if (inList) { html += '</ul>'; inList = false; }
          html += '<blockquote>' + trimmed.slice(2) + '</blockquote>';
        } else if (trimmed.startsWith('- ') || trimmed.startsWith('* ')) {
          if (!inList) { html += '<ul>'; inList = true; }
          html += '<li>' + trimmed.slice(2) + '</li>';
        } else {
          if (inList) { html += '</ul>'; inList = false; }
          html += '<p>' + trimmed + '</p>';
        }
      });
      if (inList) html += '</ul>';
      return html;
    }
  };

  window.DraftStore = DraftStore;
})();

