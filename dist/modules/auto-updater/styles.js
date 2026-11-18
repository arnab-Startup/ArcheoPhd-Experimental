/**
 * ArchaeoPhD — Auto-Updater Ergonomics & Styles
 * 
 * Clean, lightweight styles for the header Update button, progress overlay, and minimal toast.
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

      /* Progress Overlay Modal */
      .apd-update-overlay {
        position: fixed;
        inset: 0;
        background: rgba(10, 8, 7, 0.85);
        backdrop-filter: blur(8px);
        -webkit-backdrop-filter: blur(8px);
        display: flex;
        align-items: center;
        justify-content: center;
        z-index: 999999;
        animation: apd-fade-in 0.2s ease-out;
      }

      @keyframes apd-fade-in {
        from { opacity: 0; }
        to { opacity: 1; }
      }

      .apd-update-card {
        background: #181412;
        border: 1px solid rgba(212, 163, 89, 0.4);
        border-radius: 16px;
        padding: 28px;
        width: 90%;
        max-width: 480px;
        box-shadow: 0 24px 60px rgba(0, 0, 0, 0.75);
        color: #f5f0e8;
        font-family: inherit;
        animation: apd-card-in 0.25s cubic-bezier(0.16, 1, 0.3, 1);
      }

      @keyframes apd-card-in {
        from { opacity: 0; transform: scale(0.96) translateY(8px); }
        to { opacity: 1; transform: scale(1) translateY(0); }
      }

      .apd-update-badge {
        display: inline-flex;
        align-items: center;
        gap: 6px;
        padding: 3px 9px;
        border-radius: 4px;
        font-size: 10px;
        font-weight: 700;
        text-transform: uppercase;
        letter-spacing: 0.05em;
        background: rgba(148, 70, 42, 0.35);
        color: #e2b35a;
        margin-bottom: 12px;
      }

      .apd-update-title {
        font-size: 19px;
        font-weight: 700;
        margin: 0 0 6px 0;
        color: #ffffff;
        letter-spacing: -0.01em;
      }

      .apd-update-desc {
        font-size: 12.5px;
        color: #a89f91;
        margin: 0 0 22px 0;
        line-height: 1.4;
      }

      .apd-update-progress-track {
        height: 8px;
        background: rgba(255, 255, 255, 0.08);
        border-radius: 999px;
        overflow: hidden;
        position: relative;
      }

      .apd-update-progress-fill {
        height: 100%;
        background: linear-gradient(90deg, #94462a, #e2b35a);
        border-radius: 999px;
        transition: width 0.35s ease;
      }

      .apd-update-progress-meta {
        display: flex;
        justify-content: space-between;
        align-items: center;
        font-size: 11.5px;
        color: #c4b5a5;
        margin-top: 10px;
      }

      .apd-update-steps {
        margin-top: 22px;
        display: flex;
        flex-direction: column;
        gap: 10px;
        border-top: 1px solid rgba(255, 255, 255, 0.06);
        padding-top: 16px;
      }

      .apd-step {
        display: flex;
        align-items: center;
        gap: 10px;
        font-size: 11.5px;
        color: #6e6459;
        transition: all 0.2s ease;
      }

      .apd-step.active {
        color: #e2b35a;
        font-weight: 500;
      }

      .apd-step.done {
        color: #10b981;
      }

      .apd-step-icon {
        width: 18px;
        height: 18px;
        border-radius: 50%;
        display: flex;
        align-items: center;
        justify-content: center;
        font-size: 10px;
        font-weight: 700;
        background: rgba(255, 255, 255, 0.05);
        border: 1px solid rgba(255, 255, 255, 0.1);
        color: inherit;
        flex-shrink: 0;
      }

      .apd-step.done .apd-step-icon {
        background: rgba(16, 185, 129, 0.15);
        border-color: #10b981;
      }

      .apd-step.active .apd-step-icon {
        background: rgba(226, 179, 90, 0.2);
        border-color: #e2b35a;
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
