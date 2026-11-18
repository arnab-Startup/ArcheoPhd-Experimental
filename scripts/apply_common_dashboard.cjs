const fs = require('fs');
const path = require('path');

const targetFile = path.resolve(__dirname, '../dist/assets/Dashboard-W5Q9sdSC.js');

const code = `import{u as G,q as L,a as W,j as e,m as z,T as Q,s as V,t as q,S as d,v as P,n as r,P as B,F as A,A as S,w as R,M as J,x as K,G as E,e as I,y as X,z as Y,d as Z,D as C,E as ee}from"./index-CAm4ELM4.js";import{u as se}from"./useResearchData-CN6S6dj0.js";import{P as te}from"./PageHeader-B4meANjY.js";import{N as re}from"./NoProject-By5ffpqf.js";import{H as ae}from"./hourglass-CLYCD7xm.js";import{T as ce}from"./trending-up-DLPWF8aY.js";import"./folder-plus-C6cIoGzl.js";

function ge(){
  const {user: a} = G(),
        {activeProject: c, projects: i = [], loading: m, setActive: o} = L(),
        h = W(),
        {sources: g = [], sites: T = [], artifacts: D = [], claims: l = [], evidence: n = [], notes: $ = [], messages: M = []} = se(["sources","sites","artifacts","claims","evidence","notes","messages"]);

  if (m) {
    return e.jsx("div", {className: "flex items-center justify-center py-20", children: e.jsx(z, {className: "w-6 h-6 animate-spin text-muted-foreground"})});
  }

  const isProject = Boolean(c && c.id && c.id !== "default" && c.id !== "null" && c.id !== "");

  // Disputed claims & contradictory evidence
  const contradictingEv = n.filter(s => s.evidence_type === "contradicting");
  const claimsWithDispute = l.filter(s => n.some(t => t.claim_id === s.id && t.evidence_type === "contradicting"));
  const claimsWithEvidence = l.filter(s => n.some(t => t.claim_id === s.id));
  const claimsNeedingEvidence = l.filter(s => !n.some(t => t.claim_id === s.id));
  const coveragePercent = l.length ? Math.round((claimsWithEvidence.length / l.length) * 100) : 0;

  // ===========================================================================
  // COMMON PERSONAL RESEARCHER DASHBOARD (For every researcher: MA / PhD / Prof)
  // ===========================================================================
  if (!isProject) {
    const researcherName = (a && a.full_name) ? a.full_name : "Lead Researcher";
    const nonDefaultProjects = i.filter(p => p.id !== "default" && p.id !== "null");

    return e.jsxs("div", {className: "space-y-6", children: [
      // Personal Researcher Portal Header
      e.jsxs("div", {className: "rounded-2xl border border-border bg-card p-6 shadow-xs", children: [
        e.jsxs("div", {className: "flex flex-col md:flex-row md:items-center justify-between gap-4", children: [
          e.jsxs("div", {children: [
            e.jsx("div", {className: "inline-flex items-center px-2 py-0.5 rounded text-[10px] font-700 tracking-wider uppercase bg-primary/10 text-primary mb-2", children: "Researcher Portal"}),
            e.jsx("h1", {className: "font-display text-2xl font-700 text-foreground tracking-tight", children: researcherName + " — Research Home"}),
            e.jsx("p", {className: "text-xs text-muted-foreground mt-1", children: "Archaeological Research Portfolio & Fieldwork Management Workstation"})
          ]}),
          e.jsxs("div", {className: "flex items-center gap-2", children: [
            e.jsxs(r, {to: "/projects/new", className: "inline-flex items-center gap-1.5 px-3.5 py-2 rounded-lg bg-primary text-primary-foreground text-xs font-500 hover:bg-primary/90 transition-all shadow-xs", children: [
              e.jsx(P, {className: "w-3.5 h-3.5"}),
              "New Research Project"
            ]}),
            e.jsxs(r, {to: "/thesis", className: "inline-flex items-center gap-1.5 px-3 py-2 rounded-lg border border-border bg-background hover:bg-muted text-xs font-500 text-foreground transition-all shadow-xs", children: [
              e.jsx(Z, {className: "w-3.5 h-3.5"}),
              "Thesis & Writing Studio"
            ]})
          ]})
        ]})
      ]}),

      // Common Research Portfolio Statistics
      e.jsxs("div", {className: "grid grid-cols-2 md:grid-cols-4 gap-4", children: [
        e.jsxs("div", {className: "rounded-xl border border-border bg-card p-4 shadow-xs", children: [
          e.jsxs("div", {className: "flex items-center justify-between text-muted-foreground mb-1", children: [
            e.jsx("span", {className: "text-xs font-500", children: "Active Projects"}),
            e.jsx(P, {className: "w-4 h-4 text-foreground"})
          ]}),
          e.jsx("div", {className: "text-2xl font-700 text-foreground", children: nonDefaultProjects.length}),
          e.jsx("div", {className: "text-[11px] text-muted-foreground mt-1", children: "Excavations & Surveys"})
        ]}),
        e.jsxs("div", {className: "rounded-xl border border-border bg-card p-4 shadow-xs", children: [
          e.jsxs("div", {className: "flex items-center justify-between text-muted-foreground mb-1", children: [
            e.jsx("span", {className: "text-xs font-500", children: "Monograph Library"}),
            e.jsx(R, {className: "w-4 h-4 text-accent"})
          ]}),
          e.jsx("div", {className: "text-2xl font-700 text-foreground", children: g.length}),
          e.jsx("div", {className: "text-[11px] text-muted-foreground mt-1", children: "Catalogued Primary Sources"})
        ]}),
        e.jsxs("div", {className: "rounded-xl border border-border bg-card p-4 shadow-xs", children: [
          e.jsxs("div", {className: "flex items-center justify-between text-muted-foreground mb-1", children: [
            e.jsx("span", {className: "text-xs font-500", children: "Fieldnotes & Logs"}),
            e.jsx(C, {className: "w-4 h-4 text-chart-5"})
          ]}),
          e.jsx("div", {className: "text-2xl font-700 text-foreground", children: $.length}),
          e.jsx("div", {className: "text-[11px] text-muted-foreground mt-1", children: "Trench Logs & Fieldnotes"})
        ]}),
        e.jsxs("div", {className: "rounded-xl border border-border bg-card p-4 shadow-xs", children: [
          e.jsxs("div", {className: "flex items-center justify-between text-muted-foreground mb-1", children: [
            e.jsx("span", {className: "text-xs font-500", children: "Empirical Claims"}),
            e.jsx(E, {className: "w-4 h-4 text-chart-3"})
          ]}),
          e.jsx("div", {className: "text-2xl font-700 text-foreground", children: l.length}),
          e.jsx("div", {className: "text-[11px] text-muted-foreground mt-1", children: claimsWithDispute.length + " Disputed in Evidence"})
        ]})
      ]}),

      // Research Projects Portfolio (The Core Container)
      e.jsxs("div", {className: "rounded-2xl border border-border bg-card p-6 shadow-xs", children: [
        e.jsxs("div", {className: "flex items-center justify-between mb-4 border-b border-border pb-3", children: [
          e.jsxs("div", {children: [
            e.jsx("h2", {className: "font-display text-base font-600 text-foreground", children: "Research Projects Portfolio"}),
            e.jsx("p", {className: "text-xs text-muted-foreground mt-0.5", children: "Each research project maintains its own isolated artifacts, stratigraphy, and fieldnotes."})
          ]}),
          e.jsxs(r, {to: "/projects", className: "text-xs font-500 text-primary hover:underline inline-flex items-center gap-1", children: ["Manage Projects", e.jsx(S, {className: "w-3 h-3"})]})
        ]}),
        nonDefaultProjects.length === 0 ? e.jsxs("div", {className: "py-12 text-center", children: [
          e.jsx(P, {className: "w-8 h-8 text-muted-foreground/40 mx-auto mb-2"}),
          e.jsx("p", {className: "text-sm text-foreground font-500", children: "No research projects catalogued yet."}),
          e.jsx("p", {className: "text-xs text-muted-foreground mt-1", children: "Create your first excavation or survey project to begin recording fieldwork."}),
          e.jsxs(r, {to: "/projects/new", className: "inline-flex items-center gap-1.5 mt-4 px-4 py-2 rounded-lg bg-primary text-primary-foreground text-xs font-600 hover:bg-primary/90 transition-all", children: [
            e.jsx(P, {className: "w-3.5 h-3.5"}),
            "Create New Project"
          ]})
        ]}) : e.jsx("div", {className: "grid md:grid-cols-2 gap-4", children: nonDefaultProjects.map(proj => {
          return e.jsxs("div", {key: proj.id, className: "rounded-xl border border-border bg-muted/20 p-5 hover:border-primary/50 transition-all flex flex-col justify-between", children: [
            e.jsxs("div", {children: [
              e.jsxs("div", {className: "flex items-center justify-between gap-2 mb-2", children: [
                proj.is_demo ? e.jsx("span", {className: "inline-flex items-center px-2 py-0.5 rounded text-[10px] font-700 tracking-wider uppercase bg-accent/20 text-accent", children: "Benchmark Dataset"}) : e.jsx("span", {className: "inline-flex items-center px-2 py-0.5 rounded text-[10px] font-700 tracking-wider uppercase bg-primary/10 text-primary", children: "Field Project"}),
                e.jsx("span", {className: "text-[11px] font-mono text-muted-foreground", children: proj.research_period || "Multi-Period"})
              ]}),
              e.jsx("h3", {className: "font-display text-base font-600 text-foreground leading-snug", children: proj.title}),
              proj.research_question ? e.jsx("p", {className: "text-xs text-muted-foreground mt-2 line-clamp-2", children: proj.research_question}) : null,
              e.jsxs("div", {className: "flex flex-wrap items-center gap-3 mt-3 text-xs text-muted-foreground", children: [
                proj.research_region ? e.jsxs("span", {className: "inline-flex items-center gap-1 font-500", children: [
                  e.jsx(J, {className: "w-3 h-3 text-chart-2"}),
                  proj.research_region
                ]}) : null
              ]})
            ]}),
            e.jsxs("div", {className: "flex items-center justify-between pt-4 mt-4 border-t border-border", children: [
              e.jsxs("button", {
                type: "button",
                onClick: () => {
                  try {
                    localStorage.setItem("archaeo_active_project", proj.id);
                  } catch (e) {}
                  if (typeof o === "function") o(proj.id);
                },
                className: "inline-flex items-center gap-1.5 px-3.5 py-1.5 rounded-lg bg-primary text-primary-foreground text-xs font-600 hover:bg-primary/90 transition-all shadow-xs cursor-pointer",
                children: [
                  "Open Project Workspace",
                  e.jsx(S, {className: "w-3 h-3"})
                ]
              }),
              e.jsxs(r, {to: "/projects/" + proj.id + "/settings", className: "text-xs text-muted-foreground hover:text-foreground", children: ["Settings"]})
            ]})
          ]});
        })})
      ]}),

      // Global Literature & Fieldnotes Quick Access
      e.jsxs("div", {className: "grid lg:grid-cols-2 gap-6", children: [
        e.jsxs("div", {className: "rounded-2xl border border-border bg-card p-6 shadow-xs", children: [
          e.jsxs("div", {className: "flex items-center justify-between mb-4 border-b border-border pb-3", children: [
            e.jsxs("div", {className: "flex items-center gap-2", children: [
              e.jsx(R, {className: "w-4 h-4 text-accent"}),
              e.jsx("h2", {className: "font-display text-base font-600 text-foreground", children: "Recent Monographs & Excavation Reports"})
            ]}),
            e.jsx(r, {to: "/library", className: "text-xs font-500 text-primary hover:underline", children: "View All (" + g.length + ")"})
          ]}),
          g.length === 0 ? e.jsx("p", {className: "text-xs text-muted-foreground py-6 text-center", children: "No monographs catalogued yet."}) : e.jsx("div", {className: "space-y-2.5", children: g.slice(0, 4).map(s => {
            return e.jsxs(r, {key: s.id, to: "/library/" + s.id, className: "block group rounded-lg p-2 hover:bg-muted/40 transition-colors", children: [
              e.jsx("div", {className: "text-xs font-600 text-foreground group-hover:text-primary truncate", children: s.title}),
              e.jsxs("div", {className: "text-[11px] text-muted-foreground truncate mt-0.5", children: [
                s.author || "ASI Survey",
                s.year ? " · " + s.year : ""
              ]})
            ]});
          })})
        ]}),
        e.jsxs("div", {className: "rounded-2xl border border-border bg-card p-6 shadow-xs", children: [
          e.jsxs("div", {className: "flex items-center justify-between mb-4 border-b border-border pb-3", children: [
            e.jsxs("div", {className: "flex items-center gap-2", children: [
              e.jsx(C, {className: "w-4 h-4 text-chart-5"}),
              e.jsx("h2", {className: "font-display text-base font-600 text-foreground", children: "Recent Research Fieldnotes & Logs"})
            ]}),
            e.jsx(r, {to: "/notes", className: "text-xs font-500 text-primary hover:underline", children: "View All (" + $.length + ")"})
          ]}),
          $.length === 0 ? e.jsx("p", {className: "text-xs text-muted-foreground py-6 text-center", children: "No fieldnotes recorded yet."}) : e.jsx("div", {className: "space-y-2.5", children: $.slice(0, 4).map(note => {
            return e.jsxs(r, {key: note.id, to: "/notes?focus=" + note.id, className: "block group rounded-lg p-2 hover:bg-muted/40 transition-colors", children: [
              e.jsx("div", {className: "text-xs font-600 text-foreground group-hover:text-primary truncate", children: note.title}),
              e.jsx("div", {className: "text-[11px] text-muted-foreground truncate mt-0.5", children: (note.content || "").replace(/<[^>]+>/g, "").slice(0, 85)})
            ]});
          })})
        ]})
      ]})
    ]});
  }

  // ===========================================================================
  // UNIVERSAL ARCHAEOLOGICAL PROJECT WORKSPACE (Identical for MA / PhD / Prof)
  // ===========================================================================
  return e.jsxs("div", {className: "space-y-6", children: [
    // Project Workstation Header with Back to Personal Dashboard Button
    e.jsxs("div", {className: "rounded-2xl border border-border bg-card p-6 shadow-xs", children: [
      e.jsxs("div", {className: "flex flex-col md:flex-row md:items-center justify-between gap-4 border-b border-border pb-4 mb-4", children: [
        e.jsxs("div", {children: [
          e.jsxs("div", {className: "flex items-center gap-2 mb-1.5", children: [
            e.jsx("button", {
              type: "button",
              onClick: () => {
                if (window.history.length > 1) {
                  window.history.back();
                } else {
                  window.location.href = "/projects";
                }
              },
              className: "inline-flex items-center gap-1.5 px-2.5 py-1 rounded-md border border-border bg-background hover:bg-muted text-xs font-500 text-foreground transition-all cursor-pointer mr-1 shadow-2xs",
              title: "Back",
              children: [
                e.jsx(S, {className: "w-3 h-3 rotate-180"}),
                "Back"
              ]
            }),
            e.jsx("span", {className: "inline-flex items-center px-2.5 py-0.5 rounded text-[10px] font-700 tracking-wider uppercase bg-primary/15 text-primary", children: "Archaeological Project Workspace"}),
            c.is_demo ? e.jsx("span", {className: "inline-flex items-center px-2.5 py-0.5 rounded text-[10px] font-700 tracking-wider uppercase bg-accent/20 text-accent", children: "Benchmark Dataset"}) : null
          ]}),
          e.jsx("h1", {className: "font-display text-2xl font-700 text-foreground tracking-tight", children: c.title}),
          c.research_question ? e.jsx("p", {className: "text-xs italic text-muted-foreground mt-1.5 max-w-3xl", children: c.research_question}) : null,
          e.jsxs("div", {className: "flex flex-wrap items-center gap-3 mt-2 text-xs text-muted-foreground", children: [
            c.research_region ? e.jsxs("span", {className: "inline-flex items-center gap-1 font-500", children: [
              e.jsx("span", {className: "text-foreground/80 font-600", children: "Region:"}),
              " ",
              c.research_region
            ]}) : null,
            c.research_period ? e.jsxs("span", {className: "inline-flex items-center gap-1 font-500", children: [
              e.jsx("span", {className: "text-foreground/80 font-600", children: "Period:"}),
              " ",
              c.research_period
            ]}) : null,
            e.jsx("span", {className: "text-muted-foreground/40", children: "•"}),
            e.jsx("span", {className: "font-mono text-[11px] text-muted-foreground", children: "Stratigraphic Context · IntCal20 Calibrated"})
          ]})
        ]}),
        e.jsx("div", {className: "flex items-center gap-2", children: [
          e.jsxs(r, {to: "/claims", className: "inline-flex items-center gap-1 px-3 py-1.5 rounded-lg bg-primary text-primary-foreground text-xs font-500 hover:bg-primary/90 transition-all shadow-xs", children: [
            "Audit Claims",
            e.jsx(S, {className: "w-3 h-3"})
          ]})
        ]})
      ]}),

      // Project Context Health Summary
      e.jsxs("div", {className: "grid grid-cols-2 md:grid-cols-4 gap-4", children: [
        e.jsxs("div", {className: "p-3 rounded-xl bg-muted/30 border border-border/50", children: [
          e.jsx("div", {className: "text-[11px] font-500 text-muted-foreground", children: "Excavation Sites & Loci"}),
          e.jsx("div", {className: "text-xl font-700 text-foreground mt-0.5", children: T.length}),
          e.jsx("div", {className: "text-[10px] text-muted-foreground mt-0.5", children: "Stratigraphic Trenches"})
        ]}),
        e.jsxs("div", {className: "p-3 rounded-xl bg-muted/30 border border-border/50", children: [
          e.jsx("div", {className: "text-[11px] font-500 text-muted-foreground", children: "Catalogued Finds"}),
          e.jsx("div", {className: "text-xl font-700 text-chart-4 mt-0.5", children: D.length}),
          e.jsx("div", {className: "text-[10px] text-muted-foreground mt-0.5", children: "Artifacts & Typology"})
        ]}),
        e.jsxs("div", {className: "p-3 rounded-xl bg-muted/30 border border-border/50", children: [
          e.jsx("div", {className: "text-[11px] font-500 text-muted-foreground", children: "Field Reports & Sources"}),
          e.jsx("div", {className: "text-xl font-700 text-accent mt-0.5", children: g.length}),
          e.jsx("div", {className: "text-[10px] text-muted-foreground mt-0.5", children: "Primary Monographs"})
        ]}),
        e.jsxs("div", {className: "p-3 rounded-xl bg-muted/30 border border-border/50", children: [
          e.jsx("div", {className: "text-[11px] font-500 text-muted-foreground", children: "Empirical Evidence"}),
          e.jsx("div", {className: "text-xl font-700 text-chart-3 mt-0.5", children: n.length}),
          e.jsx("div", {className: "text-[10px] text-muted-foreground mt-0.5", children: claimsWithDispute.length + " Disputed Claims"})
        ]})
      ]})
    ]}),

    // Disputed Claims in this Project
    e.jsxs("div", {className: "rounded-2xl border border-border bg-card p-6 shadow-xs", children: [
      e.jsxs("div", {className: "flex items-center justify-between mb-4 border-b border-border pb-3", children: [
        e.jsxs("div", {className: "flex items-center gap-2", children: [
          e.jsx(V, {className: "w-4 h-4 text-chart-4"}),
          e.jsx("h2", {className: "font-display text-base font-600 text-foreground", children: "Disputed Claims & Contradictory Evidence"}),
          e.jsx("span", {className: "ml-2 px-2 py-0.5 rounded text-[10px] font-600 bg-chart-4/15 text-chart-4", children: claimsWithDispute.length + " Disputed Claims"})
        ]}),
        e.jsxs(r, {to: "/evidence-graph", className: "text-xs font-500 text-primary hover:underline inline-flex items-center gap-1", children: ["Open Evidence Graph", e.jsx(S, {className: "w-3 h-3"})]})
      ]}),
      claimsWithDispute.length === 0 ? e.jsxs("div", {className: "py-6 text-center text-xs text-muted-foreground flex items-center justify-center gap-2", children: [
        e.jsx(Y, {className: "w-4 h-4 text-accent"}),
        "No conflicting claims recorded in this project."
      ]}) : e.jsx("div", {className: "space-y-2.5", children: claimsWithDispute.slice(0, 4).map(claim => {
        return e.jsxs("div", {key: claim.id, className: "rounded-xl border border-border bg-muted/20 p-4 flex flex-col md:flex-row md:items-center justify-between gap-3", children: [
          e.jsxs("div", {className: "space-y-1", children: [
            e.jsx("div", {className: "text-sm font-600 text-foreground", children: claim.claim_text}),
            e.jsxs("div", {className: "text-xs text-muted-foreground flex items-center gap-2", children: [
              e.jsx("span", {className: "text-destructive font-500", children: "Contradictory evidence present"}),
              e.jsx("span", {children: "•"}),
              e.jsx("span", {children: "Category: " + (claim.category || "Stratigraphic Dating")})
            ]})
          ]}),
          e.jsxs(r, {to: "/claims/" + claim.id, className: "inline-flex items-center gap-1 px-3 py-1.5 rounded-lg border border-border bg-background hover:bg-muted text-xs font-500 text-foreground transition-all shrink-0 self-start md:self-center shadow-xs", children: [
            "Audit Dispute",
            e.jsx(S, {className: "w-3 h-3"})
          ]})
        ]});
      })})
    ]}),

    // Project Fieldnotes & Recent Sources Grid
    e.jsxs("div", {className: "grid lg:grid-cols-2 gap-6", children: [
      e.jsxs("div", {className: "rounded-2xl border border-border bg-card p-6 shadow-xs", children: [
        e.jsxs("div", {className: "flex items-center justify-between mb-4 border-b border-border pb-3", children: [
          e.jsxs("div", {className: "flex items-center gap-2", children: [
            e.jsx(R, {className: "w-4 h-4 text-accent"}),
            e.jsx("h2", {className: "font-display text-base font-600 text-foreground", children: "Excavation Monographs & Field Reports"})
          ]}),
          e.jsx(r, {to: "/library", className: "text-xs font-500 text-primary hover:underline", children: "View All (" + g.length + ")"})
        ]}),
        g.length === 0 ? e.jsx("p", {className: "text-xs text-muted-foreground py-6 text-center", children: "No sources catalogued in this project."}) : e.jsx("div", {className: "space-y-2.5", children: g.slice(0, 4).map(s => {
          return e.jsxs(r, {key: s.id, to: "/library/" + s.id, className: "block group rounded-lg p-2 hover:bg-muted/40 transition-colors", children: [
            e.jsx("div", {className: "text-xs font-600 text-foreground group-hover:text-primary truncate", children: s.title}),
            e.jsxs("div", {className: "text-[11px] text-muted-foreground truncate mt-0.5", children: [
              s.author || "ASI Survey",
              s.year ? " · " + s.year : ""
            ]})
          ]});
        })})
      ]}),
      e.jsxs("div", {className: "rounded-2xl border border-border bg-card p-6 shadow-xs", children: [
        e.jsxs("div", {className: "flex items-center justify-between mb-4 border-b border-border pb-3", children: [
          e.jsxs("div", {className: "flex items-center gap-2", children: [
            e.jsx(C, {className: "w-4 h-4 text-chart-5"}),
            e.jsx("h2", {className: "font-display text-base font-600 text-foreground", children: "Trench Fieldnotes & Stratigraphic Logs"})
          ]}),
          e.jsx(r, {to: "/notes", className: "text-xs font-500 text-primary hover:underline", children: "View All (" + $.length + ")"})
        ]}),
        $.length === 0 ? e.jsx("p", {className: "text-xs text-muted-foreground py-6 text-center", children: "No trench notes recorded in this project."}) : e.jsx("div", {className: "space-y-2.5", children: $.slice(0, 4).map(n => {
          return e.jsxs(r, {key: n.id, to: "/notes?focus=" + n.id, className: "block group rounded-lg p-2 hover:bg-muted/40 transition-colors", children: [
            e.jsx("div", {className: "text-xs font-600 text-foreground group-hover:text-primary truncate", children: n.title}),
            e.jsx("div", {className: "text-[11px] text-muted-foreground truncate mt-0.5", children: (n.content || "").replace(/<[^>]+>/g, "").slice(0, 90)})
          ]});
        })})
      ]})
    ]})
  ]});
}

export{ge as default};
`;

fs.writeFileSync(targetFile, code, 'utf8');
console.log('Successfully updated Dashboard-W5Q9sdSC.js with Common Research Personal Dashboard!');
