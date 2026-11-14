/**
 * ArchaeoPhD — Thesis Studio Writing Inspector & Side Intelligence
 * 
 * Provides live academic tone checking, chronometric citation rules,
 * stratigraphic contradiction scanning, and monograph shelf integration.
 */

(function () {
  'use strict';

  var WritingInspector = {
    analyze: function (text, sources, contradictions) {
      var fixes = [];
      if (!text) return fixes;

      var phrasingRules = [
        { bad: /\ba lot of\b/gi, fix: "a substantial corpus of", reason: "Informal quantifier in academic writing" },
        { bad: /\bbasically\b/gi, fix: "fundamentally", reason: "Colloquial filler word" },
        { bad: /\bproves that\b/gi, fix: "strongly substantiates that", reason: "Archaeological inference is empirical, rarely absolute proof" },
        { bad: /\bwe can see that\b/gi, fix: "the stratigraphic record demonstrates that", reason: "Prefer objective third-person analysis" },
        { bad: /\bobviously\b/gi, fix: "evidently", reason: "Avoid dismissive assertion" }
      ];

      phrasingRules.forEach(function (rule) {
        var match;
        while ((match = rule.bad.exec(text)) !== null) {
          fixes.push({
            type: 'style',
            matched: match[0],
            replacement: rule.fix,
            reason: rule.reason
          });
          if (fixes.length > 8) break;
        }
      });

      var yearMatch = text.match(/\b(1[0-9]{3}|20[0-2][0-9])\s*(BCE|BC|CE|AD)\b/gi);
      if (yearMatch && text.indexOf('(') === -1 && text.indexOf(')') === -1) {
        fixes.push({
          type: 'citation',
          matched: yearMatch[0],
          replacement: sources && sources.length > 0 ? ' (' + (sources[0].author || 'Author') + ' ' + (sources[0].year || 'n.d.') + ')' : ' (Citation required)',
          reason: "Absolute chronometric dates require authoritative monograph citations"
        });
      }

      (contradictions || []).forEach(function (con) {
        var aInDoc = text.toLowerCase().indexOf((con.claim_a || '').slice(0, 20).toLowerCase()) !== -1;
        var bInDoc = text.toLowerCase().indexOf((con.claim_b || '').slice(0, 20).toLowerCase()) !== -1;
        if (aInDoc || bInDoc) {
          fixes.push({
            type: 'contradiction',
            matched: con.type || 'Stratigraphic Dispute',
            replacement: ' (See stratigraphic caveat: ' + (con.claim_b || con.claim_a) + ')',
            reason: 'Active project contradiction: ' + (con.anomaly_description || 'Disputed layer sequence')
          });
        }
      });

      return fixes;
    },

    renderInspectorPanel: function (fixes, sheetEl, onAutosave) {
      var ICONS = window.APD_ICONS || {};
      var listEl = document.getElementById('apd-te-inspector-list');
      var badge = document.getElementById('apd-te-live-status-badge');
      var countEl = document.getElementById('apd-te-inspector-count');

      if (badge) {
        badge.innerText = fixes.length === 0 ? 'Document Verified' : (fixes.length + ' Suggestions Available');
      }
      if (countEl) countEl.innerText = fixes.length;

      if (!listEl) return;

      if (fixes.length === 0) {
        listEl.innerHTML = `
          <div class="p-3.5 rounded-lg bg-accent/5 border border-accent/20 text-center text-xs text-foreground/80 space-y-1">
            <span class="text-accent font-bold">${ICONS.checkCircle || ''} Clean Scholarly Prose</span>
            <p class="text-[11px] text-muted-foreground">Zero colloquialisms, unreferenced dates, or unhandled stratigraphic disputes detected.</p>
          </div>
        `;
        return;
      }

      listEl.innerHTML = fixes.map(function (f, idx) {
        return `
          <div class="p-2.5 rounded-lg border border-border bg-card text-xs space-y-1.5 shadow-sm">
            <div class="flex items-center justify-between">
              <span class="text-[10px] font-bold uppercase tracking-wider px-1.5 py-0.2 rounded ${f.type === 'contradiction' ? 'bg-destructive/15 text-destructive' : f.type === 'citation' ? 'bg-accent/15 text-accent' : 'bg-chart-4/15 text-chart-4'}">
                ${f.type === 'contradiction' ? 'Dispute Risk' : f.type === 'citation' ? 'Missing Citation' : 'Academic Style'}
              </span>
              <button data-apply-fix="${idx}" class="px-2 py-0.5 rounded bg-primary text-white text-[10px] font-semibold hover:bg-primary/90 flex items-center gap-1">
                Apply Fix
              </button>
            </div>
            <div class="text-[11.5px] font-medium text-foreground">
              Replace <code class="bg-muted px-1 py-0.2 rounded text-destructive">${f.matched}</code> with <code class="bg-muted px-1 py-0.2 rounded text-accent">${f.replacement}</code>
            </div>
            <div class="text-[10.5px] text-muted-foreground leading-snug">
              ${f.reason}
            </div>
          </div>
        `;
      }).join('');

      listEl.querySelectorAll('[data-apply-fix]').forEach(function (btn) {
        btn.onclick = function () {
          var fIdx = parseInt(btn.getAttribute('data-apply-fix'), 10);
          var fix = fixes[fIdx];
          if (fix && sheetEl) {
            var curHtml = sheetEl.innerHTML;
            var re = new RegExp(fix.matched, 'i');
            sheetEl.innerHTML = curHtml.replace(re, fix.replacement);
            if (typeof onAutosave === 'function') onAutosave();
          }
        };
      });
    }
  };

  window.WritingInspector = WritingInspector;
})();

