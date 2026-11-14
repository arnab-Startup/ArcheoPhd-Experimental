/**
 * ArchaeoPhD — Centralized Accessible Modal & Dialog System
 * 
 * Reusable modal card overlay, toast banners, and keyboard accessibility.
 * Zero emojis, 100% SVG Lucide-style vector styling.
 */

(function () {
  'use strict';

  function injectModalStyles() {
    if (document.getElementById('apd-shared-modal-styles')) return;
    var style = document.createElement('style');
    style.id = 'apd-shared-modal-styles';
    style.textContent = `
      .apd-shared-modal-overlay {
        position: fixed;
        inset: 0;
        z-index: 10000;
        background: rgba(0, 0, 0, 0.6);
        backdrop-filter: blur(4px);
        display: flex;
        align-items: center;
        justify-content: center;
        padding: 16px;
        animation: apdModalFadeIn 0.15s ease-out;
      }
      .apd-shared-modal-card {
        width: 100%;
        max-width: 540px;
        background: hsl(var(--card, 38 28% 97%));
        border: 1px solid hsl(var(--border, 36 16% 87%));
        border-radius: 12px;
        box-shadow: 0 20px 40px -8px rgba(0, 0, 0, 0.35);
        overflow: hidden;
      }
      @keyframes apdModalFadeIn {
        from { opacity: 0; transform: scale(0.98); }
        to { opacity: 1; transform: scale(1); }
      }
      .apd-shared-toast {
        position: fixed;
        bottom: 24px;
        right: 24px;
        z-index: 10001;
        background: hsl(var(--card, 38 28% 97%));
        border: 1px solid hsl(var(--border, 36 16% 87%));
        border-radius: 8px;
        padding: 10px 16px;
        box-shadow: 0 10px 25px -4px rgba(0,0,0,0.2);
        font-size: 12px;
        font-weight: 500;
        color: hsl(var(--foreground, 30 10% 12%));
        display: flex;
        align-items: center;
        gap: 8px;
        animation: apdModalFadeIn 0.2s ease-out;
      }
    `;
    document.head.appendChild(style);
  }

  var APD_MODAL = {
    show: function (options) {
      injectModalStyles();
      var existing = document.getElementById('apd-shared-modal-root');
      if (existing) existing.remove();

      options = options || {};
      var overlay = document.createElement('div');
      overlay.id = 'apd-shared-modal-root';
      overlay.className = 'apd-shared-modal-overlay';

      var icons = window.APD_ICONS || {};
      var closeIcon = icons.x || '<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="18" x2="6" y1="6" y2="18"/><line x1="6" x2="18" y1="6" y2="18"/></svg>';

      overlay.innerHTML = `
        <div class="apd-shared-modal-card p-6 space-y-4">
          <div class="flex items-start justify-between pb-3 border-b border-border">
            <div class="flex items-center gap-2.5">
              ${options.icon ? `<span class="p-2 rounded-lg bg-primary/10 border border-primary/20 text-primary">${options.icon}</span>` : ''}
              <div>
                <h2 class="text-base font-bold font-display text-foreground">${options.title || 'Dialog'}</h2>
                ${options.subtitle ? `<p class="text-xs text-muted-foreground mt-0.5">${options.subtitle}</p>` : ''}
              </div>
            </div>
            <button id="apd-modal-close-x" class="text-muted-foreground hover:text-foreground p-1 rounded-md hover:bg-muted transition-colors">
              ${closeIcon}
            </button>
          </div>

          <div id="apd-modal-body" class="space-y-3">
            ${options.contentHtml || ''}
          </div>

          <div class="flex items-center justify-end gap-2 pt-3 border-t border-border">
            ${options.cancelText !== null ? `
              <button id="apd-modal-btn-cancel" type="button" class="px-3.5 py-1.5 rounded-lg border border-border bg-card text-xs font-semibold text-muted-foreground hover:text-foreground hover:bg-muted transition-all">
                ${options.cancelText || 'Cancel'}
              </button>
            ` : ''}
            ${options.confirmText !== null ? `
              <button id="apd-modal-btn-confirm" type="button" class="inline-flex items-center gap-1.5 px-4 py-1.5 rounded-lg bg-primary text-white text-xs font-semibold hover:bg-primary/90 transition-all shadow-sm">
                ${options.confirmText || 'Confirm'}
              </button>
            ` : ''}
          </div>
        </div>
      `;

      document.body.appendChild(overlay);

      var closeBtn = overlay.querySelector('#apd-modal-close-x');
      var cancelBtn = overlay.querySelector('#apd-modal-btn-cancel');
      var confirmBtn = overlay.querySelector('#apd-modal-btn-confirm');
      var bodyEl = overlay.querySelector('#apd-modal-body');

      function close() {
        overlay.remove();
        if (typeof options.onClose === 'function') options.onClose();
      }

      if (closeBtn) closeBtn.onclick = close;
      if (cancelBtn) {
        cancelBtn.onclick = function () {
          if (typeof options.onCancel === 'function') options.onCancel();
          close();
        };
      }
      if (confirmBtn) {
        confirmBtn.onclick = function () {
          if (typeof options.onConfirm === 'function') {
            var shouldClose = options.onConfirm(overlay, close);
            if (shouldClose !== false) close();
          } else {
            close();
          }
        };
      }

      overlay.onclick = function (e) {
        if (e.target === overlay) close();
      };

      document.addEventListener('keydown', function escHandler(e) {
        if (e.key === 'Escape' && document.getElementById('apd-shared-modal-root')) {
          close();
          document.removeEventListener('keydown', escHandler);
        }
      });

      if (typeof options.onRender === 'function') {
        options.onRender(overlay, close);
      }

      return overlay;
    },

    toast: function (message, durationMs) {
      injectModalStyles();
      var existing = document.getElementById('apd-shared-toast-root');
      if (existing) existing.remove();

      var toastEl = document.createElement('div');
      toastEl.id = 'apd-shared-toast-root';
      toastEl.className = 'apd-shared-toast';
      toastEl.textContent = message;

      document.body.appendChild(toastEl);
      setTimeout(function () {
        if (toastEl && toastEl.parentNode) toastEl.remove();
      }, durationMs || 3000);
    }
  };

  window.APD_MODAL = APD_MODAL;
})();

