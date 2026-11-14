/**
 * ArchaeoPhD — Thesis Studio Controller & Lifecycle
 * 
 * Coordinates Chapter Matrix Workspace, Google Docs Ribbon Editor,
 * Live Writing Inspector, and Defense Audit views.
 */

(function () {
  'use strict';

  var ThesisStudio = {
    viewMode: 'workspace', // 'workspace' | 'chapter' | 'audit'
    activeChapter: 'Introduction',
    activeRightTab: 'ai',   // 'ai' | 'inspector' | 'claims' | 'sources'
    sources: [],
    claims: [],
    evidence: [],
    contradictions: [],
    artifacts: [],
    auditData: null,
    savedFont: 'garamond',
    savedFontSize: '12',
    savedSpacing: '1.5',

    init: function () {
      if (window.ThesisStyles) window.ThesisStyles.inject();
      this.attachNavigationListener();
    },

    attachNavigationListener: function () {
      var self = this;
      function checkPath() {
        var path = window.location.pathname;
        if (path === '/thesis' || path.endsWith('/thesis') || window.location.hash === '#/thesis') {
          self.mountIfPresent();
        }
      }
      window.addEventListener('popstate', checkPath);
      setInterval(checkPath, 600);
    },

    mountIfPresent: function () {
      var root = document.getElementById('root');
      if (!root) return;
      var existingStudio = document.getElementById('apd-thesis-studio-mount');
      if (existingStudio && existingStudio.children.length > 0) return;

      var pageTitle = document.querySelector('h1, h2, h3');
      var isThesisPage = window.location.pathname.indexOf('/thesis') !== -1 || (pageTitle && pageTitle.innerText.indexOf('Thesis') !== -1);
      if (!isThesisPage) return;

      this.loadProjectData();
    },

    loadProjectData: async function () {
      var projectId = window.APD_PROJECT ? window.APD_PROJECT.getActiveProjectId() : 'default';
      var bridge = window.nativeBridge;
      if (bridge && typeof bridge.call === 'function') {
        try { var s = await bridge.call('get_sources', {}, projectId); if (Array.isArray(s)) this.sources = s; } catch(e) {}
        try { var c = await bridge.call('get_claims', {}, projectId); if (Array.isArray(c)) this.claims = c; } catch(e) {}
        try { var ev = await bridge.call('get_evidence', {}, projectId); if (Array.isArray(ev)) this.evidence = ev; } catch(e) {}
        try { var con = await bridge.call('get_contradictions', {}, projectId); if (Array.isArray(con)) this.contradictions = con; } catch(e) {}
        try { var art = await bridge.call('get_artifacts', {}, projectId); if (Array.isArray(art)) this.artifacts = art; } catch(e) {}
        try { var a = await bridge.call('get_thesis_audit', {}, projectId); if (a) this.auditData = a; } catch(e) {}
      }
      this.render();
    },

    render: function () {
      var container = document.getElementById('apd-thesis-studio-mount');
      if (!container) {
        container = document.createElement('div');
        container.id = 'apd-thesis-studio-mount';
        container.className = 'apd-te-container space-y-4 p-2';

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

      var projectId = window.APD_PROJECT ? window.APD_PROJECT.getActiveProjectId() : 'default';
      var projectTitle = window.APD_PROJECT ? window.APD_PROJECT.getActiveProjectTitle() : 'Dissertation Workspace';
      var chapters = window.DraftStore ? window.DraftStore.getChapters(projectId) : ['Introduction'];

      if (this.viewMode === 'chapter') {
        container.innerHTML = this.renderGoogleDocsChapterStudio(chapters, projectId, projectTitle);
      } else if (this.viewMode === 'audit') {
        container.innerHTML = this.renderAuditHeader(projectTitle) + this.renderAuditView(chapters, projectId);
      } else {
        container.innerHTML = this.renderWorkspaceOverview(chapters, projectId, projectTitle);
      }

      this.bindEvents(container, projectId);
    },

    renderWorkspaceOverview: function (chapters, projectId, projectTitle) {
      var self = this;
      var ICONS = window.APD_ICONS || {};

      return `
        <div class="flex flex-col md:flex-row md:items-center justify-between gap-4 pb-4 border-b border-border">
          <div>
            <div class="flex items-center gap-2.5">
              <span class="p-2.5 rounded-xl bg-primary/10 border border-primary/20 text-primary">${ICONS.book || ''}</span>
              <div>
                <h1 class="text-2xl font-bold font-display text-foreground tracking-tight">Thesis Workspace & Chapters</h1>
                <p class="text-xs text-muted-foreground">${projectTitle} • Click any chapter to launch full academic word processor</p>
              </div>
            </div>
          </div>

          <div class="flex items-center gap-2 shrink-0 self-start md:self-center">
            <button id="apd-te-btn-new-chapter" class="inline-flex items-center gap-1.5 px-3 py-1.5 rounded-lg text-xs font-semibold bg-primary text-white hover:bg-primary/90 transition-all shadow-sm">
              ${ICONS.plus || ''} New Chapter
            </button>
            <div class="flex items-center gap-1.5 p-1 bg-muted/60 rounded-xl border border-border">
              <button id="apd-te-tab-workspace" class="apd-te-tab-btn flex items-center gap-1.5 px-3 py-1.5 rounded-lg text-xs font-semibold bg-card text-foreground shadow-sm border border-border font-bold">
                ${ICONS.layers || ''} All Chapters (${chapters.length})
              </button>
              <button id="apd-te-tab-audit" class="apd-te-tab-btn flex items-center gap-1.5 px-3 py-1.5 rounded-lg text-xs font-semibold text-muted-foreground hover:text-foreground">
                ${ICONS.shieldCheck || ''} Defense Audit
                ${this.auditData ? `<span class="px-1.5 py-0.2 rounded-full bg-primary/10 text-primary text-[10px] font-bold">${this.auditData.defense_readiness_score || 85}%</span>` : ''}
              </button>
            </div>
          </div>
        </div>

        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          ${chapters.map(function (ch, idx) {
            var cl = (self.claims || []).filter(function (c) {
              return (c.chapter || '').toLowerCase() === ch.toLowerCase();
            });
            var draft = window.DraftStore ? window.DraftStore.getDraft(projectId, ch) : '';
            var cleanText = draft.replace(/<[^>]*>/g, ' ').replace(/\s+/g, ' ').trim();
            var words = cleanText ? cleanText.split(/\s+/).filter(Boolean).length : 0;
            var pages = Math.max(1, Math.ceil(words / 250));
            var status = window.DraftStore ? window.DraftStore.getStatus(projectId, ch) : 'drafting';

            return `
              <div data-open-chapter="${encodeURIComponent(ch)}" class="rounded-xl border border-border bg-card p-4 flex flex-col justify-between space-y-3 shadow-sm hover:border-primary/50 transition-all cursor-pointer">
                <div>
                  <div class="flex items-start justify-between gap-3">
                    <div class="flex items-center gap-2.5">
                      <span class="w-7 h-7 rounded-lg bg-primary/10 border border-primary/20 flex items-center justify-center text-xs font-bold text-primary shrink-0">
                        ${idx + 1}
                      </span>
                      <div>
                        <h3 class="font-display text-base font-bold text-foreground hover:text-primary transition-colors">${ch}</h3>
                        <p class="text-xs text-muted-foreground mt-0.5">
                          ${cl.length} claim${cl.length !== 1 ? 's' : ''} • ${words} words (~${pages} page${pages !== 1 ? 's' : ''})
                        </p>
                      </div>
                    </div>

                    <div class="flex items-center gap-1.5 shrink-0">
                      <span class="text-[10px] font-semibold px-2 py-0.5 rounded-full ${status === 'ready' ? 'bg-accent/15 text-accent' : status === 'review' ? 'bg-chart-4/15 text-chart-4' : 'bg-muted text-muted-foreground'}">
                        ${status === 'ready' ? 'Defense Ready' : status === 'review' ? 'In Review' : 'Drafting'}
                      </span>
                      ${chapters.length > 1 ? `
                        <button data-delete-chapter="${encodeURIComponent(ch)}" class="text-muted-foreground/50 hover:text-destructive p-1 rounded-md hover:bg-destructive/10 transition-colors" title="Delete chapter">
                          ${ICONS.trash || ''}
                        </button>
                      ` : ''}
                    </div>
                  </div>
                </div>

                <div class="flex items-center justify-between pt-2 border-t border-border/60 text-xs">
                  <span class="text-[11px] text-muted-foreground flex items-center gap-1">
                    ${words > 0 ? `${ICONS.checkCircle || ''} ${words} words drafted` : `${ICONS.pen || ''} Empty chapter`}
                  </span>
                  <span class="text-xs font-semibold text-primary flex items-center gap-1">
                    Open Word Studio ${ICONS.chevronRight || ''}
                  </span>
                </div>
              </div>
            `;
          }).join('')}
        </div>
      `;
    },

    renderGoogleDocsChapterStudio: function (chapters, projectId, projectTitle) {
      var self = this;
      var ch = this.activeChapter;
      var rawDraft = window.DraftStore ? window.DraftStore.getDraft(projectId, ch) : '';
      var initialHtml = window.DraftStore ? window.DraftStore.textToHtml(rawDraft, ch) : '<p>Begin drafting...</p>';
      var status = window.DraftStore ? window.DraftStore.getStatus(projectId, ch) : 'drafting';
      var ICONS = window.APD_ICONS || {};
      var chapterClaims = (this.claims || []).filter(function (c) {
        return (c.chapter || '').toLowerCase() === ch.toLowerCase();
      });

      var ribbonHtml = window.FormattingRibbon ? window.FormattingRibbon.render({ savedFontSize: self.savedFontSize }) : '';
      var canvasHtml = window.DocumentCanvas ? window.DocumentCanvas.render({
        initialHtml: initialHtml,
        savedSpacing: self.savedSpacing,
        savedFont: self.savedFont,
        savedFontSize: self.savedFontSize
      }) : '';

      return `
        <!-- Top Toolbar Header -->
        <div class="flex flex-col md:flex-row md:items-center justify-between gap-3 pb-3 border-b border-border">
          <div class="flex items-center gap-2">
            <button id="apd-te-back-to-workspace" class="apd-gdocs-btn h-8 px-2.5 border border-border bg-card text-xs font-semibold flex items-center gap-1 hover:bg-muted" title="Back to All Chapters">
              ${ICONS.arrowLeft || ''} All Chapters
            </button>
            <div class="h-4 w-px bg-border mx-1"></div>
            <div>
              <h1 class="font-display text-lg font-bold text-foreground leading-none">${ch}</h1>
              <p class="text-[11px] text-muted-foreground mt-0.5">${projectTitle}</p>
            </div>
          </div>

          <div class="flex items-center gap-2">
            <button id="apd-te-trigger-inspector" class="apd-gdocs-btn px-2.5 h-7 border border-accent/40 bg-accent/5 text-xs font-semibold flex items-center gap-1.5 text-foreground">
              <span class="text-accent">${ICONS.sparkles || ''}</span>
              <span id="apd-te-live-status-badge">AI Proofreading Active</span>
            </button>

            <select id="apd-te-single-status" class="h-7 px-2 rounded-md bg-card border border-border text-xs font-medium text-foreground outline-none cursor-pointer">
              <option value="drafting" ${status === 'drafting' ? 'selected' : ''}>Status: Drafting</option>
              <option value="review" ${status === 'review' ? 'selected' : ''}>Status: In Review</option>
              <option value="ready" ${status === 'ready' ? 'selected' : ''}>Status: Defense Ready</option>
            </select>

            <div class="relative inline-block">
              <button id="apd-te-export-menu-btn" class="apd-gdocs-btn h-7 px-2.5 border border-border bg-card text-xs font-medium flex items-center gap-1.5">
                ${ICONS.download || ''} Export
              </button>
              <div id="apd-te-export-dropdown" class="hidden absolute right-0 mt-1 w-44 rounded-lg border border-border bg-card shadow-lg z-50 py-1 text-xs">
                <button id="apd-te-export-tex" class="w-full text-left px-3 py-1.5 hover:bg-muted flex items-center gap-2">
                  ${ICONS.fileText || ''} LaTeX (.tex)
                </button>
                <button id="apd-te-export-md" class="w-full text-left px-3 py-1.5 hover:bg-muted flex items-center gap-2">
                  ${ICONS.copy || ''} Markdown (.md)
                </button>
                <button id="apd-te-print-pdf" class="w-full text-left px-3 py-1.5 hover:bg-muted flex items-center gap-2">
                  ${ICONS.printer || ''} Print / PDF
                </button>
              </div>
            </div>
          </div>
        </div>

        <!-- 1-Click Chapter Switcher Ribbon -->
        <div class="flex items-center gap-1.5 overflow-x-auto pb-1 scrollbar-thin">
          <span class="text-[10px] font-bold uppercase tracking-wider text-muted-foreground shrink-0 mr-1">Switch:</span>
          ${chapters.map(function (c, idx) {
            var isSelected = c === ch;
            var cClaims = (self.claims || []).filter(function(clm) { return (clm.chapter || '').toLowerCase() === c.toLowerCase(); });
            return `
              <button data-switch-to="${encodeURIComponent(c)}" class="px-2.5 py-0.5 rounded-md text-xs whitespace-nowrap transition-all flex items-center gap-1.5 border shrink-0 ${isSelected ? 'bg-primary text-white border-primary shadow-sm font-bold' : 'bg-card text-muted-foreground border-border hover:bg-muted hover:text-foreground'}">
                <span class="text-[10px] opacity-75">${idx + 1}.</span>
                <span>${c}</span>
                ${cClaims.length > 0 ? `<span class="text-[9px] px-1 py-0.1 rounded-full ${isSelected ? 'bg-white/20 text-white' : 'bg-muted text-muted-foreground'}">${cClaims.length}</span>` : ''}
              </button>
            `;
          }).join('')}
        </div>

        <!-- Main Workspace -->
        <div class="grid grid-cols-1 lg:grid-cols-12 gap-4 items-start">
          <div class="lg:col-span-8 flex flex-col space-y-2">
            ${ribbonHtml}
            ${canvasHtml}
          </div>

          <!-- Right Intelligence Drawer -->
          <div class="lg:col-span-4 rounded-xl border border-primary/25 bg-card p-3 shadow-sm space-y-3">
            <div class="flex items-center justify-between pb-2 border-b border-border">
              <div class="flex items-center gap-1 p-0.5 bg-muted/60 rounded-lg border border-border">
                <button id="apd-te-right-tab-ai" class="px-2 py-0.5 rounded text-xs font-semibold transition-all ${this.activeRightTab === 'ai' ? 'bg-card text-foreground shadow-sm font-bold' : 'text-muted-foreground'}">
                  ${ICONS.sparkles || ''} Neural AI
                </button>
                <button id="apd-te-right-tab-inspector" class="px-2 py-0.5 rounded text-xs font-semibold transition-all ${this.activeRightTab === 'inspector' ? 'bg-card text-foreground shadow-sm font-bold' : 'text-muted-foreground'}">
                  ${ICONS.shieldCheck || ''} Inspector
                  <span id="apd-te-inspector-count" class="ml-1 px-1 py-0.1 rounded-full bg-accent/20 text-accent text-[9px] font-bold">0</span>
                </button>
                <button id="apd-te-right-tab-claims" class="px-2 py-0.5 rounded text-xs font-semibold transition-all ${this.activeRightTab === 'claims' ? 'bg-card text-foreground shadow-sm font-bold' : 'text-muted-foreground'}">
                  ${ICONS.bookmark || ''} Claims
                </button>
                <button id="apd-te-right-tab-sources" class="px-2 py-0.5 rounded text-xs font-semibold transition-all ${this.activeRightTab === 'sources' ? 'bg-card text-foreground shadow-sm font-bold' : 'text-muted-foreground'}">
                  ${ICONS.book || ''} Library
                </button>
              </div>
            </div>

            <!-- Tab 1: AI Co-Writer -->
            <div id="apd-te-tab-ai-pane" class="${this.activeRightTab === 'ai' ? '' : 'hidden'} space-y-3">
              <div class="p-2 rounded-lg bg-muted/40 border border-border text-[11px] flex items-center justify-between">
                <span class="text-muted-foreground text-[10px] uppercase font-bold">Library Grounding:</span>
                <div class="flex items-center gap-1.5 font-bold text-primary text-[10.5px]">
                  <span>${this.sources.length} Monographs</span>
                  <span>•</span>
                  <span>${chapterClaims.length} Claims</span>
                </div>
              </div>

              <div class="space-y-1.5 pt-1">
                <span class="text-[10px] font-bold uppercase tracking-wider text-muted-foreground">Custom Scholarly Inquiry:</span>
                <div class="flex gap-2">
                  <input id="apd-te-custom-query" type="text" placeholder="e.g. Synthesize Trench B locus with Kenyon radiocarbon…" class="text-xs h-8 px-2.5 rounded-md border border-border bg-transparent flex-1 outline-none focus:border-primary" />
                  <button id="apd-te-query-btn" class="h-8 px-3 rounded-md bg-primary text-white text-xs font-bold hover:bg-primary/90 flex items-center gap-1">
                    Generate
                  </button>
                </div>
              </div>

              <div id="apd-te-ai-output" class="hidden p-3 rounded-xl border border-primary/30 bg-primary/5 space-y-2">
                <div class="flex items-center justify-between text-xs pb-1 border-b border-primary/15">
                  <span class="font-bold text-primary text-[11px] flex items-center gap-1">
                    ${ICONS.sparkles || ''} Grounded Suggestion
                  </span>
                  <button id="apd-te-close-ai-output" class="text-muted-foreground hover:text-foreground">${ICONS.x || 'X'}</button>
                </div>
                <div id="apd-te-suggestion-body" class="text-xs font-serif leading-relaxed text-foreground/90 max-h-[200px] overflow-y-auto whitespace-pre-wrap p-2 rounded bg-card/80 border border-border"></div>
              </div>
            </div>

            <!-- Tab 2: Inspector -->
            <div id="apd-te-tab-inspector-pane" class="${this.activeRightTab === 'inspector' ? '' : 'hidden'} space-y-3">
              <div id="apd-te-inspector-list" class="space-y-2 max-h-[380px] overflow-y-auto pr-1">
                <div class="p-3 rounded-lg bg-muted/40 border border-border text-center text-xs text-muted-foreground">
                  Analyzing document text in real time...
                </div>
              </div>
            </div>

            <!-- Tab 3: Claims -->
            <div id="apd-te-tab-claims-pane" class="${this.activeRightTab === 'claims' ? '' : 'hidden'} space-y-2 max-h-[380px] overflow-y-auto">
              ${chapterClaims.length === 0 ? '<div class="p-3 text-xs text-muted-foreground text-center">No claims allocated to this chapter.</div>' : chapterClaims.map(function(c) {
                return '<div class="p-2 border rounded text-xs bg-card">' + (c.claim_text || '') + '</div>';
              }).join('')}
            </div>

            <!-- Tab 4: Library -->
            <div id="apd-te-tab-sources-pane" class="${this.activeRightTab === 'sources' ? '' : 'hidden'} space-y-2 max-h-[380px] overflow-y-auto">
              ${this.sources.length === 0 ? '<div class="p-3 text-xs text-muted-foreground text-center">No library monographs recorded yet.</div>' : this.sources.map(function(s) {
                return '<div class="p-2 border rounded text-xs bg-card"><strong>' + (s.author || 'Author') + ' (' + (s.year || 'n.d.') + ')</strong>: ' + (s.title || '') + '</div>';
              }).join('')}
            </div>
          </div>
        </div>
      `;
    },

    renderAuditHeader: function (projectTitle) {
      var ICONS = window.APD_ICONS || {};
      return `
        <div class="flex flex-col md:flex-row md:items-center justify-between gap-4 pb-4 border-b border-border">
          <div class="flex items-center gap-2.5">
            <span class="p-2.5 rounded-xl bg-primary/10 border border-primary/20 text-primary">${ICONS.shieldCheck || ''}</span>
            <div>
              <h1 class="text-2xl font-bold font-display text-foreground tracking-tight">Pre-Submission Defense Audit</h1>
              <p class="text-xs text-muted-foreground">${projectTitle}</p>
            </div>
          </div>
          <div class="flex items-center gap-1.5 p-1 bg-muted/60 rounded-xl border border-border">
            <button id="apd-te-tab-workspace" class="apd-te-tab-btn flex items-center gap-1.5 px-3 py-1.5 rounded-lg text-xs font-semibold text-muted-foreground hover:text-foreground">
              ${ICONS.layers || ''} All Chapters
            </button>
            <button id="apd-te-tab-audit" class="apd-te-tab-btn flex items-center gap-1.5 px-3 py-1.5 rounded-lg text-xs font-semibold bg-card text-foreground shadow-sm border border-border font-bold">
              ${ICONS.shieldCheck || ''} Defense Audit
            </button>
          </div>
        </div>
      `;
    },

    renderAuditView: function (chapters, projectId) {
      var score = (this.auditData && this.auditData.defense_readiness_score) || 85;
      var ICONS = window.APD_ICONS || {};
      return `
        <div class="space-y-6 pt-2">
          <div class="p-6 rounded-2xl border border-primary/20 bg-primary/5 flex flex-col md:flex-row md:items-center justify-between gap-6 shadow-sm">
            <div class="space-y-1.5">
              <span class="text-xs font-bold uppercase tracking-wider text-primary">Dissertation Defense Readiness</span>
              <h2 class="text-3xl font-bold font-display text-foreground">${score}% Readiness Rating</h2>
              <p class="text-xs text-muted-foreground max-w-xl">
                Synthesized across all project monographs, empirical claims, and excavation reports.
              </p>
            </div>
            <button id="apd-te-run-audit-btn" class="h-10 px-5 rounded-xl bg-primary text-white text-xs font-bold hover:bg-primary/90 flex items-center gap-2">
              ${ICONS.refresh || ''} Re-Analyze Dissertation Defense
            </button>
          </div>
        </div>
      `;
    },

    bindEvents: function (container, projectId) {
      var self = this;

      var tabWorkspace = container.querySelector('#apd-te-tab-workspace');
      var tabAudit = container.querySelector('#apd-te-tab-audit');
      if (tabWorkspace) tabWorkspace.onclick = function () { self.viewMode = 'workspace'; self.render(); };
      if (tabAudit) tabAudit.onclick = function () { self.viewMode = 'audit'; self.render(); };

      var backBtn = container.querySelector('#apd-te-back-to-workspace');
      if (backBtn) backBtn.onclick = function () { self.viewMode = 'workspace'; self.render(); };

      // Chapter openings
      container.querySelectorAll('[data-open-chapter]').forEach(function (card) {
        card.onclick = function () {
          self.activeChapter = decodeURIComponent(card.getAttribute('data-open-chapter'));
          self.viewMode = 'chapter';
          self.render();
        };
      });

      // Chapter switching from ribbon
      container.querySelectorAll('[data-switch-to]').forEach(function (btn) {
        btn.onclick = function () {
          self.activeChapter = decodeURIComponent(btn.getAttribute('data-switch-to'));
          self.render();
        };
      });

      // Chapter deletion
      container.querySelectorAll('[data-delete-chapter]').forEach(function (btn) {
        btn.onclick = function (e) {
          e.stopPropagation();
          var ch = decodeURIComponent(btn.getAttribute('data-delete-chapter'));
          if (confirm('Delete chapter "' + ch + '"?')) {
            var cur = window.DraftStore.getChapters(projectId);
            var updated = cur.filter(function (c) { return c !== ch; });
            window.DraftStore.saveChapters(projectId, updated);
            window.DraftStore.deleteDraft(projectId, ch);
            self.render();
          }
        };
      });

      // Editor sheet events
      var sheet = container.querySelector('#apd-te-doc-sheet');
      if (sheet) {
        var autosaveTimeout = null;
        var proofreadTimeout = null;

        function runAutosave() {
          if (window.DraftStore) {
            window.DraftStore.saveDraft(projectId, self.activeChapter, sheet.innerHTML);
          }
          if (window.DocumentCanvas) window.DocumentCanvas.updateStats(sheet);
        }

        sheet.addEventListener('input', function () {
          clearTimeout(autosaveTimeout);
          autosaveTimeout = setTimeout(runAutosave, 800);

          clearTimeout(proofreadTimeout);
          proofreadTimeout = setTimeout(function () {
            if (window.WritingInspector) {
              var fixes = window.WritingInspector.analyze(sheet.innerText, self.sources, self.contradictions);
              window.WritingInspector.renderInspectorPanel(fixes, sheet, runAutosave);
            }
          }, 1200);
        });

        if (window.FormattingRibbon) window.FormattingRibbon.attachEvents(sheet, runAutosave);
        if (window.DocumentCanvas) {
          window.DocumentCanvas.updateStats(sheet);
          window.DocumentCanvas.attachSelectionWatcher(sheet, function (action, text) {
            self.runSelectionAction(action, text, sheet, projectId);
          });
        }
        if (window.WritingInspector) {
          var initialFixes = window.WritingInspector.analyze(sheet.innerText, self.sources, self.contradictions);
          window.WritingInspector.renderInspectorPanel(initialFixes, sheet, runAutosave);
        }
      }

      // Exports
      var exportBtn = container.querySelector('#apd-te-export-menu-btn');
      var exportDropdown = container.querySelector('#apd-te-export-dropdown');
      if (exportBtn && exportDropdown) {
        exportBtn.onclick = function (e) {
          e.stopPropagation();
          exportDropdown.classList.toggle('hidden');
        };
        document.addEventListener('click', function () {
          exportDropdown.classList.add('hidden');
        });
      }

      var exportTex = container.querySelector('#apd-te-export-tex');
      if (exportTex) {
        exportTex.onclick = function () {
          if (window.ThesisExporters && sheet) {
            window.ThesisExporters.exportLatex(sheet.innerHTML, self.activeChapter);
          }
        };
      }
      var exportMd = container.querySelector('#apd-te-export-md');
      if (exportMd) {
        exportMd.onclick = function () {
          if (window.ThesisExporters && sheet) {
            window.ThesisExporters.exportMarkdown(sheet.innerHTML, self.activeChapter);
          }
        };
      }
      var printPdf = container.querySelector('#apd-te-print-pdf');
      if (printPdf) {
        printPdf.onclick = function () {
          window.print();
        };
      }

      // Tab switcher in intelligence drawer
      ['ai', 'inspector', 'claims', 'sources'].forEach(function (tab) {
        var tabBtn = container.querySelector('#apd-te-right-tab-' + tab);
        if (tabBtn) {
          tabBtn.onclick = function () {
            self.activeRightTab = tab;
            ['ai', 'inspector', 'claims', 'sources'].forEach(function (t) {
              var pane = container.querySelector('#apd-te-tab-' + t + '-pane');
              if (pane) pane.classList.toggle('hidden', t !== tab);
            });
          };
        }
      });
    },

    runSelectionAction: async function (action, text, sheet, projectId) {
      if (!text || !sheet) return;
      var prompt = "Polish and enhance this dissertation excerpt for academic publication: \"" + text + "\"";
      if (action === 'cite') prompt = "Recommend and format an inline academic citation for: \"" + text + "\"";
      else if (action === 'condense') prompt = "Condense into concise academic prose: \"" + text + "\"";

      var bridge = window.nativeBridge;
      if (bridge && typeof bridge.call === 'function') {
        try {
          var res = await bridge.call('invoke_llm', { prompt: prompt }, projectId);
          var rep = (res && res.answer) ? res.answer : (typeof res === 'string' ? res : '');
          if (rep) {
            document.execCommand('insertText', false, rep);
            if (window.DraftStore) window.DraftStore.saveDraft(projectId, this.activeChapter, sheet.innerHTML);
          }
        } catch(e) {
          console.error(e);
        }
      }
    }
  };

  window.archaeophdThesis = ThesisStudio;

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', function () { ThesisStudio.init(); });
  } else {
    ThesisStudio.init();
  }
})();

