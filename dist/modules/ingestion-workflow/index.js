/**
 * ArchaeoPhD — Ingestion Workflow Module Entry Point
 * 
 * Orchestrates Document Ingestion, Quality Classification,
 * Verification Queue, and Manual Transcription modals.
 */

(function () {
  'use strict';

  function injectLibraryActions() {
    var ICONS = window.APD_ICONS || {};
    var rows = document.querySelectorAll('[data-source-id], .source-card, .source-item');
    rows.forEach(function (row) {
      if (row.querySelector('.apd-wf-row-actions')) return;
      var sid = row.getAttribute('data-source-id') || row.id || '';
      if (!sid) return;

      var bar = document.createElement('div');
      bar.className = 'apd-wf-row-actions';
      bar.style.cssText = 'display:flex; gap:6px; margin-top:8px;';
      bar.innerHTML = `
        <button class="apd-wf-btn apd-wf-btn-outline" style="padding:3px 8px; font-size:11px;" title="Classify Degradation">
          ${ICONS.scale || ''} Classify
        </button>
        <button class="apd-wf-btn apd-wf-btn-outline" style="padding:3px 8px; font-size:11px;" title="Manual Double-Entry">
          ${ICONS.transcribe || ''} Transcribe
        </button>
      `;

      var btns = bar.querySelectorAll('button');
      btns[0].addEventListener('click', function (e) {
        e.stopPropagation();
        if (window.ClassificationModal) window.ClassificationModal.show(sid);
      });
      btns[1].addEventListener('click', function (e) {
        e.stopPropagation();
        if (window.ManualTranscriptionModal) window.ManualTranscriptionModal.show(sid, 1);
      });

      row.appendChild(bar);
    });
  }

  var observer = new MutationObserver(function () {
    injectLibraryActions();
  });
  if (document.documentElement) {
    observer.observe(document.documentElement, { childList: true, subtree: true });
  }

  window.archaeophdIngestion = {
    openIngestion: function (initialData) {
      if (window.IngestionModal) window.IngestionModal.show(initialData);
    },
    openClassification: function (sourceId) {
      if (window.ClassificationModal) window.ClassificationModal.show(sourceId);
    },
    openVerificationQueue: function (sourceIdFilter) {
      if (window.VerificationQueueModal) window.VerificationQueueModal.show(sourceIdFilter);
    },
    openManualTranscription: function (sourceId, initialPage) {
      if (window.ManualTranscriptionModal) window.ManualTranscriptionModal.show(sourceId, initialPage);
    }
  };
})();

