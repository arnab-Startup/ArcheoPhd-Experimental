/**
 * ArchaeoPhD — Data Root Settings Card
 * 
 * Injects workstation storage card seamlessly into the /settings view.
 * 100% offline, zero telemetry.
 */

(function () {
  'use strict';

  function injectSettingsCard(currentStatus, onRelocateClicked) {
    if (!currentStatus || !currentStatus.configured || currentStatus.is_missing) return;
    if (document.getElementById('apd-settings-card')) return;

    var isSettings = window.location.pathname.indexOf('/settings') !== -1;
    if (!isSettings) return;

    var container = document.querySelector('.space-y-6') || document.querySelector('main') || document.getElementById('root');
    if (!container) return;

    var ICONS = window.APD_ICONS || {};
    var iconHardDrive = ICONS.hardDrive || '';
    var iconFolder = ICONS.folder || '';
    var iconFolderOpen = ICONS.folderOpen || ICONS.folder || '';
    var iconWarning = ICONS.alert || ICONS.warning || '';

    var card = document.createElement('div');
    card.id = 'apd-settings-card';
    card.style.cssText = 'background: var(--apd-obsidian-card, rgba(22, 18, 16, 0.95)); border: 1px solid var(--apd-gold-border, rgba(212, 163, 89, 0.24)); border-radius: 12px; padding: 20px 24px; margin-top: 24px; box-shadow: 0 4px 20px rgba(0, 0, 0, 0.35); color: var(--apd-parchment, #f5f0e8); font-family: var(--font-body, "Inter", -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif);';

    var cloudBadge = currentStatus.cloud_service
      ? `<span style="display:inline-flex; align-items:center; gap:6px; color: var(--apd-warning, #fbbf24); background: var(--apd-warning-bg, rgba(245, 158, 11, 0.1)); border: 1px solid var(--apd-warning-border, rgba(245, 158, 11, 0.25)); padding: 4px 10px; border-radius: 9999px; font-size: 11.5px; font-weight: 500;">${iconWarning} Cloud Synced (${currentStatus.cloud_service})</span>`
      : `<span style="display:inline-flex; align-items:center; gap:6px; color: var(--apd-success, #10b981); background: var(--apd-success-bg, rgba(16, 185, 129, 0.08)); border: 1px solid var(--apd-success-border, rgba(16, 185, 129, 0.2)); padding: 4px 10px; border-radius: 9999px; font-size: 11.5px; font-weight: 500;"><span style="width:6px; height:6px; border-radius:50%; background:currentColor;"></span> 100% Offline (Local Drive)</span>`;

    card.innerHTML = `
      <div style="display:flex; justify-content:space-between; align-items:flex-start; gap:16px; margin-bottom:14px;">
        <div>
          <div style="display:flex; align-items:center; gap:8px; margin-bottom:4px;">
            <span style="color:var(--apd-gold, #d4a359); display:flex; align-items:center;">${iconHardDrive}</span>
            <h3 style="margin:0; font-family:var(--font-body, 'Inter', -apple-system, sans-serif); font-size:14px; font-weight:600; color:var(--apd-parchment-white, #ffffff); letter-spacing:-0.01em;">
              Workstation Storage Location
            </h3>
          </div>
          <p style="margin:0; font-size:13px; color:var(--apd-parchment-muted, #a89f91); line-height:1.4;">
            Local filesystem location for offline AI neural weights (4.8 GB) and research database.
          </p>
        </div>
        <div style="flex-shrink:0;">${cloudBadge}</div>
      </div>

      <div style="display:flex; align-items:center; gap:10px; background:rgba(255, 255, 255, 0.04); border:1px solid var(--apd-gold-border, rgba(212, 163, 89, 0.22)); border-radius:8px; padding:10px 14px;">
        <span style="color:var(--apd-gold, #d4a359); display:flex; align-items:center;">${iconFolder}</span>
        <code style="flex:1; font-family:var(--font-mono, monospace); font-size:13px; font-weight:500; color:var(--apd-parchment-white, #ffffff); word-break:break-all; user-select:text;">${currentStatus.data_root}</code>
        <button id="apd-settings-change-btn" type="button" class="apd-btn apd-btn-secondary" style="padding:6px 14px; font-size:12px; font-weight:500; border-radius:6px; cursor:pointer; display:flex; align-items:center; gap:6px; background:rgba(255,255,255,0.08); color:var(--apd-parchment, #f5f0e8); border:1px solid var(--apd-gold-border, rgba(212,163,89,0.3));">
          ${iconFolderOpen} Relocate...
        </button>
      </div>
    `;

    container.appendChild(card);

    var changeBtn = document.getElementById('apd-settings-change-btn');
    if (changeBtn && typeof onRelocateClicked === 'function') {
      changeBtn.addEventListener('click', onRelocateClicked);
    }
  }

  window.DataRootSettingsCard = {
    inject: injectSettingsCard
  };
})();

