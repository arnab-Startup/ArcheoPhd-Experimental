/**
 * ArchaeoPhD — Design System Colors & Theme Tokens
 * 
 * Central single source of truth for the ArchaeoPhD visual identity:
 * - Terracotta / Rust: Core academic action brand
 * - Antique Gold / Amber: Museum-grade illumination and highlights
 * - Obsidian / Basalt: Deep offline workstation surfaces
 * - Parchment: Scholarly typography & manuscript readability
 * - Status indicators: Epistemic validation, caveats, and warnings
 * 
 * Strictly zero emojis. 100% vector-ready tokens.
 */

(function () {
  'use strict';

  var RAW_COLORS = {
    // Primary Terracotta / Rust
    primary: '#9e4429',
    primaryDark: '#71321d',
    primaryBorder: '#b85435',
    primaryHover: '#b34d2f',
    primaryLight: 'rgba(158, 68, 41, 0.12)',

    // Antique Gold / Amber Accent
    gold: '#d4a359',
    goldHover: '#fef08a',
    goldBorder: 'rgba(212, 163, 89, 0.3)',
    goldBorderMuted: 'rgba(212, 163, 89, 0.18)',
    goldBg: 'rgba(212, 163, 89, 0.12)',
    goldGlow: 'rgba(212, 163, 89, 0.25)',

    // Obsidian / Basalt Dark Workstation
    obsidianDeep: '#0b0807',
    obsidianCard: '#171311',
    obsidianHeader: '#201a17',
    obsidianFooter: '#1c1714',
    obsidianInput: '#0d0a08',
    obsidianHover: '#241a17',
    backdrop: 'rgba(10, 8, 7, 0.78)',
    backdropLight: 'rgba(15, 12, 10, 0.72)',

    // Parchment & Typography
    parchment: '#f5f0e8',
    parchmentMuted: '#a89f91',
    parchmentSubtle: '#d1c7bc',
    parchmentWhite: '#ffffff',
    charcoalText: '#1a1a1a',

    // Status & Epistemic Feedback
    success: '#10b981',
    successBg: 'rgba(16, 185, 129, 0.12)',
    successBorder: 'rgba(16, 185, 129, 0.3)',

    warning: '#fbbf24',
    warningBg: 'rgba(245, 158, 11, 0.1)',
    warningBorder: 'rgba(245, 158, 11, 0.35)',

    danger: '#ef4444',
    dangerLight: '#f87171',
    dangerBg: 'rgba(239, 68, 68, 0.12)',
    dangerBorder: 'rgba(220, 38, 38, 0.35)'
  };

  // Inject CSS Custom Properties into document head
  function injectColorVariables() {
    if (document.getElementById('apd-color-variables')) return;

    var style = document.createElement('style');
    style.id = 'apd-color-variables';
    style.textContent = `
      :root {
        /* Primary Terracotta */
        --apd-primary: ${RAW_COLORS.primary};
        --apd-primary-dark: ${RAW_COLORS.primaryDark};
        --apd-primary-border: ${RAW_COLORS.primaryBorder};
        --apd-primary-hover: ${RAW_COLORS.primaryHover};
        --apd-primary-light: ${RAW_COLORS.primaryLight};

        /* Antique Gold / Amber */
        --apd-gold: ${RAW_COLORS.gold};
        --apd-gold-hover: ${RAW_COLORS.goldHover};
        --apd-gold-border: ${RAW_COLORS.goldBorder};
        --apd-gold-border-muted: ${RAW_COLORS.goldBorderMuted};
        --apd-gold-bg: ${RAW_COLORS.goldBg};
        --apd-gold-glow: ${RAW_COLORS.goldGlow};

        /* Obsidian Workstation */
        --apd-obsidian-deep: ${RAW_COLORS.obsidianDeep};
        --apd-obsidian-card: ${RAW_COLORS.obsidianCard};
        --apd-obsidian-header: ${RAW_COLORS.obsidianHeader};
        --apd-obsidian-footer: ${RAW_COLORS.obsidianFooter};
        --apd-obsidian-input: ${RAW_COLORS.obsidianInput};
        --apd-obsidian-hover: ${RAW_COLORS.obsidianHover};
        --apd-backdrop: ${RAW_COLORS.backdrop};
        --apd-backdrop-light: ${RAW_COLORS.backdropLight};

        /* Parchment & Typography */
        --apd-parchment: ${RAW_COLORS.parchment};
        --apd-parchment-muted: ${RAW_COLORS.parchmentMuted};
        --apd-parchment-subtle: ${RAW_COLORS.parchmentSubtle};
        --apd-parchment-white: ${RAW_COLORS.parchmentWhite};
        --apd-charcoal-text: ${RAW_COLORS.charcoalText};

        /* Epistemic Status */
        --apd-success: ${RAW_COLORS.success};
        --apd-success-bg: ${RAW_COLORS.successBg};
        --apd-success-border: ${RAW_COLORS.successBorder};

        --apd-warning: ${RAW_COLORS.warning};
        --apd-warning-bg: ${RAW_COLORS.warningBg};
        --apd-warning-border: ${RAW_COLORS.warningBorder};

        --apd-danger: ${RAW_COLORS.danger};
        --apd-danger-light: ${RAW_COLORS.dangerLight};
        --apd-danger-bg: ${RAW_COLORS.dangerBg};
        --apd-danger-border: ${RAW_COLORS.dangerBorder};
      }
    `;
    document.head.appendChild(style);
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', injectColorVariables);
  } else {
    injectColorVariables();
  }

  // Export typed accessors
  window.APD_COLORS = {
    hex: RAW_COLORS,
    css: {
      primary: 'var(--apd-primary)',
      primaryDark: 'var(--apd-primary-dark)',
      primaryBorder: 'var(--apd-primary-border)',
      primaryLight: 'var(--apd-primary-light)',

      gold: 'var(--apd-gold)',
      goldHover: 'var(--apd-gold-hover)',
      goldBorder: 'var(--apd-gold-border)',
      goldBg: 'var(--apd-gold-bg)',

      obsidianDeep: 'var(--apd-obsidian-deep)',
      obsidianCard: 'var(--apd-obsidian-card)',
      obsidianHeader: 'var(--apd-obsidian-header)',
      obsidianFooter: 'var(--apd-obsidian-footer)',
      obsidianInput: 'var(--apd-obsidian-input)',
      backdrop: 'var(--apd-backdrop)',

      parchment: 'var(--apd-parchment)',
      parchmentMuted: 'var(--apd-parchment-muted)',
      parchmentSubtle: 'var(--apd-parchment-subtle)',

      success: 'var(--apd-success)',
      warning: 'var(--apd-warning)',
      danger: 'var(--apd-danger)'
    },
    inject: injectColorVariables
  };
})();

