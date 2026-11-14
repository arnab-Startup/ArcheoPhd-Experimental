/**
 * ArchaeoPhD — Thesis Studio Document Canvas
 * 
 * Manages the A4 paper canvas, inch ruler, word/char statistics,
 * selection floating AI menu, and Ctrl+J inline copilot.
 */

(function () {
  'use strict';

  var DocumentCanvas = {
    render: function (options) {
      var initialHtml = options.initialHtml || '<p>Begin drafting your archaeological analysis here...</p>';
      var savedSpacing = options.savedSpacing || '1.5';
      var savedFont = options.savedFont || 'garamond';
      var savedFontSize = options.savedFontSize || '12';
      var ICONS = window.APD_ICONS || {};

      return `
        <!-- Page Canvas: Authentic Ruler + A4 Centered Sheet -->
        <div class="apd-te-page-wrapper">
          
          <!-- Document Ruler Bar with Inch Markings -->
          <div class="apd-doc-ruler">
            <span class="apd-ruler-tick" style="left: 0%">0</span>
            <span class="apd-ruler-subtick" style="left: 7.14%"></span>
            <span class="apd-ruler-tick" style="left: 14.28%">1</span>
            <span class="apd-ruler-subtick" style="left: 21.42%"></span>
            <span class="apd-ruler-tick" style="left: 28.56%">2</span>
            <span class="apd-ruler-subtick" style="left: 35.7%"></span>
            <span class="apd-ruler-tick" style="left: 42.84%">3</span>
            <span class="apd-ruler-subtick" style="left: 49.98%"></span>
            <span class="apd-ruler-tick" style="left: 57.12%">4</span>
            <span class="apd-ruler-subtick" style="left: 64.26%"></span>
            <span class="apd-ruler-tick" style="left: 71.4%">5</span>
            <span class="apd-ruler-subtick" style="left: 78.54%"></span>
            <span class="apd-ruler-tick" style="left: 85.68%">6</span>
            <span class="apd-ruler-subtick" style="left: 92.82%"></span>
            <span class="apd-ruler-tick" style="left: 99.5%">7</span>
          </div>

          <!-- The Pristine Academic Sheet -->
          <div id="apd-te-doc-sheet" contenteditable="true" spellcheck="true" class="apd-te-page-sheet" data-spacing="${savedSpacing}" data-font="${savedFont}" data-size="${savedFontSize}">
            ${initialHtml}
          </div>

        </div>

        <!-- Floating Selection Toolbar -->
        <div id="apd-te-floating-ai-bar" class="hidden">
          <button data-float-act="polish" class="apd-gdocs-btn text-xs font-semibold px-2 py-1 flex items-center gap-1">
            <span>${ICONS.sparkles || ''}</span> Polish
          </button>
          <button data-float-act="cite" class="apd-gdocs-btn text-xs font-semibold px-2 py-1 flex items-center gap-1">
            <span>${ICONS.book || ''}</span> Cite
          </button>
          <button data-float-act="condense" class="apd-gdocs-btn text-xs font-semibold px-2 py-1 flex items-center gap-1">
            <span>${ICONS.minus || ''}</span> Condense
          </button>
        </div>

        <!-- Bottom Word, Character, and Page Status Bar -->
        <div class="p-2 border rounded-lg bg-card border-border flex flex-wrap items-center justify-between text-xs text-muted-foreground gap-2 select-none shadow-sm">
          <div class="flex items-center gap-4">
            <span id="apd-te-word-stat" class="font-bold text-foreground">0 words</span>
            <span id="apd-te-char-stat">0 characters</span>
            <span id="apd-te-page-stat">~1 page</span>
            <span>A4 Standard Format</span>
          </div>
          <div class="flex items-center gap-2 text-[11px]">
            <span class="flex items-center gap-1 text-accent font-semibold">${ICONS.checkCircle || ''} Real-time autosave</span>
            <span>•</span>
            <span class="text-muted-foreground">Press Ctrl+J for inline AI</span>
          </div>
        </div>
      `;
    },

    updateStats: function (sheetEl) {
      if (!sheetEl) return;
      var text = sheetEl.innerText || '';
      var words = text.trim() ? text.trim().split(/\s+/).length : 0;
      var chars = text.length;
      var pages = Math.max(1, Math.ceil(words / 450));

      var wEl = document.getElementById('apd-te-word-stat');
      var cEl = document.getElementById('apd-te-char-stat');
      var pEl = document.getElementById('apd-te-page-stat');

      if (wEl) wEl.textContent = words + ' words';
      if (cEl) cEl.textContent = chars + ' characters';
      if (pEl) pEl.textContent = '~' + pages + ' ' + (pages === 1 ? 'page' : 'pages');
    },

    attachSelectionWatcher: function (sheetEl, onAiAction) {
      if (!sheetEl) return;
      var bar = document.getElementById('apd-te-floating-ai-bar');
      if (!bar) return;

      document.addEventListener('selectionchange', function () {
        var sel = window.getSelection();
        if (!sel || sel.isCollapsed || !sel.rangeCount) {
          bar.style.display = 'none';
          return;
        }

        var range = sel.getRangeAt(0);
        if (!sheetEl.contains(range.commonAncestorContainer)) {
          bar.style.display = 'none';
          return;
        }

        var rect = range.getBoundingClientRect();
        if (rect.width > 5) {
          bar.style.display = 'flex';
          bar.style.top = (window.scrollY + rect.top - 42) + 'px';
          bar.style.left = Math.max(10, (window.scrollX + rect.left + (rect.width / 2) - 100)) + 'px';
        } else {
          bar.style.display = 'none';
        }
      });

      bar.querySelectorAll('[data-float-act]').forEach(function (btn) {
        btn.onclick = function () {
          var act = btn.getAttribute('data-float-act');
          var sel = window.getSelection();
          var selectedText = sel ? sel.toString() : '';
          bar.style.display = 'none';
          if (typeof onAiAction === 'function') {
            onAiAction(act, selectedText);
          }
        };
      });

      // Ctrl+J inline Help Me Write trigger
      sheetEl.addEventListener('keydown', function (e) {
        if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'j') {
          e.preventDefault();
          var inlineBtn = document.getElementById('apd-te-inline-help-write-btn');
          if (inlineBtn) inlineBtn.click();
        }
      });
    }
  };

  window.DocumentCanvas = DocumentCanvas;
})();

