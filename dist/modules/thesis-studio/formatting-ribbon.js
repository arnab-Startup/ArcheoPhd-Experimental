/**
 * ArchaeoPhD — Thesis Studio Formatting Ribbon
 * 
 * Google Docs / MS Word style grouped toolbar, typographic font picker,
 * size stepper, alignment controls, table & plate insertion.
 */

(function () {
  'use strict';

  var FormattingRibbon = {
    render: function (options) {
      var ICONS = window.APD_ICONS || {};
      var savedFontSize = options.savedFontSize || '12';

      return `
        <!-- GOOGLE DOCS / MICROSOFT WORD STYLE ICON TOOLBAR RIBBON -->
        <div class="apd-gdocs-ribbon p-1.5 flex flex-wrap items-center gap-1 text-xs select-none sticky top-0 z-20">
          
          <!-- Undo / Redo / Print -->
          <button data-doc-cmd="undo" class="apd-gdocs-btn" title="Undo (Ctrl+Z)">${ICONS.undo || ''}</button>
          <button data-doc-cmd="redo" class="apd-gdocs-btn" title="Redo (Ctrl+Y)">${ICONS.redo || ''}</button>
          <button id="apd-te-ribbon-print" class="apd-gdocs-btn" title="Print (Ctrl+P)">${ICONS.printer || ''}</button>
          <div class="h-4 w-px bg-border mx-0.5"></div>

          <!-- Paragraph Hierarchy (Normal, H1, H2, H3, Blockquote) -->
          <select id="apd-te-format-block" class="h-7 bg-card border border-border text-[11px] rounded px-2 text-foreground outline-none cursor-pointer">
            <option value="p">Normal text</option>
            <option value="h1">Heading 1 (Chapter)</option>
            <option value="h2">Heading 2 (Section)</option>
            <option value="h3">Heading 3 (Sub-section)</option>
            <option value="blockquote">Blockquote (Excerpt)</option>
          </select>

          <!-- 5 Authentic Typefaces / Font Styles -->
          <select id="apd-te-font-family" class="h-7 bg-card border border-border text-[11px] rounded px-2 text-foreground outline-none cursor-pointer font-serif">
            <option value="garamond">Garamond (Academic)</option>
            <option value="times">Times New Roman (Standard)</option>
            <option value="georgia">Georgia (Scholarly)</option>
            <option value="modern">Computer Modern (LaTeX)</option>
            <option value="sans">Modern Sans (Clean)</option>
            <option value="mono">JetBrains Mono (Data)</option>
          </select>

          <!-- Font Size Stepper (- 12 +) -->
          <div class="flex items-center border border-border rounded h-7 bg-card">
            <button id="apd-te-font-size-dec" class="px-1.5 h-full hover:bg-muted text-muted-foreground hover:text-foreground">${ICONS.minus || '-'}</button>
            <span id="apd-te-font-size-val" class="px-1 text-[11.5px] font-bold text-foreground min-w-[20px] text-center">${savedFontSize}</span>
            <button id="apd-te-font-size-inc" class="px-1.5 h-full hover:bg-muted text-muted-foreground hover:text-foreground">${ICONS.plus || '+'}</button>
          </div>
          <div class="h-4 w-px bg-border mx-0.5"></div>

          <!-- Basic Formatting: Bold, Italic, Underline, Strikethrough -->
          <button data-doc-cmd="bold" class="apd-gdocs-btn font-bold" title="Bold (Ctrl+B)">${ICONS.bold || ''}</button>
          <button data-doc-cmd="italic" class="apd-gdocs-btn" title="Italic (Ctrl+I)">${ICONS.italic || ''}</button>
          <button data-doc-cmd="underline" class="apd-gdocs-btn" title="Underline (Ctrl+U)">${ICONS.underline || ''}</button>
          <button data-doc-cmd="strikeThrough" class="apd-gdocs-btn" title="Strikethrough">${ICONS.strikethrough || ''}</button>
          <button data-doc-cmd="superscript" class="apd-gdocs-btn" title="Superscript / Footnote">${ICONS.superscript || ''}</button>
          <button data-doc-cmd="subscript" class="apd-gdocs-btn" title="Subscript">${ICONS.subscript || ''}</button>
          <div class="h-4 w-px bg-border mx-0.5"></div>

          <!-- Text Color Palette Dropdown -->
          <div class="relative inline-block">
            <button id="apd-te-text-color-btn" class="apd-gdocs-btn flex items-center gap-0.5" title="Text Color">
              ${ICONS.textColor || ''} ${ICONS.chevronDown || ''}
            </button>
            <div id="apd-te-text-color-picker" class="hidden absolute top-full left-0 mt-1 p-2 rounded-lg border border-border bg-card shadow-lg z-50 grid grid-cols-4 gap-1.5">
              <div data-color="#1a1a1a" class="w-5 h-5 rounded cursor-pointer border border-border" style="background: #1a1a1a" title="Black"></div>
              <div data-color="#8c2d19" class="w-5 h-5 rounded cursor-pointer" style="background: #8c2d19" title="Terracotta"></div>
              <div data-color="#1e40af" class="w-5 h-5 rounded cursor-pointer" style="background: #1e40af" title="Navy"></div>
              <div data-color="#047857" class="w-5 h-5 rounded cursor-pointer" style="background: #047857" title="Forest"></div>
              <div data-color="#b45309" class="w-5 h-5 rounded cursor-pointer" style="background: #b45309" title="Amber"></div>
              <div data-color="#6b21a8" class="w-5 h-5 rounded cursor-pointer" style="background: #6b21a8" title="Purple"></div>
              <div data-color="#be123c" class="w-5 h-5 rounded cursor-pointer" style="background: #be123c" title="Crimson"></div>
              <div data-color="#4b5563" class="w-5 h-5 rounded cursor-pointer" style="background: #4b5563" title="Muted Slate"></div>
            </div>
          </div>

          <!-- Highlighter Color Palette Dropdown -->
          <div class="relative inline-block">
            <button id="apd-te-highlight-btn" class="apd-gdocs-btn flex items-center gap-0.5" title="Highlight Color">
              ${ICONS.highlighter || ''} ${ICONS.chevronDown || ''}
            </button>
            <div id="apd-te-highlight-picker" class="hidden absolute top-full left-0 mt-1 p-2 rounded-lg border border-border bg-card shadow-lg z-50 grid grid-cols-4 gap-1.5">
              <div data-bg="#fef08a" class="w-5 h-5 rounded cursor-pointer border border-border" style="background: #fef08a" title="Yellow"></div>
              <div data-bg="#bbf7d0" class="w-5 h-5 rounded cursor-pointer" style="background: #bbf7d0" title="Green"></div>
              <div data-bg="#bae6fd" class="w-5 h-5 rounded cursor-pointer" style="background: #bae6fd" title="Cyan"></div>
              <div data-bg="#fbcfe8" class="w-5 h-5 rounded cursor-pointer" style="background: #fbcfe8" title="Pink"></div>
              <div data-bg="#fed7aa" class="w-5 h-5 rounded cursor-pointer" style="background: #fed7aa" title="Orange"></div>
              <div data-bg="#e9d5ff" class="w-5 h-5 rounded cursor-pointer" style="background: #e9d5ff" title="Lavender"></div>
              <div data-bg="transparent" class="w-5 h-5 rounded cursor-pointer border border-border flex items-center justify-center text-[10px] text-muted-foreground" title="None">X</div>
            </div>
          </div>
          <div class="h-4 w-px bg-border mx-0.5"></div>

          <!-- Alignment: Left, Center, Right, Justify -->
          <button data-doc-cmd="justifyLeft" class="apd-gdocs-btn" title="Align Left">${ICONS.alignLeft || ''}</button>
          <button data-doc-cmd="justifyCenter" class="apd-gdocs-btn" title="Align Center">${ICONS.alignCenter || ''}</button>
          <button data-doc-cmd="justifyRight" class="apd-gdocs-btn" title="Align Right">${ICONS.alignRight || ''}</button>
          <button data-doc-cmd="justifyFull" class="apd-gdocs-btn active" title="Justify (Academic standard)">${ICONS.alignJustify || ''}</button>
          
          <!-- Line Spacing Dropdown -->
          <select id="apd-te-line-spacing" class="h-7 bg-card border border-border text-[11px] rounded px-1.5 text-foreground outline-none cursor-pointer">
            <option value="1.5">1.5 Spacing</option>
            <option value="2.0">2.0 Double</option>
            <option value="1.15">1.15 Spacing</option>
            <option value="1.0">1.0 Single</option>
          </select>
          <div class="h-4 w-px bg-border mx-0.5"></div>

          <!-- Lists, Indent, Outdent -->
          <button data-doc-cmd="insertUnorderedList" class="apd-gdocs-btn" title="Bullet List">${ICONS.list || ''}</button>
          <button data-doc-cmd="insertOrderedList" class="apd-gdocs-btn" title="Numbered List">${ICONS.listOrdered || ''}</button>
          <button id="apd-te-outdent-btn" class="apd-gdocs-btn" title="Decrease Indent">${ICONS.outdent || ''}</button>
          <button id="apd-te-indent-btn" class="apd-gdocs-btn" title="Increase Indent">${ICONS.indent || ''}</button>
          <div class="h-4 w-px bg-border mx-0.5"></div>

          <!-- Insert Image / Plate / Table / Divider / Clear Formatting -->
          <button id="apd-te-insert-img-btn" class="apd-gdocs-btn" title="Insert Image / Archaeological Plate">${ICONS.image || ''}</button>
          <input type="file" id="apd-te-hidden-file-inp" accept="image/*" class="hidden" />
          <button id="apd-te-insert-table-btn" class="apd-gdocs-btn" title="Insert Archaeological Data Table">${ICONS.table || ''}</button>
          <button id="apd-te-insert-hr-btn" class="apd-gdocs-btn" title="Insert Horizontal Page Divider">${ICONS.horizontalRule || ''}</button>
          <button data-doc-cmd="removeFormat" class="apd-gdocs-btn" title="Clear Formatting">${ICONS.clearFormatting || ''}</button>

          <!-- Inline Help Me Write Pill -->
          <button id="apd-te-inline-help-write-btn" class="px-2.5 h-7 rounded bg-primary text-white text-xs font-semibold hover:bg-primary/90 flex items-center gap-1 ml-auto shadow-sm" title="Invoke Inline AI Copilot (Ctrl+J)">
            ${ICONS.sparkles || ''} Help Me Write
          </button>
        </div>
      `;
    },

    attachEvents: function (sheetEl, onStateChange) {
      if (!sheetEl) return;

      // Basic execCommands
      document.querySelectorAll('[data-doc-cmd]').forEach(function (btn) {
        btn.onclick = function () {
          var cmd = btn.getAttribute('data-doc-cmd');
          document.execCommand(cmd, false, null);
          sheetEl.focus();
          if (onStateChange) onStateChange();
        };
      });

      // Paragraph format blocks
      var formatBlockSelect = document.getElementById('apd-te-format-block');
      if (formatBlockSelect) {
        formatBlockSelect.onchange = function () {
          var tag = formatBlockSelect.value;
          document.execCommand('formatBlock', false, tag === 'p' ? '<p>' : '<' + tag + '>');
          sheetEl.focus();
          if (onStateChange) onStateChange();
        };
      }

      // Font Family
      var fontFamilySelect = document.getElementById('apd-te-font-family');
      if (fontFamilySelect) {
        fontFamilySelect.onchange = function () {
          sheetEl.setAttribute('data-font', fontFamilySelect.value);
          if (onStateChange) onStateChange();
        };
      }

      // Font Size Stepper
      var sizeVal = document.getElementById('apd-te-font-size-val');
      var sizeInc = document.getElementById('apd-te-font-size-inc');
      var sizeDec = document.getElementById('apd-te-font-size-dec');
      if (sizeInc && sizeDec && sizeVal) {
        sizeInc.onclick = function () {
          var cur = parseInt(sizeVal.textContent, 10) || 12;
          if (cur < 24) {
            cur += 1;
            sizeVal.textContent = cur;
            sheetEl.setAttribute('data-size', String(cur));
            if (onStateChange) onStateChange();
          }
        };
        sizeDec.onclick = function () {
          var cur = parseInt(sizeVal.textContent, 10) || 12;
          if (cur > 9) {
            cur -= 1;
            sizeVal.textContent = cur;
            sheetEl.setAttribute('data-size', String(cur));
            if (onStateChange) onStateChange();
          }
        };
      }

      // Line Spacing
      var lineSpacingSelect = document.getElementById('apd-te-line-spacing');
      if (lineSpacingSelect) {
        lineSpacingSelect.onchange = function () {
          sheetEl.setAttribute('data-spacing', lineSpacingSelect.value);
          if (onStateChange) onStateChange();
        };
      }

      // Indent & Outdent
      var indentBtn = document.getElementById('apd-te-indent-btn');
      var outdentBtn = document.getElementById('apd-te-outdent-btn');
      if (indentBtn) indentBtn.onclick = function () { document.execCommand('indent', false, null); };
      if (outdentBtn) outdentBtn.onclick = function () { document.execCommand('outdent', false, null); };

      // Horizontal Rule
      var hrBtn = document.getElementById('apd-te-insert-hr-btn');
      if (hrBtn) hrBtn.onclick = function () { document.execCommand('insertHorizontalRule', false, null); };

      // Color pickers toggling
      var colorBtn = document.getElementById('apd-te-text-color-btn');
      var colorPicker = document.getElementById('apd-te-text-color-picker');
      if (colorBtn && colorPicker) {
        colorBtn.onclick = function (e) {
          e.stopPropagation();
          colorPicker.classList.toggle('hidden');
        };
        colorPicker.querySelectorAll('[data-color]').forEach(function (sq) {
          sq.onclick = function () {
            document.execCommand('foreColor', false, sq.getAttribute('data-color'));
            colorPicker.classList.add('hidden');
          };
        });
      }

      var hlBtn = document.getElementById('apd-te-highlight-btn');
      var hlPicker = document.getElementById('apd-te-highlight-picker');
      if (hlBtn && hlPicker) {
        hlBtn.onclick = function (e) {
          e.stopPropagation();
          hlPicker.classList.toggle('hidden');
        };
        hlPicker.querySelectorAll('[data-bg]').forEach(function (sq) {
          sq.onclick = function () {
            document.execCommand('hiliteColor', false, sq.getAttribute('data-bg'));
            hlPicker.classList.add('hidden');
          };
        });
      }

      document.addEventListener('click', function () {
        if (colorPicker) colorPicker.classList.add('hidden');
        if (hlPicker) hlPicker.classList.add('hidden');
      });

      // Insert Table
      var tableBtn = document.getElementById('apd-te-insert-table-btn');
      if (tableBtn) {
        tableBtn.onclick = function () {
          var rows = prompt('Number of rows:', '3');
          var cols = prompt('Number of columns:', '3');
          var rCount = parseInt(rows, 10);
          var cCount = parseInt(cols, 10);
          if (rCount > 0 && cCount > 0) {
            var html = '<table class="apd-table"><thead><tr>';
            for (var c = 0; c < cCount; c++) {
              html += '<th>Header ' + (c + 1) + '</th>';
            }
            html += '</tr></thead><tbody>';
            for (var r = 0; r < rCount - 1; r++) {
              html += '<tr>';
              for (var c2 = 0; c2 < cCount; c2++) {
                html += '<td>Data</td>';
              }
              html += '</tr>';
            }
            html += '</tbody></table><p></p>';
            document.execCommand('insertHTML', false, html);
          }
        };
      }

      // Insert Image / Archaeological Plate
      var imgBtn = document.getElementById('apd-te-insert-img-btn');
      var hiddenFile = document.getElementById('apd-te-hidden-file-inp');
      if (imgBtn && hiddenFile) {
        imgBtn.onclick = function () { hiddenFile.click(); };
        hiddenFile.onchange = function () {
          if (hiddenFile.files && hiddenFile.files[0]) {
            var reader = new FileReader();
            reader.onload = function (e) {
              var caption = prompt('Figure Caption (e.g. Plate IV: Stratigraphic Section Trench B):', 'Plate I: Archaeological Artifact');
              var html = '<figure class="apd-doc-figure">' +
                '<img src="' + e.target.result + '" alt="' + (caption || 'Plate') + '" />' +
                '<figcaption contenteditable="true">' + (caption || 'Archaeological Plate') + '</figcaption>' +
                '</figure><p></p>';
              document.execCommand('insertHTML', false, html);
            };
            reader.readAsDataURL(hiddenFile.files[0]);
          }
        };
      }
    }
  };

  window.FormattingRibbon = FormattingRibbon;
})();

