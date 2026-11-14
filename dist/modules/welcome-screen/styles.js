/**
 * ArchaeoPhD — Welcome Screen Stylesheet
 * 
 * Scoped styles for the welcome workstation portal.
 */

(function () {
  'use strict';

  function injectWelcomeStyles() {
    if (document.getElementById('apd-welcome-styles')) return;

    var style = document.createElement('style');
    style.id = 'apd-welcome-styles';
    style.textContent = `
      #apd-welcome-screen {
        position: fixed;
        inset: 0;
        z-index: 99999;
        display: flex;
        flex-direction: column;
        justify-content: space-between;
        background-color: var(--apd-obsidian-deep, #0b0807);
        background-image: 
          radial-gradient(ellipse at 50% 45%, rgba(16, 11, 9, 0.55) 0%, rgba(12, 8, 7, 0.78) 60%, rgba(8, 6, 5, 0.94) 100%),
          url('/assets/welcome-bg.jpg');
        background-size: cover;
        background-position: center bottom;
        background-repeat: no-repeat;
        color: var(--apd-parchment, #f5f0e8);
        font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
        overflow-y: auto;
        overflow-x: hidden;
        user-select: none;
        -webkit-user-select: none;
        transition: opacity 0.25s cubic-bezier(0.16, 1, 0.3, 1), visibility 0.25s ease;
      }

      #apd-welcome-screen.hidden {
        opacity: 0;
        visibility: hidden;
        pointer-events: none;
      }

      /* Top Title Bar */
      .apd-w-topbar {
        display: flex;
        align-items: center;
        justify-content: space-between;
        padding: 12px 32px;
        background: rgba(12, 8, 7, 0.65);
        backdrop-filter: blur(12px);
        border-bottom: 1px solid var(--apd-gold-border-muted, rgba(212, 163, 89, 0.16));
      }
      .apd-w-brand-pill {
        display: inline-flex;
        align-items: center;
        gap: 10px;
        font-size: 13.5px;
        letter-spacing: 0.02em;
        color: var(--apd-parchment, #f5f0e8);
      }
      .apd-w-brand-pill svg {
        width: 22px;
        height: 22px;
      }
      .apd-w-version {
        color: var(--apd-gold, #d4a359);
        font-size: 11.5px;
        font-weight: 600;
        margin-left: 6px;
        letter-spacing: 0.04em;
        opacity: 0.9;
      }
      .apd-w-skip-btn {
        display: inline-flex;
        align-items: center;
        gap: 6px;
        background: rgba(255, 255, 255, 0.06);
        border: 1px solid var(--apd-gold-border, rgba(212, 163, 89, 0.35));
        color: var(--apd-parchment, #f5f0e8);
        padding: 6px 16px;
        border-radius: 6px;
        font-size: 12px;
        font-weight: 500;
        cursor: pointer;
        transition: all 0.2s ease;
      }
      .apd-w-skip-btn:hover {
        background: var(--apd-gold-bg, rgba(212, 163, 89, 0.22));
        border-color: var(--apd-gold, #d4a359);
        color: var(--apd-parchment-white, #ffffff);
      }

      /* Center Hero Container */
      .apd-w-center {
        max-width: 1040px;
        width: 100%;
        margin: 0 auto;
        padding: 16px 24px;
        display: flex;
        flex-direction: column;
        align-items: center;
        text-align: center;
      }

      /* Hero Emblem */
      .apd-w-emblem-wrap {
        width: 68px;
        height: 68px;
        margin-bottom: 6px;
        filter: drop-shadow(0 6px 18px rgba(163, 51, 32, 0.5));
        transition: transform 0.3s cubic-bezier(0.16, 1, 0.3, 1);
        cursor: pointer;
      }
      .apd-w-emblem-wrap:hover {
        transform: scale(1.08);
      }

      /* Brand Titles */
      .apd-w-brand-title {
        font-family: Georgia, "Times New Roman", Garamond, serif;
        font-size: 34px;
        font-weight: 700;
        letter-spacing: 0.02em;
        color: #ffffff;
        margin: 0;
        line-height: 1.15;
      }
      .apd-w-brand-title span.gold-ai {
        background: linear-gradient(135deg, #fef08a 0%, #f59e0b 50%, #d97706 100%);
        -webkit-background-clip: text;
        -webkit-text-fill-color: transparent;
        font-style: italic;
        margin-left: 6px;
      }
      .apd-w-tagline {
        font-size: 10.5px;
        font-weight: 600;
        letter-spacing: 0.28em;
        text-transform: uppercase;
        color: #d4a359;
        margin-top: 5px;
        margin-bottom: 22px;
        opacity: 0.95;
      }

      /* Main Heading */
      .apd-w-main-heading {
        font-family: Georgia, "Times New Roman", serif;
        font-size: 40px;
        font-weight: 600;
        color: #ffffff;
        margin: 0 0 10px 0;
        text-shadow: 0 2px 14px rgba(0, 0, 0, 0.85);
        letter-spacing: 0.01em;
      }
      .apd-w-main-heading span.gold-ai-heading {
        background: linear-gradient(135deg, #fef08a 0%, #f59e0b 60%, #b45309 100%);
        -webkit-background-clip: text;
        -webkit-text-fill-color: transparent;
        font-style: italic;
        margin-left: 8px;
      }
      .apd-w-sub-desc {
        font-size: 15px;
        line-height: 1.5;
        color: #e5ded4;
        max-width: 640px;
        margin: 0 0 34px 0;
        text-shadow: 0 1px 6px rgba(0, 0, 0, 0.85);
      }

      /* Four Action Cards */
      .apd-w-cards-grid {
        display: grid;
        grid-template-columns: repeat(4, 1fr);
        gap: 16px;
        width: 100%;
        max-width: 980px;
        margin-bottom: 30px;
      }
      @media (max-width: 860px) {
        .apd-w-cards-grid {
          grid-template-columns: repeat(2, 1fr);
        }
      }

      .apd-w-card {
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
        text-align: center;
        padding: 24px 18px;
        border-radius: 14px;
        cursor: pointer;
        transition: all 0.25s cubic-bezier(0.16, 1, 0.3, 1);
        text-decoration: none;
        color: inherit;
        position: relative;
        overflow: hidden;
      }

      .apd-w-card-glass {
        background: rgba(22, 16, 14, 0.78);
        backdrop-filter: blur(14px);
        border: 1px solid var(--apd-gold-border, rgba(212, 163, 89, 0.24));
        box-shadow: 0 4px 20px rgba(0, 0, 0, 0.45);
      }
      .apd-w-card-glass:hover {
        background: var(--apd-obsidian-hover, rgba(36, 26, 23, 0.92));
        border-color: var(--apd-gold, #d4a359);
        transform: translateY(-3px);
        box-shadow: 0 8px 28px rgba(0, 0, 0, 0.6), 0 0 16px var(--apd-gold-glow, rgba(212, 163, 89, 0.25));
      }

      .apd-w-card-featured {
        background: linear-gradient(145deg, #7b1827 0%, #460d16 100%);
        border: 1px solid var(--apd-gold, rgba(245, 190, 100, 0.7));
        box-shadow: 0 8px 26px -2px rgba(123, 24, 39, 0.55), 0 0 18px var(--apd-gold-glow, rgba(212, 163, 89, 0.35));
      }
      .apd-w-card-featured:hover {
        background: linear-gradient(145deg, #911e2f 0%, #520f1a 100%);
        border-color: var(--apd-gold-hover, #fcd34d);
        transform: translateY(-4px) scale(1.02);
        box-shadow: 0 12px 34px rgba(123, 24, 39, 0.75), 0 0 24px var(--apd-gold-glow, rgba(245, 190, 100, 0.5));
      }

      .apd-w-card-icon-wrap {
        width: 40px;
        height: 40px;
        display: flex;
        align-items: center;
        justify-content: center;
        margin-bottom: 12px;
        color: var(--apd-gold-hover, #fef08a);
      }
      .apd-w-card-title {
        font-size: 16px;
        font-weight: 600;
        color: var(--apd-parchment-white, #ffffff);
        margin-bottom: 6px;
        letter-spacing: 0.01em;
      }
      .apd-w-card-desc {
        font-size: 12.5px;
        line-height: 1.4;
        color: var(--apd-parchment-subtle, #d1c7bc);
        opacity: 0.92;
      }
      .apd-w-card-featured .apd-w-card-desc {
        color: var(--apd-parchment, #fae8d4);
      }

      /* Feature Ribbon */
      .apd-w-ribbon-wrap {
        width: 100%;
        max-width: 980px;
        margin-bottom: 24px;
        position: relative;
        padding-top: 18px;
        border-top: 1px solid var(--apd-gold-border, rgba(212, 163, 89, 0.28));
      }
      .apd-w-ribbon-node {
        position: absolute;
        top: -5px;
        left: 50%;
        transform: translateX(-50%) rotate(45deg);
        width: 9px;
        height: 9px;
        background: var(--apd-gold, #d4a359);
        box-shadow: 0 0 8px var(--apd-gold, #d4a359);
      }
      .apd-w-ribbon-items {
        display: flex;
        align-items: center;
        justify-content: space-between;
        gap: 12px;
        flex-wrap: wrap;
      }
      .apd-w-ribbon-btn {
        display: flex;
        flex-direction: column;
        align-items: center;
        gap: 6px;
        padding: 8px 14px;
        border-radius: 8px;
        background: transparent;
        border: none;
        color: #e5ded5;
        cursor: pointer;
        transition: all 0.2s ease;
        text-decoration: none;
      }
      .apd-w-ribbon-btn:hover {
        background: rgba(255, 255, 255, 0.08);
        color: #fef08a;
        transform: translateY(-2px);
      }
      .apd-w-ribbon-btn svg {
        width: 22px;
        height: 22px;
        color: #d4a359;
        transition: transform 0.2s ease;
      }
      .apd-w-ribbon-btn:hover svg {
        transform: scale(1.12);
        color: #fef08a;
      }
      .apd-w-ribbon-btn span {
        font-size: 12px;
        font-weight: 500;
        letter-spacing: 0.02em;
      }

      /* Trust Badges Bar */
      .apd-w-trust-bar {
        display: flex;
        align-items: center;
        justify-content: center;
        gap: 40px;
        flex-wrap: wrap;
        padding: 14px 24px 26px 24px;
        color: #cfc6b8;
      }
      .apd-w-trust-item {
        display: flex;
        align-items: center;
        gap: 10px;
        font-size: 13px;
      }
      .apd-w-trust-item svg {
        width: 18px;
        height: 18px;
        color: #d4a359;
      }
      .apd-w-trust-title {
        font-weight: 600;
        color: #f5f0e8;
      }
      .apd-w-trust-sub {
        font-size: 11.5px;
        color: #a89f92;
        margin-left: 3px;
      }
    `;
    document.head.appendChild(style);
  }

  window.WelcomeStyles = {
    inject: injectWelcomeStyles
  };
})();

