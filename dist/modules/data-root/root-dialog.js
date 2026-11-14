/**
 * ArchaeoPhD — Data Root Dialog Renderer
 * 
 * Renders First Launch Setup and Missing Storage modals using
 * standard Lucide SVG icons from window.APD_ICONS.
 */

(function () {
  'use strict';

  function injectStyles() {
    if (document.getElementById('apd-dataroot-styles')) return;
    var style = document.createElement('style');
    style.id = 'apd-dataroot-styles';
    style.textContent = `
      .apd-dr-backdrop {
        position: fixed;
        inset: 0;
        background: var(--apd-backdrop, rgba(10, 8, 7, 0.78));
        backdrop-filter: blur(10px);
        -webkit-backdrop-filter: blur(10px);
        z-index: 999999;
        display: flex;
        align-items: center;
        justify-content: center;
        padding: 24px;
        font-family: var(--font-body, "Inter", -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif);
        color: var(--apd-parchment, #f5f0e8);
        animation: apdFadeIn 0.22s cubic-bezier(0.16, 1, 0.3, 1);
      }
      @keyframes apdFadeIn {
        from { opacity: 0; transform: scale(0.96) translateY(4px); }
        to { opacity: 1; transform: scale(1) translateY(0); }
      }
      .apd-dr-modal {
        background: var(--apd-obsidian-card, #171311);
        border: 1px solid var(--apd-gold-border, rgba(212, 163, 89, 0.28));
        border-radius: 16px;
        width: 100%;
        max-width: 580px;
        box-shadow: 0 25px 65px -10px rgba(0, 0, 0, 0.85), 0 0 0 1px var(--apd-gold-glow, rgba(212, 163, 89, 0.15));
        overflow: hidden;
        display: flex;
        flex-direction: column;
        color: var(--apd-parchment, #f5f0e8);
      }
      .apd-dr-header {
        padding: 24px 28px 18px;
        border-bottom: 1px solid var(--apd-gold-border-muted, rgba(212, 163, 89, 0.18));
        background: var(--apd-obsidian-header, #201a17);
      }
      .apd-dr-badge {
        display: inline-flex;
        align-items: center;
        gap: 6px;
        font-size: 11px;
        font-weight: 700;
        text-transform: uppercase;
        letter-spacing: 0.08em;
        color: var(--apd-gold, #d4a359);
        background: var(--apd-gold-bg, rgba(212, 163, 89, 0.12));
        border: 1px solid var(--apd-gold-border, rgba(212, 163, 89, 0.3));
        padding: 4px 10px;
        border-radius: 999px;
        margin-bottom: 12px;
      }
      .apd-dr-title {
        font-family: var(--font-heading, "Fraunces", Georgia, serif);
        font-size: 22px;
        font-weight: 600;
        color: var(--apd-parchment-white, #ffffff);
        margin: 0 0 6px 0;
        line-height: 1.25;
        letter-spacing: -0.015em;
      }
      .apd-dr-subtitle {
        font-size: 13.5px;
        color: var(--apd-parchment-muted, #a89f91);
        margin: 0;
        line-height: 1.5;
      }
      .apd-dr-body {
        padding: 24px 28px;
        display: flex;
        flex-direction: column;
        gap: 18px;
        max-height: 70vh;
        overflow-y: auto;
        background: var(--apd-obsidian-card, #171311);
      }
      .apd-dr-label {
        font-size: 12.5px;
        font-weight: 600;
        color: var(--apd-parchment, #f5f0e8);
        margin-bottom: 6px;
        display: flex;
        align-items: center;
        justify-content: space-between;
      }
      .apd-dr-label-sub {
        font-weight: 400;
        font-size: 11px;
        color: var(--apd-gold, #d4a359);
      }
      .apd-dr-input-row {
        display: flex;
        gap: 8px;
      }
      .apd-dr-input {
        flex: 1;
        background: var(--apd-obsidian-input, #0d0a08);
        border: 1.5px solid var(--apd-gold-border, rgba(212, 163, 89, 0.28));
        border-radius: 8px;
        padding: 10px 14px;
        font-size: 13px;
        color: var(--apd-parchment-white, #ffffff);
        outline: none;
        font-family: var(--font-mono, monospace);
      }
      .apd-dr-input:focus {
        border-color: var(--apd-gold, #d4a359);
        box-shadow: 0 0 0 3px var(--apd-gold-glow, rgba(212, 163, 89, 0.2));
      }
      .apd-dr-btn {
        padding: 9px 18px;
        font-size: 13px;
        font-weight: 600;
        border-radius: 8px;
        cursor: pointer;
        display: inline-flex;
        align-items: center;
        justify-content: center;
        gap: 8px;
        transition: all 0.15s ease;
        border: 1px solid transparent;
        font-family: inherit;
        user-select: none;
      }
      .apd-dr-btn-primary {
        background: linear-gradient(135deg, var(--apd-primary, #9e4429) 0%, var(--apd-primary-dark, #71321d) 100%);
        color: var(--apd-parchment-white, #ffffff);
        border: 1px solid var(--apd-primary-border, #b85435);
        box-shadow: 0 3px 12px rgba(113, 50, 29, 0.45);
      }
      .apd-dr-btn-primary:hover {
        filter: brightness(1.12);
        transform: translateY(-1px);
      }
      .apd-dr-btn-secondary {
        background: rgba(255, 255, 255, 0.07);
        color: var(--apd-parchment, #f5f0e8);
        border: 1px solid var(--apd-gold-border, rgba(212, 163, 89, 0.3));
      }
      .apd-dr-btn-secondary:hover {
        background: var(--apd-gold-bg, rgba(212, 163, 89, 0.18));
        border-color: var(--apd-gold, #d4a359);
        color: var(--apd-parchment-white, #ffffff);
      }
      .apd-dr-warning-box {
        background: var(--apd-warning-bg, rgba(245, 158, 11, 0.1));
        border: 1px solid var(--apd-warning-border, rgba(245, 158, 11, 0.35));
        border-radius: 8px;
        padding: 14px 16px;
        display: flex;
        gap: 12px;
        color: var(--apd-parchment, #f5f0e8);
        font-size: 12.5px;
        line-height: 1.5;
      }
      .apd-dr-warning-icon {
        color: var(--apd-warning, #fbbf24);
        font-size: 20px;
        flex-shrink: 0;
      }
      .apd-dr-warning-title {
        font-weight: 700;
        color: var(--apd-warning, #fbbf24);
        margin-bottom: 2px;
      }
      .apd-dr-footer {
        padding: 16px 28px;
        border-top: 1px solid var(--apd-gold-border-muted, rgba(212, 163, 89, 0.18));
        background: var(--apd-obsidian-footer, #1c1714);
        display: flex;
        justify-content: flex-end;
        gap: 10px;
      }
      .apd-dr-privacy-note {
        font-size: 12px;
        color: var(--apd-parchment-muted, #a89f91);
        line-height: 1.5;
        display: flex;
        align-items: center;
        gap: 8px;
      }
      .apd-dr-privacy-note svg {
        color: var(--apd-gold, #d4a359);
        flex-shrink: 0;
      }
    `;
    document.head.appendChild(style);
  }

  function closeModal() {
    var el = document.getElementById('apd-dataroot-modal-root');
    if (el) el.remove();
  }

  var RootDialog = {
    close: closeModal,

    showFirstLaunchModal: function (status, onConfirmed) {
      closeModal();
      injectStyles();

      var ICONS = window.APD_ICONS || {};
      var iconLandmark = ICONS.landmark || '';
      var iconFolder = ICONS.folder || '';
      var iconWarning = ICONS.alert || ICONS.warning || '';
      var iconLock = ICONS.lock || '';
      var iconRefresh = ICONS.refresh || '';
      var iconCheck = ICONS.checkCircle || ICONS.check || '';

      var initialPath = (status && (status.data_root || status.default_path)) || 'D:\\ArchaeoPhD-Data';
      var cloudWarning = (status && status.cloud_service) || '';

      var backdrop = document.createElement('div');
      backdrop.id = 'apd-dataroot-modal-root';
      backdrop.className = 'apd-dr-backdrop';

      backdrop.innerHTML = `
        <div class="apd-dr-modal">
          <div class="apd-dr-header">
            <div class="apd-dr-badge">
              <span>${iconLandmark}</span> ArchaeoPhD Offline Workstation
            </div>
            <h1 class="apd-dr-title">Select Research Data Root</h1>
            <p class="apd-dr-subtitle">
              Choose where your archaeological libraries, native vector indexes, and local AI model weights (GGUF) will be stored.
            </p>
          </div>
          <div class="apd-dr-body">
            <div>
              <div class="apd-dr-label">
                <span>Data Root Directory:</span>
                <span class="apd-dr-label-sub">Choose fast internal drive or external SSD</span>
              </div>
              <div class="apd-dr-input-row">
                <input type="text" id="apd-dr-path-input" class="apd-dr-input" value="${initialPath}">
                <button type="button" id="apd-dr-browse-btn" class="apd-dr-btn apd-dr-btn-secondary">
                  ${iconFolder} Browse...
                </button>
              </div>
            </div>

            <div id="apd-dr-cloud-alert" style="display: ${cloudWarning ? 'flex' : 'none'};" class="apd-dr-warning-box">
              <div class="apd-dr-warning-icon">${iconWarning}</div>
              <div>
                <div class="apd-dr-warning-title">Cloud Storage Detected (<span id="apd-dr-cloud-service">${cloudWarning}</span>)</div>
                <div>
                  Storing multi-gigabyte models and raw research data inside a cloud-synced folder can consume cloud quota and risk unintentionally syncing unpublished archaeological findings. A dedicated local drive (e.g. <code>D:\\\\ArchaeoPhD-Data</code>) is strongly recommended.
                </div>
              </div>
            </div>

            <div class="apd-dr-privacy-note">
              <span>${iconLock}</span>
              <span>Your selection is saved locally in your user profile settings. Zero telemetry or research data leaves your PC.</span>
            </div>
          </div>
          <div class="apd-dr-footer">
            <button type="button" id="apd-dr-default-btn" class="apd-dr-btn apd-dr-btn-secondary">
              Use Standard Default
            </button>
            <button type="button" id="apd-dr-confirm-btn" class="apd-dr-btn apd-dr-btn-primary">
              Confirm &amp; Initialize Workstation
            </button>
          </div>
        </div>`;

      document.body.appendChild(backdrop);

      var inputEl = document.getElementById('apd-dr-path-input');
      var browseBtn = document.getElementById('apd-dr-browse-btn');
      var defaultBtn = document.getElementById('apd-dr-default-btn');
      var confirmBtn = document.getElementById('apd-dr-confirm-btn');
      var cloudAlert = document.getElementById('apd-dr-cloud-alert');
      var cloudServiceSpan = document.getElementById('apd-dr-cloud-service');

      function verifyCloud(p) {
        if (!window.RootStore) return;
        window.RootStore.checkCloudSync(p).then(function (res) {
          if (res && res.cloud_service) {
            cloudServiceSpan.textContent = res.cloud_service;
            cloudAlert.style.display = 'flex';
          } else {
            cloudAlert.style.display = 'none';
          }
        }).catch(function () {});
      }

      inputEl.addEventListener('input', function () {
        verifyCloud(inputEl.value.trim());
      });

      browseBtn.addEventListener('click', function () {
        browseBtn.disabled = true;
        if (!window.RootStore) return;
        window.RootStore.browseFolder(inputEl.value.trim())
          .then(function (res) {
            browseBtn.disabled = false;
            if (res && !res.cancelled && res.path) {
              inputEl.value = res.path;
              if (res.cloud_service) {
                cloudServiceSpan.textContent = res.cloud_service;
                cloudAlert.style.display = 'flex';
              } else {
                cloudAlert.style.display = 'none';
              }
            }
          })
          .catch(function () {
            browseBtn.disabled = false;
          });
      });

      defaultBtn.addEventListener('click', function () {
        if (status && status.default_path) {
          inputEl.value = status.default_path;
          verifyCloud(status.default_path);
        }
      });

      confirmBtn.addEventListener('click', function () {
        var selectedPath = inputEl.value.trim();
        if (!selectedPath) return;

        confirmBtn.disabled = true;
        confirmBtn.innerHTML = '<span>' + iconRefresh + '</span> Initializing...';

        if (window.RootStore) {
          window.RootStore.setDataRoot(selectedPath)
            .then(function () {
              confirmBtn.innerHTML = '<span>' + iconCheck + '</span> Initialized!';
              setTimeout(function () {
                closeModal();
                if (typeof onConfirmed === 'function') onConfirmed();
              }, 350);
            })
            .catch(function (err) {
              confirmBtn.disabled = false;
              confirmBtn.innerHTML = 'Confirm &amp; Initialize Workstation';
              if (window.APD_MODAL) {
                window.APD_MODAL.toast('Failed to set Data Root: ' + (err.message || err), 'error');
              }
            });
        }
      });
    },

    showMissingRootModal: function (status, onReconnected) {
      closeModal();
      injectStyles();

      var ICONS = window.APD_ICONS || {};
      var iconWarning = ICONS.alert || ICONS.warning || '';
      var iconFolder = ICONS.folder || '';
      var iconRefresh = ICONS.refresh || '';
      var iconCheck = ICONS.checkCircle || ICONS.check || '';

      var backdrop = document.createElement('div');
      backdrop.id = 'apd-dataroot-modal-root';
      backdrop.className = 'apd-dr-backdrop';

      backdrop.innerHTML = `
        <div class="apd-dr-modal">
          <div class="apd-dr-header" style="background: rgba(220, 38, 38, 0.12); border-bottom: 1px solid rgba(220, 38, 38, 0.25);">
            <div class="apd-dr-badge" style="color: #f87171; background: rgba(220, 38, 38, 0.15); border-color: rgba(220, 38, 38, 0.35);">
              <span>${iconWarning}</span> Storage Disconnected
            </div>
            <h1 class="apd-dr-title" style="color: #ffffff;">Research Data Root Unavailable</h1>
            <p class="apd-dr-subtitle">
              The configured research storage location cannot be accessed.
            </p>
          </div>
          <div class="apd-dr-body">
            <div style="background: rgba(220, 38, 38, 0.08); border: 1px solid rgba(220, 38, 38, 0.25); border-radius: 8px; padding: 14px 16px;">
              <div style="font-size: 12px; color: #f87171; margin-bottom: 4px; font-weight: 600;">Configured Data Root:</div>
              <div style="font-family: var(--font-mono, monospace); font-size: 13px; color: #ffffff; word-break: break-all; font-weight: 600;">${(status && status.data_root) || ''}</div>
            </div>

            <div style="font-size: 13px; color: #d1c7bc; line-height: 1.6;">
              If your research data is stored on an <strong style="color: #ffffff;">external SSD or USB drive</strong>, please plug it into your computer. ArchaeoPhD never writes to a fallback drive to protect against fragmenting your research graphs.
            </div>
          </div>
          <div class="apd-dr-footer" style="justify-content: space-between;">
            <button type="button" id="apd-dr-missing-browse-btn" class="apd-dr-btn apd-dr-btn-secondary">
              ${iconFolder} Select Different Folder...
            </button>
            <button type="button" id="apd-dr-retry-btn" class="apd-dr-btn apd-dr-btn-primary">
              ${iconRefresh} Retry Connection
            </button>
          </div>
        </div>
      `;

      document.body.appendChild(backdrop);

      var retryBtn = document.getElementById('apd-dr-retry-btn');
      var browseBtn = document.getElementById('apd-dr-missing-browse-btn');

      retryBtn.addEventListener('click', function () {
        retryBtn.disabled = true;
        retryBtn.innerHTML = '<span>' + iconRefresh + '</span> Checking...';

        if (window.RootStore) {
          window.RootStore.fetchStatus().then(function (newStatus) {
            if (!newStatus.is_missing) {
              retryBtn.innerHTML = '<span>' + iconCheck + '</span> Connected!';
              setTimeout(function () {
                closeModal();
                if (typeof onReconnected === 'function') onReconnected();
              }, 300);
            } else {
              retryBtn.disabled = false;
              retryBtn.innerHTML = iconRefresh + ' Retry Connection';
              if (window.APD_MODAL) {
                window.APD_MODAL.toast('Drive or path is still unavailable. Please reconnect the drive or select a different folder.', 'warning');
              }
            }
          }).catch(function () {
            retryBtn.disabled = false;
            retryBtn.innerHTML = iconRefresh + ' Retry Connection';
          });
        }
      });

      browseBtn.addEventListener('click', function () {
        RootDialog.showFirstLaunchModal(status, onReconnected);
      });
    }
  };

  window.RootDialog = RootDialog;
})();

