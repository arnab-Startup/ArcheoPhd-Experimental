/**
 * ArchaeoPhD — Auto-Updater Ergonomics & Styles
 * 
 * Provides polished, responsive, and distraction-free styling for the
 * top-right header update button, 1-click update modal, and status toasts.
 * 100% CSS variables-driven with ZERO emojis.
 */

(function () {
  'use strict';

  function injectStyles() {
    if (document.getElementById('apd-auto-updater-styles')) return;

    var style = document.createElement('style');
    style.id = 'apd-auto-updater-styles';
    style.textContent = `
      /* Header Update Button in Top-Right Navigation */
      .apd-header-update-btn {
        display: inline-flex;
        align-items: center;
        gap: 7px;
        height: 32px;
        padding: 0 12px;
        border-radius: 7px;
        font-family: var(--font-body, "Inter", -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif);
        font-size: 12px;
        font-weight: 500;
        cursor: pointer;
        transition: all 0.18s cubic-bezier(0.16, 1, 0.3, 1);
        user-select: none;
        white-space: nowrap;
        border: 1px solid transparent;
        background: transparent;
        color: var(--apd-parchment-muted, #94a3b8);
      }

      .apd-header-update-btn:hover {
        background: rgba(255, 255, 255, 0.06);
        color: var(--apd-parchment-white, #ffffff);
      }

      /* Update Available State: warm glowing badge */
      .apd-header-update-btn.has-update {
        background: rgba(148, 70, 42, 0.16);
        border-color: rgba(200, 155, 60, 0.38);
        color: #fce7cf;
        box-shadow: 0 2px 10px rgba(148, 70, 42, 0.22);
      }

      .apd-header-update-btn.has-update:hover {
        background: rgba(148, 70, 42, 0.28);
        border-color: rgba(200, 155, 60, 0.6);
        color: #ffffff;
        box-shadow: 0 3px 14px rgba(148, 70, 42, 0.35);
        transform: translateY(-1px);
      }

      .apd-update-dot {
        width: 7px;
        height: 7px;
        border-radius: 50%;
        background: #e2b35a;
        box-shadow: 0 0 8px rgba(226, 179, 90, 0.8);
        animation: apd-pulse-glow 2s infinite ease-in-out;
        flex-shrink: 0;
      }

      @keyframes apd-pulse-glow {
        0%, 100% {
          transform: scale(1);
          opacity: 1;
        }
        50% {
          transform: scale(1.3);
          opacity: 0.6;
        }
      }

      /* Modern 1-Click Update Dialog Modal */
      .apd-updater-backdrop {
        position: fixed;
        inset: 0;
        background: rgba(8, 10, 15, 0.78);
        backdrop-filter: blur(8px);
        -webkit-backdrop-filter: blur(8px);
        z-index: 99999;
        display: flex;
        align-items: center;
        justify-content: center;
        padding: 20px;
        opacity: 0;
        pointer-events: none;
        transition: opacity 0.2s cubic-bezier(0.16, 1, 0.3, 1);
      }

      .apd-updater-backdrop.open {
        opacity: 1;
        pointer-events: auto;
      }

      .apd-updater-card {
        background: #181412;
        border: 1px solid rgba(212, 163, 89, 0.28);
        border-radius: 14px;
        width: 100%;
        max-width: 520px;
        box-shadow: 0 20px 48px rgba(0, 0, 0, 0.65), 0 0 0 1px rgba(255, 255, 255, 0.04);
        overflow: hidden;
        color: #f5f0e8;
        font-family: var(--font-body, "Inter", -apple-system, sans-serif);
        transform: translateY(12px) scale(0.98);
        transition: all 0.22s cubic-bezier(0.16, 1, 0.3, 1);
      }

      .apd-updater-backdrop.open .apd-updater-card {
        transform: translateY(0) scale(1);
      }

      .apd-updater-header {
        display: flex;
        align-items: center;
        justify-content: space-between;
        padding: 18px 22px;
        border-bottom: 1px solid rgba(255, 255, 255, 0.08);
        background: rgba(255, 255, 255, 0.02);
      }

      .apd-updater-title-wrap {
        display: flex;
        align-items: center;
        gap: 10px;
      }

      .apd-updater-icon-badge {
        width: 34px;
        height: 34px;
        border-radius: 8px;
        background: rgba(148, 70, 42, 0.25);
        border: 1px solid rgba(200, 155, 60, 0.35);
        color: #e2b35a;
        display: flex;
        align-items: center;
        justify-content: center;
        flex-shrink: 0;
      }

      .apd-updater-title {
        margin: 0;
        font-size: 15px;
        font-weight: 600;
        color: #ffffff;
      }

      .apd-updater-subtitle {
        margin: 0;
        font-size: 12px;
        color: #a89f91;
      }

      .apd-updater-close {
        background: transparent;
        border: none;
        color: #a89f91;
        width: 28px;
        height: 28px;
        border-radius: 6px;
        cursor: pointer;
        display: flex;
        align-items: center;
        justify-content: center;
        transition: background 0.15s, color 0.15s;
      }

      .apd-updater-close:hover {
        background: rgba(255, 255, 255, 0.08);
        color: #ffffff;
      }

      .apd-updater-body {
        padding: 22px;
      }

      .apd-version-pill-row {
        display: flex;
        align-items: center;
        gap: 10px;
        margin-bottom: 14px;
      }

      .apd-ver-pill {
        padding: 3px 9px;
        border-radius: 6px;
        font-size: 11.5px;
        font-weight: 600;
        letter-spacing: 0.02em;
      }

      .apd-ver-pill.new {
        background: rgba(16, 185, 129, 0.15);
        border: 1px solid rgba(16, 185, 129, 0.35);
        color: #34d399;
      }

      .apd-ver-pill.current {
        background: rgba(255, 255, 255, 0.06);
        border: 1px solid rgba(255, 255, 255, 0.12);
        color: #94a3b8;
      }

      .apd-updater-reassurance {
        background: rgba(200, 155, 60, 0.08);
        border: 1px solid rgba(200, 155, 60, 0.22);
        border-radius: 9px;
        padding: 12px 14px;
        margin-bottom: 16px;
        font-size: 12.5px;
        color: #e5ded3;
        line-height: 1.45;
        display: flex;
        align-items: flex-start;
        gap: 10px;
      }

      .apd-updater-reassurance strong {
        color: #ffffff;
      }

      .apd-updater-highlights {
        margin: 0 0 18px 0;
        padding: 0 0 0 18px;
        font-size: 12.5px;
        color: #d1c8bb;
        line-height: 1.6;
      }

      .apd-updater-highlights li {
        margin-bottom: 4px;
      }

      /* Progress Bar State */
      .apd-updater-progress-wrap {
        display: none;
        margin-top: 14px;
        background: rgba(0, 0, 0, 0.28);
        border: 1px solid rgba(255, 255, 255, 0.08);
        border-radius: 8px;
        padding: 14px;
      }

      .apd-progress-track {
        height: 6px;
        border-radius: 3px;
        background: rgba(255, 255, 255, 0.08);
        overflow: hidden;
        position: relative;
        margin-bottom: 8px;
      }

      .apd-progress-fill {
        position: absolute;
        top: 0;
        left: 0;
        height: 100%;
        width: 0%;
        background: linear-gradient(90deg, #94462a, #e2b35a);
        border-radius: 3px;
        transition: width 0.3s ease;
      }

      .apd-progress-fill.indeterminate {
        width: 40%;
        animation: apd-progress-indet 1.4s infinite ease-in-out;
      }

      @keyframes apd-progress-indet {
        0% { left: -40%; }
        100% { left: 100%; }
      }

      .apd-progress-label {
        font-size: 12px;
        color: #a89f91;
        display: flex;
        justify-content: space-between;
      }

      .apd-updater-footer {
        display: flex;
        align-items: center;
        justify-content: flex-end;
        gap: 10px;
        padding: 14px 22px;
        border-top: 1px solid rgba(255, 255, 255, 0.08);
        background: rgba(0, 0, 0, 0.15);
      }

      .apd-btn-update-cancel {
        background: transparent;
        border: 1px solid rgba(255, 255, 255, 0.14);
        color: #d1c8bb;
        padding: 8px 16px;
        border-radius: 7px;
        font-size: 12.5px;
        font-weight: 500;
        cursor: pointer;
        transition: all 0.15s;
      }

      .apd-btn-update-cancel:hover {
        background: rgba(255, 255, 255, 0.06);
        color: #ffffff;
      }

      .apd-btn-update-primary {
        background: linear-gradient(135deg, #94462a 0%, #b35634 100%);
        border: 1px solid rgba(200, 155, 60, 0.4);
        color: #ffffff;
        padding: 8px 20px;
        border-radius: 7px;
        font-size: 12.5px;
        font-weight: 600;
        cursor: pointer;
        display: inline-flex;
        align-items: center;
        gap: 8px;
        box-shadow: 0 2px 10px rgba(148, 70, 42, 0.35);
        transition: all 0.18s cubic-bezier(0.16, 1, 0.3, 1);
      }

      .apd-btn-update-primary:hover {
        background: linear-gradient(135deg, #aa5131 0%, #c55f3a 100%);
        box-shadow: 0 4px 16px rgba(148, 70, 42, 0.5);
        transform: translateY(-1px);
      }

      .apd-btn-update-primary:disabled {
        opacity: 0.6;
        cursor: not-allowed;
        transform: none;
      }

      /* Clean Top-Right Toast Notification */
      .apd-updater-toast {
        position: fixed;
        top: 20px;
        right: 20px;
        background: #1e1916;
        border: 1px solid rgba(212, 163, 89, 0.3);
        border-radius: 9px;
        padding: 12px 16px;
        box-shadow: 0 10px 30px rgba(0, 0, 0, 0.45);
        color: #f5f0e8;
        font-family: var(--font-body, "Inter", -apple-system, sans-serif);
        font-size: 12.5px;
        display: flex;
        align-items: center;
        gap: 10px;
        z-index: 100000;
        transform: translateY(-20px);
        opacity: 0;
        pointer-events: none;
        transition: all 0.22s cubic-bezier(0.16, 1, 0.3, 1);
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
