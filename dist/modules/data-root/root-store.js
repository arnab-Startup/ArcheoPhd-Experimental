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
      var call = (window.APD_BRIDGE && typeof window.APD_BRIDGE.callNative === 'function')
        ? window.APD_BRIDGE.callNative
        : ((window.APD_BRIDGE && typeof window.APD_BRIDGE.call === 'function')
            ? window.APD_BRIDGE.call
            : (window.nativeBridge ? window.nativeBridge.call : null));
      if (!call) return Promise.resolve(_status);
      return call.call(window.APD_BRIDGE || window.nativeBridge, 'get_data_root_status')
        .then(function (res) {
          _status = res || {};
          RootStore._notify();
          return _status;
        }).catch(function() { return _status; });
    },

    setDataRoot: function (dataRootPath) {
      var call = (window.APD_BRIDGE && typeof window.APD_BRIDGE.callNative === 'function')
        ? window.APD_BRIDGE.callNative
        : ((window.APD_BRIDGE && typeof window.APD_BRIDGE.call === 'function')
            ? window.APD_BRIDGE.call
            : (window.nativeBridge ? window.nativeBridge.call : null));
      if (!call) return Promise.resolve(_status);
      return call.call(window.APD_BRIDGE || window.nativeBridge, 'set_data_root', { data_root: dataRootPath })
        .then(function (res) {
          return RootStore.fetchStatus();
        });
    },

    browseFolder: function (initialPath) {
      var call = (window.APD_BRIDGE && typeof window.APD_BRIDGE.callNative === 'function')
        ? window.APD_BRIDGE.callNative
        : ((window.APD_BRIDGE && typeof window.APD_BRIDGE.call === 'function')
            ? window.APD_BRIDGE.call
            : (window.nativeBridge ? window.nativeBridge.call : null));
      if (!call) return Promise.resolve(null);
      return call.call(window.APD_BRIDGE || window.nativeBridge, 'browse_folder', { initial_path: initialPath || '' });
    },

    checkCloudSync: function (path) {
      var call = (window.APD_BRIDGE && typeof window.APD_BRIDGE.callNative === 'function')
        ? window.APD_BRIDGE.callNative
        : ((window.APD_BRIDGE && typeof window.APD_BRIDGE.call === 'function')
            ? window.APD_BRIDGE.call
            : (window.nativeBridge ? window.nativeBridge.call : null));
      if (!call) return Promise.resolve(null);
      return call.call(window.APD_BRIDGE || window.nativeBridge, 'check_cloud_sync', { path: path });
    }
  };

  window.RootStore = RootStore;
})();

