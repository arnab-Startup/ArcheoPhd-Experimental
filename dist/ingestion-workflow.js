/**
 * ArchaeoPhD — Ingestion Workflow (Compatibility Forwarder)
 * 
 * Re-routes calls to the modular architecture in dist/modules/ingestion-workflow/.
 */

(function () {
  'use strict';

  if (!window.archaeophdIngestion) {
    window.archaeophdIngestion = {
      openIngestion: function (data) {
        if (window.IngestionModal) window.IngestionModal.show(data);
      },
      openClassification: function (id) {
        if (window.ClassificationModal) window.ClassificationModal.show(id);
      },
      openVerificationQueue: function (filter) {
        if (window.VerificationQueueModal) window.VerificationQueueModal.show(filter);
      },
      openManualTranscription: function (id, page) {
        if (window.ManualTranscriptionModal) window.ManualTranscriptionModal.show(id, page);
      }
    };
  }
})();
