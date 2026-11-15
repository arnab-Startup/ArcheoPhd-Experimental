/**
 * ArchaeoPhD — Auto-Updater Ergonomics & Styles
 * 
 * Clean, lightweight styles for the header Update button and minimal toast.
 * Zero clutter, zero emojis.
 */

(function () {
  'use strict';

  function injectStyles() {
    if (document.getElementById('apd-auto-updater-styles')) return;

    var style = document.createElement('style');
    style.id = 'apd-auto-updater-styles';
    style.textContent = `
      /* Header Update Button */
      .apd-header-update-btn {
        display: inline-flex;
        align-items: center;
        gap: 6px;
        height: 30px;
        padding: 0 10px;
        border-radius: 6px;
        font-family: inherit;
        font-size: 12px;
        font-weight: 500;
        cursor: pointer;
        transition: all 0.15s ease;
        user-select: none;
        white-space: nowrap;
        border: 1px solid transparent;
        background: transparent;
        color: var(--muted-foreground, #94a3b8);
      }

      .apd-header-update-btn:hover {
        background: rgba(255, 255, 255, 0.06);
        color: #ffffff;
      }

      /* Update Available */
      .apd-header-update-btn.has-update {
        background: rgba(148, 70, 42, 0.2);
        border-color: rgba(200, 155, 60, 0.4);
        color: #fce7cf;
        box-shadow: 0 1px 6px rgba(148, 70, 42, 0.25);
      }

      .apd-header-update-btn.has-update:hover {
        background: rgba(148, 70, 42, 0.35);
        border-color: rgba(200, 155, 60, 0.7);
        color: #ffffff;
      }

      /* Updating In-Progress */
      .apd-header-update-btn.updating {
        background: rgba(255, 255, 255, 0.08);
        border-color: rgba(255, 255, 255, 0.2);
        color: #ffffff;
        cursor: wait;
      }

      /* Up to date */
      .apd-header-update-btn.up-to-date {
        opacity: 0.75;
      }

      .apd-header-update-btn.up-to-date:hover {
        opacity: 1;
      }

      .apd-update-dot {
        width: 6px;
        height: 6px;
        border-radius: 50%;
        background: #e2b35a;
        box-shadow: 0 0 6px rgba(226, 179, 90, 0.9);
        animation: apd-pulse-glow 1.8s infinite ease-in-out;
        flex-shrink: 0;
      }

      @keyframes apd-pulse-glow {
        0%, 100% { opacity: 1; transform: scale(1); }
        50% { opacity: 0.5; transform: scale(1.2); }
      }

      .apd-spin {
        animation: apd-rotate 1s linear infinite;
      }

      @keyframes apd-rotate {
        from { transform: rotate(0deg); }
        to { transform: rotate(360deg); }
      }

      /* Clean Minimal Toast */
      .apd-updater-toast {
        position: fixed;
        bottom: 24px;
        right: 24px;
        background: #1e1916;
        border: 1px solid rgba(212, 163, 89, 0.35);
        border-radius: 8px;
        padding: 10px 16px;
        box-shadow: 0 8px 24px rgba(0, 0, 0, 0.4);
        color: #f5f0e8;
        font-size: 12.5px;
        font-family: inherit;
        z-index: 100000;
        transform: translateY(12px);
        opacity: 0;
        pointer-events: none;
        transition: all 0.2s ease;
      }

      .apd-updater-toast.visible {
        transform: translateY(0);
        opacity: 1;
        pointer-events: auto;
      }
    `;

    document.head.appendChild(style);
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', injectStyles);
  } else {
    injectStyles();
  }
})();
