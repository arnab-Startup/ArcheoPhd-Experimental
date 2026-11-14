/**
 * ArchaeoPhD — Thesis Studio Stylesheet
 * 
 * Google Docs / Microsoft Word styling for academic dissertation studio.
 */

(function () {
  'use strict';

  function injectThesisStyles() {
    if (document.getElementById('apd-thesis-editor-styles')) return;

    var style = document.createElement('style');
    style.id = 'apd-thesis-editor-styles';
    style.textContent = `
      .apd-te-container {
        font-family: var(--font-body, "Inter", -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif);
        color: hsl(var(--foreground, 30 10% 12%));
      }
      
      /* Google Docs Ribbon & Toolbar */
      .apd-gdocs-ribbon {
        background: hsl(var(--card, 38 28% 97%));
        border: 1px solid hsl(var(--border, 36 16% 87%));
        border-radius: 10px;
        box-shadow: 0 2px 8px -1px rgba(0,0,0,0.06);
      }
      .apd-gdocs-btn {
        display: inline-flex;
        align-items: center;
        justify-content: center;
        height: 28px;
        min-width: 28px;
        padding: 0 6px;
        border-radius: 5px;
        color: hsl(var(--foreground, 30 10% 12%));
        border: 1px solid transparent;
        background: transparent;
        cursor: pointer;
        transition: background 0.12s ease, border-color 0.12s ease;
      }
      .apd-gdocs-btn:hover {
        background: hsl(var(--muted, 36 20% 93%));
        border-color: hsl(var(--border, 36 16% 87%));
      }
      .apd-gdocs-btn:active, .apd-gdocs-btn.active {
        background: hsl(var(--primary, 24 75% 42%) / 0.15);
        color: hsl(var(--primary, 24 75% 42%));
      }

      /* Authentic Document Ruler Bar */
      .apd-doc-ruler {
        width: 100%;
        max-width: 820px;
        height: 18px;
        background: hsl(var(--muted, 36 20% 93%) / 0.85);
        border: 1px solid rgba(0,0,0,0.1);
        border-bottom: none;
        border-radius: 4px 4px 0 0;
        display: flex;
        align-items: flex-end;
        position: relative;
        user-select: none;
        box-sizing: border-box;
      }
      .apd-ruler-tick {
        position: absolute;
        bottom: 0;
        height: 6px;
        border-left: 1px solid rgba(0,0,0,0.3);
        font-size: 8.5px;
        line-height: 1;
        padding-left: 2px;
        color: rgba(0,0,0,0.6);
      }
      .apd-ruler-subtick {
        position: absolute;
        bottom: 0;
        height: 3px;
        border-left: 1px solid rgba(0,0,0,0.18);
      }
      .dark .apd-doc-ruler {
        background: #232220;
        border-color: rgba(255,255,255,0.1);
      }
      .dark .apd-ruler-tick {
        border-left-color: rgba(255,255,255,0.3);
        color: rgba(255,255,255,0.6);
      }
      
      /* Google Docs-style Academic Page Canvas */
      .apd-te-page-wrapper {
        display: flex;
        flex-direction: column;
        align-items: center;
        background: hsl(var(--muted, 36 20% 93%) / 0.45);
        padding: 20px 16px 64px;
        min-height: calc(100vh - 220px);
        overflow-y: auto;
      }
      
      .apd-te-page-sheet {
        width: 100%;
        max-width: 820px;
        min-height: 1080px;
        background: #ffffff;
        color: #1a1a1a;
        box-shadow: 0 4px 20px -2px rgba(0, 0, 0, 0.08), 0 1px 3px rgba(0,0,0,0.05);
        border: 1px solid rgba(0, 0, 0, 0.08);
        border-radius: 0 0 4px 4px;
        padding: 64px 76px;
        outline: none;
        box-sizing: border-box;
        position: relative;
        font-family: "Garamond", "Georgia", "Cambria", "Times New Roman", serif;
        font-size: 15px;
        line-height: 1.65;
        letter-spacing: 0.01em;
        transition: font-size 0.1s ease;
      }
      
      .dark .apd-te-page-sheet {
        background: #181715;
        color: #e6e3df;
        border-color: rgba(255, 255, 255, 0.08);
        box-shadow: 0 4px 24px -2px rgba(0, 0, 0, 0.4);
      }
      
      .apd-te-page-sheet:focus { outline: none; }
      
      /* Typographic Fonts */
      .apd-te-page-sheet[data-font="garamond"] { font-family: "Garamond", "Georgia", serif; }
      .apd-te-page-sheet[data-font="times"] { font-family: "Times New Roman", "Times", serif; }
      .apd-te-page-sheet[data-font="georgia"] { font-family: "Georgia", "Garamond", serif; }
      .apd-te-page-sheet[data-font="modern"] { font-family: "Computer Modern", "Latin Modern Roman", serif; }
      .apd-te-page-sheet[data-font="sans"] { font-family: "Inter", -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; }
      .apd-te-page-sheet[data-font="mono"] { font-family: "JetBrains Mono", "SFMono-Regular", Consolas, monospace; }

      /* Dynamic Font Size */
      .apd-te-page-sheet[data-size="11"] { font-size: 13.5px; }
      .apd-te-page-sheet[data-size="12"] { font-size: 15px; }
      .apd-te-page-sheet[data-size="14"] { font-size: 17px; }
      .apd-te-page-sheet[data-size="16"] { font-size: 19px; }
      .apd-te-page-sheet[data-size="18"] { font-size: 21px; }

      /* Spacing */
      .apd-te-page-sheet[data-spacing="1.0"] { line-height: 1.25; }
      .apd-te-page-sheet[data-spacing="1.15"] { line-height: 1.4; }
      .apd-te-page-sheet[data-spacing="1.5"] { line-height: 1.7; }
      .apd-te-page-sheet[data-spacing="2.0"] { line-height: 2.15; }

      /* Academic Sheet Elements */
      .apd-te-page-sheet h1 {
        font-size: 26px;
        font-weight: 700;
        margin-top: 0;
        margin-bottom: 22px;
        line-height: 1.25;
        border-bottom: 1px solid rgba(0,0,0,0.12);
        padding-bottom: 10px;
      }
      .dark .apd-te-page-sheet h1 { border-bottom-color: rgba(255,255,255,0.12); }
      .apd-te-page-sheet h2 {
        font-size: 20px;
        font-weight: 700;
        margin-top: 28px;
        margin-bottom: 12px;
      }
      .apd-te-page-sheet h3 {
        font-size: 16.5px;
        font-weight: 600;
        margin-top: 20px;
        margin-bottom: 8px;
      }
      .apd-te-page-sheet p {
        margin-top: 0;
        margin-bottom: 14px;
        text-align: justify;
        text-justify: inter-word;
      }
      .apd-te-page-sheet blockquote {
        margin: 18px 0;
        padding: 8px 20px;
        border-left: 3px solid hsl(var(--primary, 24 75% 42%));
        background: rgba(140, 130, 115, 0.06);
        font-style: italic;
        color: inherit;
        opacity: 0.92;
      }
      .apd-te-page-sheet table {
        width: 100%;
        border-collapse: collapse;
        margin: 20px 0;
        font-size: 13.5px;
      }
      .apd-te-page-sheet th, .apd-te-page-sheet td {
        border: 1px solid rgba(140, 130, 115, 0.25);
        padding: 8px 12px;
        text-align: left;
      }
      .apd-te-page-sheet th {
        background: rgba(140, 130, 115, 0.1);
        font-weight: 600;
      }
      
      /* Figures & Plates */
      .apd-doc-figure {
        margin: 24px auto;
        padding: 10px;
        background: rgba(140, 130, 115, 0.04);
        border: 1px solid rgba(140, 130, 115, 0.2);
        border-radius: 6px;
        display: flex;
        flex-direction: column;
        align-items: center;
        max-width: 90%;
      }
      .apd-doc-figure img {
        max-width: 100%;
        max-height: 480px;
        height: auto;
        border-radius: 4px;
        display: block;
        box-shadow: 0 2px 8px rgba(0,0,0,0.1);
      }
      .apd-doc-figure figcaption {
        font-size: 12.5px;
        font-style: italic;
        color: hsl(var(--muted-foreground, 30 8% 46%));
        margin-top: 8px;
        text-align: center;
        outline: none;
        width: 100%;
      }
      
      /* Horizontal Page Break Line */
      .apd-te-page-sheet hr {
        border: none;
        border-top: 1px solid rgba(140, 130, 115, 0.3);
        margin: 28px 0;
      }
      
      /* Floating Inline AI Toolbar */
      #apd-te-floating-ai-bar {
        position: absolute;
        z-index: 9999;
        display: none;
        background: hsl(var(--card, 38 28% 97%));
        border: 1px solid hsl(var(--primary, 24 75% 42%) / 0.4);
        border-radius: 8px;
        padding: 4px 6px;
        box-shadow: 0 10px 25px -3px rgba(0, 0, 0, 0.18), 0 4px 6px -2px rgba(0, 0, 0, 0.05);
        display: flex;
        align-items: center;
        gap: 4px;
      }

      /* Inline Help Me Write Bar */
      .apd-inline-prompt-box {
        margin: 16px 0;
        border: 1px solid hsl(var(--primary, 24 75% 42%) / 0.35);
        background: hsl(var(--primary, 24 75% 42%) / 0.04);
        border-radius: 8px;
        padding: 12px 14px;
        font-family: var(--font-body, sans-serif);
      }

      .apd-te-pulse {
        animation: apdTePulse 1.5s infinite ease-in-out;
      }
      @keyframes apdTePulse {
        0%, 100% { opacity: 1; }
        50% { opacity: 0.5; }
      }

      /* Chapter Creation Modal */
      .apd-te-modal-overlay {
        position: fixed;
        inset: 0;
        z-index: 10000;
        background: rgba(0, 0, 0, 0.55);
        backdrop-filter: blur(4px);
        display: flex;
        align-items: center;
        justify-content: center;
        padding: 16px;
        animation: apdTeFadeIn 0.15s ease-out;
      }
      .apd-te-modal-card {
        width: 100%;
        max-width: 520px;
        background: hsl(var(--card, 38 28% 97%));
        border: 1px solid hsl(var(--border, 36 16% 87%));
        border-radius: 12px;
        box-shadow: 0 20px 40px -8px rgba(0, 0, 0, 0.3);
        overflow: hidden;
      }
      @keyframes apdTeFadeIn {
        from { opacity: 0; transform: scale(0.98); }
        to { opacity: 1; transform: scale(1); }
      }
      
      @media print {
        body * { visibility: hidden; }
        #apd-te-doc-sheet, #apd-te-doc-sheet * { visibility: visible; }
        #apd-te-doc-sheet {
          position: absolute;
          left: 0;
          top: 0;
          width: 100% !important;
          max-width: 100% !important;
          box-shadow: none !important;
          border: none !important;
          padding: 0 !important;
        }
      }
    `;
    document.head.appendChild(style);
  }

  window.ThesisStyles = {
    inject: injectThesisStyles
  };
})();

