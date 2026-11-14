/**
 * ArchaeoPhD — Shared Native Bridge Client
 * 
 * Safe, robust IPC communication with the desktop C++ backend.
 */

(function () {
  'use strict';

  var APD_BRIDGE = {
    call: async function (action, payload, projectId) {
      payload = payload || {};
      projectId = projectId || (window.APD_PROJECT ? window.APD_PROJECT.getActiveId() : 'default');

      var bridge = window.nativeBridge;
      if (!bridge || typeof bridge.call !== 'function') {
        console.warn('[BridgeClient] window.nativeBridge is not available for action:', action);
        return null;
      }

      try {
        var res = await bridge.call(action, payload, projectId);
        return res;
      } catch (err) {
        console.error('[BridgeClient] Error executing action ' + action + ':', err);
        throw err;
      }
    },

    getClaims: async function (projectId) {
      var res = await this.call('get_claims', {}, projectId);
      return Array.isArray(res) ? res : [];
    },

    getEvidence: async function (projectId) {
      var res = await this.call('get_evidence', {}, projectId);
      return Array.isArray(res) ? res : [];
    },

    getSources: async function (projectId) {
      var res = await this.call('get_sources', {}, projectId);
      return Array.isArray(res) ? res : [];
    },

    getContradictions: async function (projectId) {
      var res = await this.call('get_contradictions', {}, projectId);
      return Array.isArray(res) ? res : [];
    },

    invokeLlm: async function (promptText, projectId) {
      return await this.call('invoke_llm', { prompt: promptText }, projectId);
    }
  };

  window.APD_BRIDGE = APD_BRIDGE;
})();

