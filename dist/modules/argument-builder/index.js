/**
 * ArchaeoPhD — Argument Builder Module Controller
 * 
 * Orchestrates Store, View, AI Co-Arguer, and Modal System.
 */

(function () {
  'use strict';

  var ArgumentController = {
    activeQuestionFilter: 'all',
    claims: [],
    evidence: [],
    sources: [],
    hypotheses: [],
    questions: [],

    init: function () {
      this.attachNavigationListener();
    },

    attachNavigationListener: function () {
      var self = this;
      function checkPath() {
        var path = window.location.pathname;
        if (path === '/argument-builder' || path.endsWith('/argument-builder') || window.location.hash === '#/argument-builder') {
          self.mountIfPresent();
        }
      }
      window.addEventListener('popstate', checkPath);
      setInterval(checkPath, 600);
    },

    mountIfPresent: function () {
      var root = document.getElementById('root');
      if (!root) return;
      var existingStudio = document.getElementById('apd-argument-studio-mount');
      if (existingStudio && existingStudio.children.length > 0) return;

      var pageTitle = document.querySelector('h1, h2, h3');
      var isArgPage = window.location.pathname.indexOf('/argument-builder') !== -1 || (pageTitle && pageTitle.innerText.indexOf('Argument') !== -1);
      if (!isArgPage) return;

      this.loadData();
    },

    loadData: async function () {
      var projectId = window.APD_PROJECT ? window.APD_PROJECT.getActiveId() : 'default';
      this.questions = window.ArgumentStore ? window.ArgumentStore.getQuestions(projectId) : [];
      this.hypotheses = window.ArgumentStore ? window.ArgumentStore.getHypotheses(projectId) : [];

      if (window.APD_BRIDGE) {
        try { this.claims = await window.APD_BRIDGE.getClaims(projectId); } catch(e) {}
        try { this.evidence = await window.APD_BRIDGE.getEvidence(projectId); } catch(e) {}
        try { this.sources = await window.APD_BRIDGE.getSources(projectId); } catch(e) {}
      }
      this.render();
    },

    render: function () {
      var container = document.getElementById('apd-argument-studio-mount');
      if (!container) {
        container = document.createElement('div');
        container.id = 'apd-argument-studio-mount';

        var root = document.getElementById('root');
        var mainContent = root ? root.querySelector('main, .p-6, .space-y-6, div > div') : null;
        if (mainContent) {
          mainContent.innerHTML = '';
          mainContent.appendChild(container);
        } else if (root) {
          root.innerHTML = '';
          root.appendChild(container);
        }
      }

      container.innerHTML = window.DialecticView.render(this);
      this.bindEvents(container);
    },

    bindEvents: function (container) {
      var self = this;
      var projectId = window.APD_PROJECT ? window.APD_PROJECT.getActiveId() : 'default';

      // Research question filter
      var filterBtns = container.querySelectorAll('[data-rq-filter]');
      filterBtns.forEach(function (btn) {
        btn.onclick = function () {
          self.activeQuestionFilter = decodeURIComponent(btn.getAttribute('data-rq-filter'));
          self.render();
        };
      });

      // New Hypothesis button
      var newHypBtn = container.querySelector('#apd-arg-btn-new-hyp');
      var emptyAddBtn = container.querySelector('#apd-arg-btn-empty-add');
      if (newHypBtn) newHypBtn.onclick = function () { self.showAddHypothesisDialog(projectId); };
      if (emptyAddBtn) emptyAddBtn.onclick = function () { self.showAddHypothesisDialog(projectId); };

      // New Question button
      var newRqBtn = container.querySelector('#apd-arg-btn-new-rq');
      if (newRqBtn) newRqBtn.onclick = function () { self.showAddQuestionDialog(projectId); };

      // Delete Hypothesis
      var deleteHypBtns = container.querySelectorAll('[data-delete-hyp]');
      deleteHypBtns.forEach(function (btn) {
        btn.onclick = function () {
          var hId = decodeURIComponent(btn.getAttribute('data-delete-hyp'));
          if (window.confirm('Are you sure you want to delete this hypothesis?')) {
            self.hypotheses = window.ArgumentStore.deleteHypothesis(projectId, hId);
            self.render();
          }
        };
      });

      // Change Status
      var statusSelects = container.querySelectorAll('[data-hyp-status]');
      statusSelects.forEach(function (sel) {
        sel.onchange = function () {
          var hId = decodeURIComponent(sel.getAttribute('data-hyp-status'));
          window.ArgumentStore.updateHypothesis(projectId, hId, { status: sel.value });
        };
      });

      // Link Claim
      var linkSelects = container.querySelectorAll('[data-link-claim-hyp]');
      linkSelects.forEach(function (sel) {
        sel.onchange = function () {
          var cId = sel.value;
          if (!cId) return;
          var hId = decodeURIComponent(sel.getAttribute('data-link-claim-hyp'));
          window.ArgumentStore.linkClaim(projectId, hId, cId);
          self.hypotheses = window.ArgumentStore.getHypotheses(projectId);
          self.render();
        };
      });

      // Unlink Claim
      var unlinkBtns = container.querySelectorAll('[data-unlink-claim-hyp]');
      unlinkBtns.forEach(function (btn) {
        btn.onclick = function () {
          var hId = decodeURIComponent(btn.getAttribute('data-unlink-claim-hyp'));
          var cId = decodeURIComponent(btn.getAttribute('data-unlink-claim-id'));
          window.ArgumentStore.unlinkClaim(projectId, hId, cId);
          self.hypotheses = window.ArgumentStore.getHypotheses(projectId);
          self.render();
        };
      });

      // Surface Counterarguments (Local AI)
      var aiBtns = container.querySelectorAll('[data-ai-counterarg]');
      aiBtns.forEach(function (btn) {
        btn.onclick = async function () {
          var hId = decodeURIComponent(btn.getAttribute('data-ai-counterarg'));
          var hyp = self.hypotheses.find(function(h) { return h.id === hId; });
          if (!hyp) return;

          var boxId = hId.replace(/[^a-zA-Z0-9_-]/g, '_');
          var box = document.getElementById('apd-arg-ai-box-' + boxId);
          if (!box) return;

          var contentEl = box.querySelector('.apd-arg-ai-content');
          box.classList.remove('hidden');
          if (contentEl) contentEl.innerText = 'Consulting local offline neural model (Qwen 2.5 7B) across ingested monographs...';

          var answer = await window.AiCoArguer.generateCounterarguments(hyp.text, self.sources, projectId);
          if (contentEl) contentEl.innerText = answer;
        };
      });

      // Close AI Output box
      var closeAiBtns = container.querySelectorAll('[data-close-ai-box]');
      closeAiBtns.forEach(function (btn) {
        btn.onclick = function () {
          var boxId = btn.getAttribute('data-close-ai-box');
          var box = document.getElementById('apd-arg-ai-box-' + boxId);
          if (box) box.classList.add('hidden');
        };
      });

      // Export to Thesis Studio
      var exportThesisBtns = container.querySelectorAll('[data-export-thesis-hyp]');
      exportThesisBtns.forEach(function (btn) {
        btn.onclick = function () {
          var hId = decodeURIComponent(btn.getAttribute('data-export-thesis-hyp'));
          self.showExportToThesisDialog(hId, projectId);
        };
      });
    },

    showAddHypothesisDialog: function (projectId) {
      var self = this;
      var icons = window.APD_ICONS || {};

      window.APD_MODAL.show({
        title: 'Add Working Hypothesis',
        subtitle: 'Formulate a testable thesis claim for your dissertation',
        icon: icons.workflow,
        confirmText: '+ Save Hypothesis',
        contentHtml: `
          <div>
            <label for="apd-arg-input-hyp" class="block text-xs font-semibold text-foreground mb-1.5">Hypothesis Statement *</label>
            <textarea id="apd-arg-input-hyp" rows="3" placeholder="e.g. City IV was destroyed c. 1400 BCE during Late Bronze I, corroborated by continuous domestic storage jar assemblages..." class="w-full p-2.5 rounded-lg border border-border bg-card text-xs text-foreground placeholder:text-muted-foreground focus:outline-none focus:ring-1 focus:ring-primary font-medium"></textarea>
          </div>
          <div>
            <label for="apd-arg-select-rq" class="block text-xs font-semibold text-foreground mb-1.5">Linked Research Question (Optional)</label>
            <select id="apd-arg-select-rq" class="w-full h-9 px-2.5 rounded-md border border-border bg-card text-xs text-foreground outline-none cursor-pointer">
              <option value="">No linked question</option>
              ${this.questions.map(function(q) { return `<option value="${q.id}">${q.question}</option>`; }).join('')}
            </select>
          </div>
          <div id="apd-arg-dialog-error" class="hidden text-xs text-destructive bg-destructive/10 border border-destructive/20 rounded-md px-2.5 py-1.5 font-medium"></div>
        `,
        onConfirm: function (overlay) {
          var text = (overlay.querySelector('#apd-arg-input-hyp').value || '').trim();
          var rqId = overlay.querySelector('#apd-arg-select-rq').value;
          var errEl = overlay.querySelector('#apd-arg-dialog-error');

          if (!text) {
            errEl.textContent = 'Please enter a hypothesis statement.';
            errEl.classList.remove('hidden');
            return false;
          }

          window.ArgumentStore.addHypothesis(projectId, { text: text, research_question_id: rqId });
          self.hypotheses = window.ArgumentStore.getHypotheses(projectId);
          self.render();
          return true;
        }
      });
    },

    showAddQuestionDialog: function (projectId) {
      var self = this;
      var icons = window.APD_ICONS || {};

      window.APD_MODAL.show({
        title: 'Add Research Question',
        subtitle: 'Define an overarching doctoral inquiry for your thesis',
        icon: icons.helpCircle,
        confirmText: '+ Save Question',
        contentHtml: `
          <div>
            <label for="apd-arg-input-rq" class="block text-xs font-semibold text-foreground mb-1.5">Research Question *</label>
            <input id="apd-arg-input-rq" type="text" placeholder="e.g. How does stratigraphic conflagration correlate with typo-chronology at Tell es-Sultan?" class="w-full h-9 px-3 rounded-lg border border-border bg-card text-xs text-foreground placeholder:text-muted-foreground focus:outline-none focus:ring-1 focus:ring-primary font-medium" />
          </div>
          <div id="apd-arg-rq-error" class="hidden text-xs text-destructive bg-destructive/10 border border-destructive/20 rounded-md px-2.5 py-1.5 font-medium"></div>
        `,
        onConfirm: function (overlay) {
          var text = (overlay.querySelector('#apd-arg-input-rq').value || '').trim();
          var errEl = overlay.querySelector('#apd-arg-rq-error');

          if (!text) {
            errEl.textContent = 'Please enter a research question.';
            errEl.classList.remove('hidden');
            return false;
          }

          window.ArgumentStore.addQuestion(projectId, text);
          self.questions = window.ArgumentStore.getQuestions(projectId);
          self.render();
          return true;
        }
      });
    },

    showExportToThesisDialog: function (hId, projectId) {
      var self = this;
      var hyp = this.hypotheses.find(function (h) { return h.id === hId; });
      if (!hyp) return;

      var chapters = [];
      try {
        var rawCh = localStorage.getItem('archaeophd_thesis_chapters_' + projectId);
        if (rawCh) chapters = JSON.parse(rawCh);
      } catch(e) {}
      if (!Array.isArray(chapters) || chapters.length === 0) {
        chapters = ['Introduction', 'Literature Review', 'Methodology', 'Chapter 1', 'Discussion', 'Conclusion'];
      }

      window.APD_MODAL.show({
        title: 'Insert Argument into Thesis',
        subtitle: 'Directly inject structured dialectic outline into your dissertation draft',
        icon: window.APD_ICONS ? window.APD_ICONS.bookOpen : '',
        confirmText: 'Inject into Canvas',
        contentHtml: `
          <div class="p-3 rounded-lg bg-muted/40 border border-border text-xs space-y-1">
            <div class="font-bold text-foreground">Hypothesis to Insert:</div>
            <div class="text-muted-foreground font-serif italic">"${hyp.text}"</div>
          </div>
          <div>
            <label for="apd-arg-target-chapter" class="block text-xs font-semibold text-foreground mb-1.5">Target Thesis Chapter</label>
            <select id="apd-arg-target-chapter" class="w-full h-9 px-2.5 rounded-md border border-border bg-card text-xs text-foreground outline-none cursor-pointer">
              ${chapters.map(function(c) { return `<option value="${c}">${c}</option>`; }).join('')}
            </select>
          </div>
        `,
        onConfirm: function (overlay) {
          var targetCh = overlay.querySelector('#apd-arg-target-chapter').value;
          if (!targetCh) return;

          var linkedClaims = (self.claims || []).filter(function (c) {
            return (hyp.claim_ids || []).indexOf(c.id) !== -1;
          });

          var supportClaims = linkedClaims.filter(function (c) {
            return !(self.evidence || []).some(function (ev) {
              return ev.claim_id === c.id && ev.evidence_type === 'contradicting';
            });
          });

          var contradictClaims = linkedClaims.filter(function (c) {
            return (self.evidence || []).some(function (ev) {
              return ev.claim_id === c.id && ev.evidence_type === 'contradicting';
            });
          });

          var injectedBlock = `
            <hr/>
            <h2>Argument Dialectic: ${hyp.text}</h2>
            <p><strong>Thesis Statement / Working Hypothesis:</strong> <em>“${hyp.text}”</em> (Status: ${hyp.status.toUpperCase()})</p>
            <h3>Empirical Corroboration & Supporting Claims</h3>
            ${supportClaims.length === 0 ? '<p><em>No supporting claims compiled yet.</em></p>' : `
              <ul>
                ${supportClaims.map(function(sc) {
                  var ev = (self.evidence || []).filter(function(e) { return e.claim_id === sc.id; });
                  var evText = ev.map(function(e) { return e.evidence_text; }).join('; ');
                  return `<li><strong>Claim:</strong> ${sc.claim_text}${evText ? ` — <em>Evidence: ${evText}</em>` : ''}</li>`;
                }).join('')}
              </ul>
            `}
            <h3>Viva Voce Counterarguments & Competing Theories</h3>
            ${contradictClaims.length === 0 ? '<p><em>No contradictory counterarguments registered.</em></p>' : `
              <ul>
                ${contradictClaims.map(function(cc) {
                  var ev = (self.evidence || []).filter(function(e) { return e.claim_id === cc.id; });
                  var evText = ev.map(function(e) { return e.evidence_text; }).join('; ');
                  return `<li><strong>Counter-Interpretation:</strong> ${cc.claim_text}${evText ? ` — <em>Rival Citation: ${evText}</em>` : ''}</li>`;
                }).join('')}
              </ul>
            `}
            <p><strong>Examiner Defense Footnote:</strong> Further empirical soundings and calibrated chronometric envelopes are required to reconcile competing typological assertions.</p>
          `;

          var storageKey = 'archaeophd_thesis_draft_' + projectId + '_' + encodeURIComponent(targetCh);
          var existingDraft = localStorage.getItem(storageKey) || `<h1>${targetCh}</h1><p>Begin drafting...</p>`;
          localStorage.setItem(storageKey, existingDraft + '\n' + injectedBlock);

          window.APD_MODAL.toast('Argument injected into "' + targetCh + '"!');
          return true;
        }
      });
    }
  };

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', function () { ArgumentController.init(); });
  } else {
    ArgumentController.init();
  }

  window.archaeophdArgument = ArgumentController;
})();

