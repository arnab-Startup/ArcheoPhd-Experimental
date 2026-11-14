/**
 * ArchaeoPhD — Data Root Store
 * 
 * Manages native storage status and folder relocation via APD_BRIDGE.
 * 100% offline, zero cloud telemetry.
 */

(function () {
  'use strict';

  var _status = null;
  var _listeners = [];

  var RootStore = {
    getStatus: function () {
      return _status;
    },

    subscribe: function (fn) {
      if (typeof fn === 'function') _listeners.push(fn);
      return function () {
        _listeners = _listeners.filter(function (l) { return l !== fn; });
      };
    },

    _notify: function () {
      for (var i = 0; i < _listeners.length; i++) {
        try { _listeners[i](_status); } catch (e) { console.error(e); }
      }
    },

    fetchStatus: function () {
      if (!window.APD_BRIDGE) return Promise.reject(new Error('APD_BRIDGE not available'));
      return window.APD_BRIDGE.callNative('get_data_root_status')
        .then(function (res) {
          _status = res || {};
          RootStore._notify();
          return _status;
        });
    },

    setDataRoot: function (dataRootPath) {
      if (!window.APD_BRIDGE) return Promise.reject(new Error('APD_BRIDGE not available'));
      return window.APD_BRIDGE.callNative('set_data_root', { data_root: dataRootPath })
        .then(function (res) {
          return RootStore.fetchStatus();
        });
    },

    browseFolder: function (initialPath) {
      if (!window.APD_BRIDGE) return Promise.reject(new Error('APD_BRIDGE not available'));
      return window.APD_BRIDGE.callNative('browse_folder', { initial_path: initialPath || '' });
    },

    checkCloudSync: function (path) {
      if (!window.APD_BRIDGE) return Promise.reject(new Error('APD_BRIDGE not available'));
      return window.APD_BRIDGE.callNative('check_cloud_sync', { path: path });
    }
  };

  window.RootStore = RootStore;
})();

