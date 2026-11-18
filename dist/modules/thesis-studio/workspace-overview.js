/**
 * ArchaeoPhD — Thesis Studio Workspace Overview & Project Gate
 * 
 * Renders the Chapter Matrix Workspace and the No-Project Selection Guard.
 */

(function () {
  'use strict';

  var WorkspaceOverview = {
    renderNoProjectPrompt: function (container) {
      var projs = [];
      try {
        projs = JSON.parse(localStorage.getItem('archaeophd_local_projects') || '[]');
        if (!Array.isArray(projs)) projs = [];
      } catch (e) {}
      var realProjs = projs.filter(function (p) { return p && p.id && p.id !== 'default' && p.id !== 'null'; });

      var html = '<div style="display:flex;flex-direction:column;align-items:center;justify-content:center;min-height:55vh;padding:40px 20px;text-align:center;font-family:system-ui,-apple-system,sans-serif;">' +
        '<div style="width:56px;height:56px;border-radius:16px;background:rgba(147,43,26,0.08);color:#932B1A;display:grid;place-items:center;margin-bottom:16px;">' +
          '<svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M4 19.5A2.5 2.5 0 0 1 6.5 17H20"></path><path d="M6.5 2H20v20H6.5A2.5 2.5 0 0 1 4 19.5v-15A2.5 2.5 0 0 1 6.5 2z"></path></svg>' +
        '</div>' +
        '<h2 style="font-family:Georgia,serif;font-size:22px;font-weight:600;color:#2D2620;margin:0 0 8px 0;">Doctoral Thesis Studio</h2>' +
        '<p style="font-size:13px;color:#786E65;max-width:480px;line-height:1.5;margin:0 0 24px 0;">Doctoral monographs and chapter drafts are maintained within dedicated research projects. Select an active research project to open its thesis workspace:</p>';

      if (realProjs.length > 0) {
        html += '<div style="display:flex;flex-direction:column;gap:10px;width:100%;max-width:440px;margin-bottom:24px;">';
        realProjs.forEach(function (p) {
          var title = (p.title || p.name || 'Archaeological Project').replace(/"/g, '&quot;');
          var sub = (p.research_period || p.research_region || 'Excavation Unit').replace(/"/g, '&quot;');
          html += '<button onclick="(function(){ try{ localStorage.setItem(\'archaeo_active_project\',\'' + p.id + '\'); }catch(e){} if(typeof window.__appNavigate===\'function\'){ window.__appNavigate(\'/thesis\'); } else { window.location.reload(); } })()" style="display:flex;align-items:center;justify-content:space-between;padding:12px 16px;border-radius:10px;background:#FFFFFF;border:1px solid #E8E2D9;cursor:pointer;text-align:left;transition:all 0.15s ease;" onmouseover="this.style.borderColor=\'#932B1A\';this.style.background=\'#FAF8F5\';" onmouseout="this.style.borderColor=\'#E8E2D9\';this.style.background=\'#FFFFFF\';">' +
            '<div style="flex:1;min-width:0;">' +
              '<div style="font-size:14px;font-weight:600;color:#2D2620;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;">' + title + '</div>' +
              '<div style="font-size:11px;color:#8C8273;margin-top:2px;">' + sub + '</div>' +
            '</div>' +
            '<span style="font-size:12px;font-weight:600;color:#932B1A;margin-left:12px;">Open Thesis &rarr;</span>' +
          '</button>';
        });
        html += '</div>';
      }

      html += '<div style="display:flex;gap:12px;">' +
        '<button onclick="(function(){ if(typeof window.__appNavigate===\'function\'){ window.__appNavigate(\'/projects/new\'); } else { window.location.href=\'/projects/new\'; } })()" style="padding:10px 18px;border-radius:8px;background:#932B1A;color:#fff;border:none;font-size:13px;font-weight:600;cursor:pointer;">New Research Project</button>' +
        '<button onclick="(function(){ if(typeof window.__appNavigate===\'function\'){ window.__appNavigate(\'/projects\'); } else { window.location.href=\'/projects\'; } })()" style="padding:10px 18px;border-radius:8px;background:#EDE8DF;color:#4A3F35;border:1px solid #D5CBBF;font-size:13px;font-weight:600;cursor:pointer;">Browse All Projects</button>' +
      '</div></div>';

      container.innerHTML = html;
    },

    renderOverview: function (chapters, projectId, projectTitle, claims, auditData) {
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
                ${auditData ? `<span class="px-1.5 py-0.2 rounded-full bg-primary/10 text-primary text-[10px] font-bold">${auditData.defense_readiness_score || 85}%</span>` : ''}
              </button>
            </div>
          </div>
        </div>

        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          ${chapters.map(function (ch, idx) {
            var cl = (claims || []).filter(function (c) {
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
    }
  };

  window.ThesisWorkspaceOverview = WorkspaceOverview;
})();
