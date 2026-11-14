/**
 * ArchaeoPhD — Dialectic View Renderer
 * 
 * Renders the two-column dialectic argument structure:
 * Left: Supporting Claims (Corroborating Case)
 * Right: Counterarguments & Disputed Claims (Viva Vulnerabilities)
 */

(function () {
  'use strict';

  var DialecticView = {
    render: function (state) {
      var icons = window.APD_ICONS || {};
      var projectTitle = window.APD_PROJECT ? window.APD_PROJECT.getActiveTitle() : 'Archaeological Project';

      var filteredHyps = state.hypotheses;
      if (state.activeQuestionFilter !== 'all') {
        filteredHyps = state.hypotheses.filter(function (h) {
          return h.research_question_id === state.activeQuestionFilter;
        });
      }

      return `
        <div class="apd-arg-container space-y-6 p-2">
          <!-- Top Header Ribbon -->
          <div class="flex flex-col md:flex-row md:items-center justify-between gap-4 pb-4 border-b border-border">
            <div>
              <div class="flex items-center gap-2.5">
                <span class="p-2.5 rounded-xl bg-primary/10 border border-primary/20 text-primary">
                  ${icons.workflow}
                </span>
                <div>
                  <h1 class="text-2xl font-bold font-display text-foreground tracking-tight">Argument Builder & Dialectic Reasoner</h1>
                  <p class="text-xs text-muted-foreground">${projectTitle} • Map testable hypotheses to supporting evidence vs. rival counterarguments</p>
                </div>
              </div>
            </div>

            <div class="flex items-center gap-2 shrink-0 self-start md:self-center">
              <button id="apd-arg-btn-new-rq" class="inline-flex items-center gap-1.5 px-3 py-1.5 rounded-lg text-xs font-semibold border border-border bg-card text-foreground hover:bg-muted transition-all shadow-sm">
                ${icons.helpCircle} New Question
              </button>
              <button id="apd-arg-btn-new-hyp" class="inline-flex items-center gap-1.5 px-3.5 py-1.5 rounded-lg text-xs font-semibold bg-primary text-white hover:bg-primary/90 transition-all shadow-sm">
                ${icons.plus} New Hypothesis
              </button>
            </div>
          </div>

          <!-- Research Question Filter Switcher -->
          <div class="flex items-center gap-2 overflow-x-auto pb-1 scrollbar-thin">
            <span class="text-[10px] font-bold uppercase tracking-wider text-muted-foreground shrink-0 mr-1">Questions:</span>
            <button data-rq-filter="all" class="px-3 py-1 rounded-md text-xs whitespace-nowrap transition-all border shrink-0 ${state.activeQuestionFilter === 'all' ? 'bg-primary text-white border-primary shadow-sm font-bold' : 'bg-card text-muted-foreground border-border hover:bg-muted'}">
              All Hypotheses (${state.hypotheses.length})
            </button>
            ${state.questions.map(function (q) {
              var isSelected = state.activeQuestionFilter === q.id;
              var count = state.hypotheses.filter(function(h) { return h.research_question_id === q.id; }).length;
              return `
                <button data-rq-filter="${encodeURIComponent(q.id)}" class="px-3 py-1 rounded-md text-xs whitespace-nowrap transition-all border shrink-0 flex items-center gap-1.5 ${isSelected ? 'bg-primary text-white border-primary shadow-sm font-bold' : 'bg-card text-muted-foreground border-border hover:bg-muted'}">
                  <span class="truncate max-w-[240px]">${q.question}</span>
                  <span class="text-[10px] opacity-75">(${count})</span>
                </button>
              `;
            }).join('')}
          </div>

          <!-- Hypotheses List -->
          ${filteredHyps.length === 0 ? `
            <div class="rounded-xl border border-dashed border-border bg-card/40 p-12 text-center space-y-3">
              <div class="w-12 h-12 rounded-full bg-primary/10 border border-primary/20 text-primary flex items-center justify-center mx-auto">
                ${icons.workflow}
              </div>
              <div>
                <h3 class="font-display text-base font-bold text-foreground">No Hypotheses Found</h3>
                <p class="text-xs text-muted-foreground mt-1 max-w-md mx-auto">Formulate a working hypothesis for your dissertation, then attach supporting claims and rival counterarguments.</p>
              </div>
              <button id="apd-arg-btn-empty-add" class="inline-flex items-center gap-1.5 px-4 py-2 rounded-lg bg-primary text-white text-xs font-semibold hover:bg-primary/90 transition-all shadow-sm">
                ${icons.plus} Formulate First Hypothesis
              </button>
            </div>
          ` : `
            <div class="space-y-6">
              ${filteredHyps.map(function (h) {
                return DialecticView.renderCard(h, state);
              }).join('')}
            </div>
          `}
        </div>
      `;
    },

    renderCard: function (h, state) {
      var icons = window.APD_ICONS || {};
      var rq = state.questions.find(function(q) { return q.id === h.research_question_id; });
      var linkedClaims = (state.claims || []).filter(function (c) {
        return (h.claim_ids || []).indexOf(c.id) !== -1;
      });

      var supportClaims = linkedClaims.filter(function (c) {
        return !(state.evidence || []).some(function (ev) {
          return ev.claim_id === c.id && ev.evidence_type === 'contradicting';
        });
      });

      var contradictClaims = linkedClaims.filter(function (c) {
        return (state.evidence || []).some(function (ev) {
          return ev.claim_id === c.id && ev.evidence_type === 'contradicting';
        });
      });

      var unlinkedProjectClaims = (state.claims || []).filter(function (c) {
        return (h.claim_ids || []).indexOf(c.id) === -1;
      });

      return `
        <div class="rounded-xl border border-border bg-card p-5 space-y-4 shadow-sm hover:border-border/80 transition-all">
          <!-- Hypothesis Card Header -->
          <div class="flex items-start justify-between gap-3">
            <div class="space-y-1 flex-1">
              ${rq ? `
                <div class="flex items-center gap-1.5 text-xs text-primary font-medium">
                  ${icons.helpCircle}
                  <span>${rq.question}</span>
                </div>
              ` : ''}
              <div class="flex items-center gap-2">
                <span class="text-[10px] font-bold uppercase tracking-wider text-muted-foreground">Hypothesis</span>
                <span class="text-muted-foreground/60">${icons.arrowDown}</span>
              </div>
              <h2 class="font-display text-lg font-bold text-foreground leading-snug">
                ${h.text}
              </h2>
            </div>

            <div class="flex items-center gap-2 shrink-0">
              <select data-hyp-status="${encodeURIComponent(h.id)}" class="h-7 px-2 rounded-md bg-muted/60 border border-border text-xs font-semibold text-foreground outline-none cursor-pointer">
                <option value="unresolved" ${h.status === 'unresolved' ? 'selected' : ''}>Status: Unresolved</option>
                <option value="supported" ${h.status === 'supported' ? 'selected' : ''}>Status: Supported</option>
                <option value="contested" ${h.status === 'contested' ? 'selected' : ''}>Status: Contested</option>
                <option value="refuted" ${h.status === 'refuted' ? 'selected' : ''}>Status: Refuted</option>
              </select>

              <button data-delete-hyp="${encodeURIComponent(h.id)}" class="text-muted-foreground hover:text-destructive p-1.5 rounded-md hover:bg-destructive/10 transition-colors" title="Delete hypothesis">
                ${icons.trash}
              </button>
            </div>
          </div>

          <!-- Two-Column Dialectic Split Layout -->
          <div class="grid grid-cols-1 md:grid-cols-2 gap-4 pt-2">
            
            <!-- LEFT: SUPPORTING CLAIMS (Green Theme) -->
            <div class="rounded-lg border border-accent/30 bg-accent/5 p-3.5 space-y-3">
              <div class="flex items-center justify-between pb-1.5 border-b border-accent/20">
                <span class="text-xs font-bold text-accent uppercase tracking-wider flex items-center gap-1.5">
                  ${icons.shieldCheck} Supporting Claims (${supportClaims.length})
                </span>
                <span class="text-[10px] font-semibold text-accent/80">Corroborating Case</span>
              </div>

              ${supportClaims.length === 0 ? `
                <p class="text-xs text-muted-foreground italic py-3 text-center">
                  No supporting claims linked yet. Connect claims below.
                </p>
              ` : `
                <div class="space-y-2.5">
                  ${supportClaims.map(function (c) {
                    return DialecticView.renderClaimCard(c, h.id, state, 'support');
                  }).join('')}
                </div>
              `}
            </div>

            <!-- RIGHT: COUNTERARGUMENTS & RIVALS (Red/Amber Theme) -->
            <div class="rounded-lg border border-destructive/30 bg-destructive/5 p-3.5 space-y-3">
              <div class="flex items-center justify-between pb-1.5 border-b border-destructive/20">
                <span class="text-xs font-bold text-destructive uppercase tracking-wider flex items-center gap-1.5">
                  ${icons.alertTriangle} Counterarguments & Rivals (${contradictClaims.length})
                </span>
                <span class="text-[10px] font-semibold text-destructive/80">Viva Vulnerabilities</span>
              </div>

              ${contradictClaims.length === 0 ? `
                <p class="text-xs text-muted-foreground italic py-3 text-center">
                  Zero contradictory claims detected. Click "Surface Counterarguments" to examine rival scholarship.
                </p>
              ` : `
                <div class="space-y-2.5">
                  ${contradictClaims.map(function (c) {
                    return DialecticView.renderClaimCard(c, h.id, state, 'contradict');
                  }).join('')}
                </div>
              `}
            </div>

          </div>

          <!-- Bottom Action Bar: Link Claim, AI Counterargument, Export to Thesis -->
          <div class="flex flex-col sm:flex-row items-center justify-between gap-3 pt-3 border-t border-border/80 text-xs">
            
            <div class="flex items-center gap-2 w-full sm:w-auto flex-1 max-w-md">
              <select data-link-claim-hyp="${encodeURIComponent(h.id)}" class="w-full h-8 px-2.5 rounded-md border border-border bg-card text-xs text-foreground outline-none cursor-pointer">
                <option value="">+ Connect Registered Claim to this Hypothesis...</option>
                ${unlinkedProjectClaims.map(function (c) {
                  return `
                    <option value="${c.id}">
                      ${c.claim_text.length > 70 ? c.claim_text.slice(0, 70) + '...' : c.claim_text}
                    </option>
                  `;
                }).join('')}
              </select>
            </div>

            <div class="flex items-center gap-2 self-end sm:self-auto shrink-0">
              <button data-ai-counterarg="${encodeURIComponent(h.id)}" class="h-8 px-2.5 rounded-md border border-primary/30 bg-primary/5 text-primary hover:bg-primary/10 font-semibold text-xs flex items-center gap-1.5 transition-all">
                ${icons.sparkles} Surface Counterarguments
              </button>

              <button data-export-thesis-hyp="${encodeURIComponent(h.id)}" class="h-8 px-3 rounded-md bg-foreground text-background hover:bg-foreground/90 font-semibold text-xs flex items-center gap-1.5 transition-all shadow-xs">
                ${icons.bookOpen} Insert into Thesis
              </button>
            </div>

          </div>

          <!-- AI Counterargument Output Shelf -->
          <div id="apd-arg-ai-box-${h.id.replace(/[^a-zA-Z0-9_-]/g, '_')}" class="hidden p-4 rounded-lg border border-primary/20 bg-primary/5 space-y-2 text-xs">
            <div class="flex items-center justify-between font-bold text-foreground">
              <span class="flex items-center gap-1.5 text-primary">
                ${icons.sparkles} Grounded Viva Examination Counterargument Analysis
              </span>
              <button data-close-ai-box="${h.id.replace(/[^a-zA-Z0-9_-]/g, '_')}" class="text-muted-foreground hover:text-foreground">
                ${icons.x}
              </button>
            </div>
            <div class="apd-arg-ai-content font-serif text-foreground/90 leading-relaxed whitespace-pre-wrap"></div>
          </div>

        </div>
      `;
    },

    renderClaimCard: function (c, hypId, state, type) {
      var icons = window.APD_ICONS || {};
      var isSupport = type === 'support';
      var borderClass = isSupport ? 'border-accent/20' : 'border-destructive/20';
      var arrowColor = isSupport ? 'text-accent' : 'text-destructive';
      var evList = (state.evidence || []).filter(function(e) { return e.claim_id === c.id; });

      return `
        <div class="rounded-md border ${borderClass} bg-card p-3 space-y-1.5 text-xs shadow-xs">
          <div class="flex items-start justify-between gap-2">
            <div class="flex items-start gap-1.5 flex-1">
              <span class="${arrowColor} mt-0.5 shrink-0">${icons.arrowRight}</span>
              <span class="font-semibold text-foreground leading-snug">${c.claim_text}</span>
            </div>
            <button data-unlink-claim-hyp="${encodeURIComponent(hypId)}" data-unlink-claim-id="${encodeURIComponent(c.id)}" class="text-muted-foreground hover:text-destructive p-0.5" title="Unlink claim">
              ${icons.x}
            </button>
          </div>

          ${evList.length > 0 ? `
            <div class="ml-4 pl-2 border-l border-border/80 space-y-1 mt-1">
              ${evList.map(function(ev) {
                var srcNames = (ev.source_ids || []).map(function(sid) {
                  var src = state.sources.find(function(s) { return s.id === sid; });
                  return src ? (src.author ? src.author + ' (' + (src.year || 'n.d.') + ')' : src.title) : 'Ingested Monograph';
                }).join(', ');

                return `
                  <div class="text-[11px] text-muted-foreground">
                    <div>${ev.evidence_text}</div>
                    ${srcNames ? `<div class="text-[10px] ${arrowColor} flex items-center gap-1 font-medium mt-0.5">${icons.fileText} ${srcNames}</div>` : ''}
                  </div>
                `;
              }).join('')}
            </div>
          ` : ''}
        </div>
      `;
    }
  };

  window.DialecticView = DialecticView;
})();

