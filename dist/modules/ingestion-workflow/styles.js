/**
 * ArchaeoPhD — Ingestion Workflow Stylesheet
 * 
 * Scoped styles for Document Ingestion, Reliability Classification,
 * Discrepancy Verification Queue, and Manual Transcription Canvas.
 */

(function () {
  'use strict';

  function injectWorkflowStyles() {
    if (document.getElementById('apd-workflow-styles')) return;

    var style = document.createElement('style');
    style.id = 'apd-workflow-styles';
    style.textContent = `
      /* Root Backdrop */
      .apd-wf-backdrop {
        position: fixed;
        inset: 0;
        background: rgba(15, 12, 10, 0.72);
        backdrop-filter: blur(8px);
        -webkit-backdrop-filter: blur(8px);
        z-index: 999990;
        display: flex;
        align-items: center;
        justify-content: center;
        padding: 24px;
        font-family: var(--font-body, "Inter", -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif);
        color: hsl(var(--foreground, 30 10% 12%));
        animation: apdWfFadeIn 0.22s cubic-bezier(0.16, 1, 0.3, 1);
      }
      @keyframes apdWfFadeIn {
        from { opacity: 0; transform: scale(0.98); }
        to { opacity: 1; transform: scale(1); }
      }

      /* Modal Containers */
      .apd-wf-modal {
        background: hsl(var(--card, 40 33% 99%));
        border: 1px solid hsl(var(--border, 36 16% 87%));
        border-radius: calc(var(--radius, 10px) + 6px);
        width: 100%;
        max-width: 680px;
        box-shadow: 0 30px 60px -15px rgba(0, 0, 0, 0.3), 0 0 0 1px hsl(var(--border, 36 16% 87%) / 0.5);
        overflow: hidden;
        display: flex;
        flex-direction: column;
        color: hsl(var(--card-foreground, 30 10% 12%));
        max-height: 90vh;
      }
      .apd-wf-modal-wide {
        max-width: 1100px;
        height: 88vh;
      }

      /* Header */
      .apd-wf-header {
        padding: 22px 28px 18px;
        border-bottom: 1px solid hsl(var(--border, 36 16% 87%));
        background: hsl(var(--muted, 38 20% 94%) / 0.45);
        display: flex;
        align-items: flex-start;
        justify-content: space-between;
        gap: 16px;
      }
      .apd-wf-header-left {
        flex: 1;
      }
      .apd-wf-close-btn {
        background: transparent;
        border: none;
        font-size: 20px;
        line-height: 1;
        cursor: pointer;
        color: hsl(var(--muted-foreground, 30 8% 42%));
        padding: 4px 8px;
        border-radius: var(--radius, 6px);
        transition: background 0.15s, color 0.15s;
      }
      .apd-wf-close-btn:hover {
        background: hsl(var(--muted, 38 20% 94%));
        color: hsl(var(--foreground, 30 10% 12%));
      }

      /* Typography */
      .apd-wf-title {
        font-family: var(--font-heading, "Fraunces", Georgia, serif);
        font-size: 22px;
        font-weight: 600;
        color: hsl(var(--foreground, 30 10% 12%));
        margin: 0 0 4px 0;
        line-height: 1.25;
        letter-spacing: -0.015em;
      }
      .apd-wf-subtitle {
        font-size: 13px;
        color: hsl(var(--muted-foreground, 30 8% 42%));
        margin: 0;
        line-height: 1.5;
      }

      /* Badges */
      .apd-wf-badge {
        display: inline-flex;
        align-items: center;
        gap: 6px;
        font-size: 11px;
        font-weight: 700;
        text-transform: uppercase;
        letter-spacing: 0.08em;
        padding: 3px 9px;
        border-radius: 999px;
        margin-bottom: 8px;
        border: 1px solid transparent;
      }
      .apd-wf-badge-terracotta {
        color: hsl(var(--primary, 18 58% 28%));
        background: hsl(var(--primary, 18 58% 28%) / 0.1);
        border-color: hsl(var(--primary, 18 58% 28%) / 0.25);
      }
      .apd-wf-badge-amber {
        color: hsl(32 85% 35%);
        background: hsl(38 92% 50% / 0.12);
        border-color: hsl(38 92% 50% / 0.35);
      }
      .apd-wf-badge-emerald {
        color: hsl(158 64% 32%);
        background: hsl(158 64% 42% / 0.12);
        border-color: hsl(158 64% 42% / 0.35);
      }
      .apd-wf-badge-crimson {
        color: hsl(0 72% 40%);
        background: hsl(0 72% 51% / 0.12);
        border-color: hsl(0 72% 51% / 0.35);
      }

      /* Modal Body & Scrolling */
      .apd-wf-body {
        padding: 24px 28px;
        display: flex;
        flex-direction: column;
        gap: 20px;
        overflow-y: auto;
        flex: 1;
      }

      /* Inputs and Labels */
      .apd-wf-field {
        display: flex;
        flex-direction: column;
        gap: 6px;
      }
      .apd-wf-label {
        font-size: 12.5px;
        font-weight: 600;
        color: hsl(var(--foreground, 30 10% 12%));
        display: flex;
        align-items: center;
        justify-content: space-between;
      }
      .apd-wf-input {
        background: hsl(var(--background, 40 30% 98%));
        border: 1px solid hsl(var(--input, 36 16% 87%));
        border-radius: var(--radius, 8px);
        padding: 9px 13px;
        font-size: 13px;
        color: hsl(var(--foreground, 30 10% 12%));
        outline: none;
        transition: border-color 0.15s, box-shadow 0.15s;
        font-family: inherit;
        width: 100%;
        box-sizing: border-box;
      }
      .apd-wf-input:focus {
        border-color: hsl(var(--ring, 18 58% 28%));
        box-shadow: 0 0 0 3px hsl(var(--ring, 18 58% 28%) / 0.18);
      }
      .apd-wf-input-mono {
        font-family: var(--font-mono, monospace);
        font-size: 12px;
      }
      .apd-wf-input-row {
        display: flex;
        gap: 8px;
      }

      /* Buttons */
      .apd-wf-btn {
        padding: 8px 16px;
        font-size: 13px;
        font-weight: 600;
        border-radius: var(--radius, 8px);
        cursor: pointer;
        display: inline-flex;
        align-items: center;
        justify-content: center;
        gap: 7px;
        transition: all 0.15s ease;
        border: 1px solid transparent;
        font-family: inherit;
        text-decoration: none;
        white-space: nowrap;
      }
      .apd-wf-btn:disabled, .apd-wf-btn[disabled] {
        opacity: 0.45;
        cursor: not-allowed;
        pointer-events: none;
      }
      .apd-wf-btn-primary {
        background: hsl(var(--primary, 18 58% 28%));
        color: hsl(var(--primary-foreground, 40 30% 98%));
        border-color: hsl(var(--primary, 18 58% 28%));
      }
      .apd-wf-btn-primary:hover:not(:disabled) {
        filter: brightness(1.08);
        transform: translateY(-1px);
        box-shadow: 0 3px 10px hsl(var(--primary, 18 58% 28%) / 0.25);
      }
      .apd-wf-btn-secondary {
        background: hsl(var(--secondary, 38 22% 93%));
        color: hsl(var(--secondary-foreground, 30 10% 14%));
        border-color: hsl(var(--border, 36 16% 87%));
      }
      .apd-wf-btn-secondary:hover:not(:disabled) {
        background: hsl(var(--muted, 38 20% 94%));
      }
      .apd-wf-btn-outline {
        background: transparent;
        color: hsl(var(--foreground, 30 10% 12%));
        border-color: hsl(var(--border, 36 16% 87%));
      }
      .apd-wf-btn-outline:hover:not(:disabled) {
        background: hsl(var(--muted, 38 20% 94%) / 0.5);
      }
      .apd-wf-btn-danger {
        background: hsl(0 72% 51% / 0.12);
        color: hsl(0 72% 45%);
        border-color: hsl(0 72% 51% / 0.3);
      }
      .apd-wf-btn-danger:hover:not(:disabled) {
        background: hsl(0 72% 51% / 0.22);
      }

      /* Alert / Notice Boxes */
      .apd-wf-box {
        border-radius: var(--radius, 10px);
        padding: 14px 16px;
        display: flex;
        gap: 12px;
        font-size: 12.5px;
        line-height: 1.55;
        border: 1px solid transparent;
      }
      .apd-wf-box-amber {
        background: hsl(38 92% 50% / 0.08);
        border-color: hsl(38 92% 50% / 0.3);
        color: hsl(var(--foreground, 30 10% 12%));
      }
      .apd-wf-box-crimson {
        background: hsl(0 72% 51% / 0.08);
        border-color: hsl(0 72% 51% / 0.3);
        color: hsl(var(--foreground, 30 10% 12%));
      }
      .apd-wf-box-emerald {
        background: hsl(158 64% 42% / 0.08);
        border-color: hsl(158 64% 42% / 0.3);
        color: hsl(var(--foreground, 30 10% 12%));
      }
      .apd-wf-box-muted {
        background: hsl(var(--muted, 38 20% 94%) / 0.6);
        border-color: hsl(var(--border, 36 16% 87%));
        color: hsl(var(--foreground, 30 10% 12%));
      }
      .apd-wf-box-content {
        flex: 1;
      }
      .apd-wf-box-title {
        font-weight: 700;
        margin-bottom: 3px;
        font-size: 13px;
      }

      /* Footer */
      .apd-wf-footer {
        padding: 16px 28px;
        border-top: 1px solid hsl(var(--border, 36 16% 87%));
        background: hsl(var(--muted, 38 20% 94%) / 0.3);
        display: flex;
        justify-content: flex-end;
        align-items: center;
        gap: 10px;
      }

      /* Classification Option Cards */
      .apd-wf-tier-grid {
        display: grid;
        grid-template-columns: 1fr 1fr;
        gap: 14px;
      }
      .apd-wf-tier-card {
        border: 2px solid hsl(var(--border, 36 16% 87%));
        border-radius: var(--radius, 10px);
        padding: 16px;
        background: hsl(var(--background, 40 30% 98%));
        cursor: pointer;
        transition: all 0.15s ease;
        display: flex;
        flex-direction: column;
        gap: 8px;
      }
      .apd-wf-tier-card:hover {
        border-color: hsl(var(--ring, 18 58% 28%) / 0.5);
      }
      .apd-wf-tier-card.selected {
        border-color: hsl(var(--primary, 18 58% 28%));
        background: hsl(var(--primary, 18 58% 28%) / 0.04);
        box-shadow: 0 0 0 2px hsl(var(--primary, 18 58% 28%) / 0.18);
      }
      .apd-wf-tier-name {
        font-family: var(--font-heading, "Fraunces", Georgia, serif);
        font-size: 16px;
        font-weight: 600;
        display: flex;
        align-items: center;
        justify-content: space-between;
      }
      .apd-wf-tier-desc {
        font-size: 12px;
        color: hsl(var(--muted-foreground, 30 8% 42%));
        line-height: 1.45;
      }

      /* Verification Queue Discrepancy Card */
      .apd-wf-vcard {
        border: 1px solid hsl(var(--border, 36 16% 87%));
        border-radius: var(--radius, 10px);
        background: hsl(var(--card, 40 33% 99%));
        padding: 18px 20px;
        display: flex;
        flex-direction: column;
        gap: 14px;
        box-shadow: 0 2px 8px -2px rgba(0, 0, 0, 0.05);
      }
      .apd-wf-crop-box {
        background: #111;
        border: 1px solid hsl(var(--border, 36 16% 87%));
        border-radius: var(--radius, 8px);
        padding: 12px;
        display: flex;
        flex-direction: column;
        align-items: center;
        gap: 8px;
      }
      .apd-wf-crop-img {
        max-width: 100%;
        max-height: 160px;
        object-fit: contain;
        border-radius: 4px;
        box-shadow: 0 4px 12px rgba(0, 0, 0, 0.4);
      }
      .apd-wf-crop-label {
        font-size: 11px;
        font-family: var(--font-mono, monospace);
        color: #aaa;
      }
      .apd-wf-candidates-grid {
        display: grid;
        grid-template-columns: 1fr 1fr;
        gap: 12px;
      }
      .apd-wf-candidate-box {
        border: 1px solid hsl(var(--border, 36 16% 87%));
        border-radius: var(--radius, 8px);
        padding: 12px 14px;
        background: hsl(var(--muted, 38 20% 94%) / 0.4);
        display: flex;
        flex-direction: column;
        gap: 4px;
      }
      .apd-wf-candidate-title {
        font-size: 11px;
        font-weight: 700;
        text-transform: uppercase;
        letter-spacing: 0.06em;
        color: hsl(var(--muted-foreground, 30 8% 42%));
      }
      .apd-wf-candidate-val {
        font-family: var(--font-heading, "Fraunces", Georgia, serif);
        font-size: 16px;
        font-weight: 600;
        color: hsl(var(--foreground, 30 10% 12%));
      }

      /* Manual Transcription Split Screen */
      .apd-wf-split {
        display: flex;
        flex: 1;
        overflow: hidden;
      }
      .apd-wf-split-left {
        flex: 1.1;
        border-right: 1px solid hsl(var(--border, 36 16% 87%));
        display: flex;
        flex-direction: column;
        background: #181512;
        color: #eee;
        overflow: hidden;
      }
      .apd-wf-split-right {
        flex: 1;
        display: flex;
        flex-direction: column;
        overflow-y: auto;
        background: hsl(var(--card, 40 33% 99%));
      }
      .apd-wf-scan-viewport {
        flex: 1;
        display: flex;
        align-items: center;
        justify-content: center;
        overflow: auto;
        padding: 24px;
        position: relative;
      }
      .apd-wf-scan-canvas {
        background: #fdfaf3;
        color: #1a1612;
        width: 100%;
        max-width: 520px;
        min-height: 480px;
        padding: 32px 36px;
        box-shadow: 0 10px 30px rgba(0, 0, 0, 0.5);
        font-family: Georgia, "Times New Roman", serif;
        line-height: 1.6;
        font-size: 13.5px;
        border-radius: 2px;
      }
      .apd-wf-scan-toolbar {
        padding: 10px 18px;
        background: #100e0c;
        border-top: 1px solid #282420;
        display: flex;
        align-items: center;
        justify-content: space-between;
        font-size: 12px;
        color: #aaa;
      }
      .apd-wf-fact-row {
        display: grid;
        grid-template-columns: 1.2fr 1.5fr 1fr 34px;
        gap: 8px;
        align-items: center;
      }

      /* Floating Research Quick-Action Dock */
      .apd-wf-dock {
        position: fixed;
        bottom: 20px;
        left: 50%;
        transform: translateX(-50%);
        z-index: 99990;
        white-space: nowrap;
        display: flex;
        align-items: center;
        gap: 6px;
        background: hsl(var(--card, 40 33% 99%));
        border: 1px solid hsl(var(--border, 36 16% 87%));
        padding: 4px 8px;
        border-radius: 8px;
        box-shadow: 0 4px 18px -2px rgba(0, 0, 0, 0.16), 0 0 0 1px hsl(var(--border, 36 16% 87%) / 0.5);
        font-family: var(--font-body, "Inter", sans-serif);
      }
      .apd-wf-dock-btn {
        background: transparent;
        border: none;
        font-size: 12.5px;
        font-weight: 600;
        color: hsl(var(--foreground, 30 10% 12%));
        padding: 6px 12px;
        border-radius: 999px;
        cursor: pointer;
        display: inline-flex;
        align-items: center;
        gap: 6px;
        transition: all 0.15s ease;
      }
      .apd-wf-dock-btn:hover {
        background: hsl(var(--muted, 38 20% 94%));
        color: hsl(var(--primary, 18 58% 28%));
      }
      .apd-wf-dock-badge {
        background: hsl(var(--primary, 18 58% 28%));
        color: #fff;
        font-size: 10px;
        font-weight: 700;
        padding: 1px 6px;
        border-radius: 999px;
      }
    `;
    document.head.appendChild(style);
  }

  window.IngestionStyles = {
    inject: injectWorkflowStyles
  };
})();

