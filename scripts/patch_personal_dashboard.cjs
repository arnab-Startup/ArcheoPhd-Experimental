const fs = require('fs');
const path = require('path');

const targetFile = path.resolve(__dirname, '../dist/assets/Dashboard-W5Q9sdSC.js');
let code = fs.readFileSync(targetFile, 'utf8');

const sStr = 'if (!isProject) {';
const sIdx = code.indexOf(sStr);
if (sIdx === -1) {
  console.error('Cannot find if (!isProject) {');
  process.exit(1);
}

const eStr = '// ===========================================================================\n  // UNIVERSAL ARCHAEOLOGICAL PROJECT WORKSPACE';
let eIdx = code.indexOf(eStr);
if (eIdx === -1) {
  const alt = '// Project Workstation Header with Back to Personal Dashboard Button';
  eIdx = code.indexOf(alt);
}
if (eIdx === -1) {
  console.error('Cannot find end of personal dashboard section');
  process.exit(1);
}

console.log('Replacing personal dashboard section from', sIdx, 'to', eIdx);

const replacement = `if (!isProject) {
    const researcherName = (a && a.full_name) ? a.full_name : "Lead Researcher";
    const researcherAffiliation = (a && a.data && a.data.university) ? a.data.university : "Doctoral Research Fellow";
    const nonDefaultProjects = (i || []).filter(p => p && p.id && p.id !== "default" && p.id !== "null");

    const enterProject = (projId, targetRoute = "/dashboard") => {
      try { localStorage.setItem("archaeo_active_project", projId); } catch(err) {}
      if (typeof o === "function") o(projId);
      if (typeof h === "function") h(targetRoute);
    };

    return e.jsxs("div", {className: "space-y-6 animate-fade-in pb-12", children: [
      // 1. Scholar Identity & Command Masthead
      e.jsxs("div", {className: "rounded-2xl border border-border bg-card p-6 md:p-8 shadow-xs relative overflow-hidden", children: [
        e.jsxs("div", {className: "relative z-10 flex flex-col md:flex-row md:items-center justify-between gap-6", children: [
          e.jsxs("div", {className: "space-y-1.5", children: [
            e.jsxs("div", {className: "flex items-center gap-2", children: [
              e.jsx("span", {className: "inline-flex items-center px-2.5 py-0.5 rounded-full text-[10px] font-700 tracking-wider uppercase bg-primary/10 text-primary border border-primary/20", children: "Doctoral Research Workstation"}),
              e.jsxs("span", {className: "inline-flex items-center gap-1.5 text-[11px] font-500 text-muted-foreground", children: [
                e.jsx("span", {className: "w-2 h-2 rounded-full bg-emerald-500 inline-block"}),
                "Local Vault Active - Offline Vector Engine Ready"
              ]})
            ]}),
            e.jsx("h1", {className: "font-display text-2xl md:text-3xl font-700 text-foreground tracking-tight", children: researcherName + " - Research Portfolio"}),
            e.jsx("p", {className: "text-xs md:text-sm text-muted-foreground max-w-2xl", children: researcherAffiliation + " - Unified management of excavations, dialectic thesis writing, and primary literature."})
          ]}),
          e.jsxs("div", {className: "flex flex-wrap items-center gap-2.5 shrink-0", children: [
            e.jsxs(r, {to: "/projects/new", className: "inline-flex items-center gap-2 px-4 py-2.5 rounded-lg bg-primary text-primary-foreground text-xs font-600 hover:bg-primary/90 transition-all shadow-xs cursor-pointer", children: [
              e.jsx(B, {className: "w-4 h-4"}),
              "New Research Project"
            ]}),
            e.jsxs(r, {to: "/projects", className: "inline-flex items-center gap-2 px-4 py-2.5 rounded-lg border border-border bg-background hover:bg-muted text-xs font-600 text-foreground transition-all shadow-xs cursor-pointer", children: [
              e.jsx(P, {className: "w-4 h-4 text-primary"}),
              "Browse Research Projects"
            ]})
          ]})
        ]})
      ]}),

      // 2. Scholar Executive Metrics (Non-Repetitive Academic Pillars)
      e.jsxs("div", {className: "grid grid-cols-2 lg:grid-cols-4 gap-4", children: [
        e.jsxs(r, {to: "/projects", className: "group rounded-xl border border-border bg-card p-5 hover:border-primary/40 transition-all shadow-2xs block", children: [
          e.jsxs("div", {className: "flex items-center justify-between text-muted-foreground mb-2", children: [
            e.jsx("span", {className: "text-[11px] font-600 uppercase tracking-wider text-muted-foreground", children: "Fieldwork Projects"}),
            e.jsx(P, {className: "w-4 h-4 text-primary group-hover:scale-110 transition-transform"})
          ]}),
          e.jsx("div", {className: "text-2xl font-700 text-foreground", children: nonDefaultProjects.length}),
          e.jsx("div", {className: "text-[11px] text-muted-foreground mt-1 flex items-center gap-1", children: [
            "Excavations & Regional Surveys",
            e.jsx(S, {className: "w-3 h-3 text-primary opacity-0 group-hover:opacity-100 transition-opacity"})
          ]})
        ]}),
        e.jsxs(r, {to: "/library", className: "group rounded-xl border border-border bg-card p-5 hover:border-primary/40 transition-all shadow-2xs block", children: [
          e.jsxs("div", {className: "flex items-center justify-between text-muted-foreground mb-2", children: [
            e.jsx("span", {className: "text-[11px] font-600 uppercase tracking-wider text-muted-foreground", children: "Primary Sources"}),
            e.jsx(R, {className: "w-4 h-4 text-accent group-hover:scale-110 transition-transform"})
          ]}),
          e.jsx("div", {className: "text-2xl font-700 text-foreground", children: g.length}),
          e.jsx("div", {className: "text-[11px] text-muted-foreground mt-1 flex items-center gap-1", children: [
            "Monographs & Excavation Reports",
            e.jsx(S, {className: "w-3 h-3 text-accent opacity-0 group-hover:opacity-100 transition-opacity"})
          ]})
        ]}),
        e.jsxs(r, {to: "/literature-graph", className: "group rounded-xl border border-border bg-card p-5 hover:border-primary/40 transition-all shadow-2xs block", children: [
          e.jsxs("div", {className: "flex items-center justify-between text-muted-foreground mb-2", children: [
            e.jsx("span", {className: "text-[11px] font-600 uppercase tracking-wider text-muted-foreground", children: "Citation Network"}),
            e.jsx(ee, {className: "w-4 h-4 text-chart-2 group-hover:scale-110 transition-transform"})
          ]}),
          e.jsx("div", {className: "text-2xl font-700 text-foreground", children: g.length}),
          e.jsx("div", {className: "text-[11px] text-muted-foreground mt-1 flex items-center gap-1", children: [
            "Cross-Monograph Graph",
            e.jsx(S, {className: "w-3 h-3 text-chart-2 opacity-0 group-hover:opacity-100 transition-opacity"})
          ]})
        ]}),
        e.jsxs(r, {to: "/notes", className: "group rounded-xl border border-border bg-card p-5 hover:border-primary/40 transition-all shadow-2xs block", children: [
          e.jsxs("div", {className: "flex items-center justify-between text-muted-foreground mb-2", children: [
            e.jsx("span", {className: "text-[11px] font-600 uppercase tracking-wider text-muted-foreground", children: "Scholar Fieldnotes"}),
            e.jsx(C, {className: "w-4 h-4 text-chart-5 group-hover:scale-110 transition-transform"})
          ]}),
          e.jsx("div", {className: "text-2xl font-700 text-foreground", children: $.length}),
          e.jsx("div", {className: "text-[11px] text-muted-foreground mt-1 flex items-center gap-1", children: [
            "Field Observations & Ideas",
            e.jsx(S, {className: "w-3 h-3 text-chart-5 opacity-0 group-hover:opacity-100 transition-opacity"})
          ]})
        ]})
      ]}),

      // 3. Main Scholarly Columns (Split: Left = Fieldwork & Projects, Right = Knowledge Vault & Assistant)
      e.jsxs("div", {className: "grid grid-cols-1 lg:grid-cols-12 gap-6 items-start", children: [
        // LEFT COLUMN: Fieldwork Projects & Dissertation Studio (7 cols)
        e.jsxs("div", {className: "lg:col-span-7 space-y-6", children: [
          // Fieldwork & Research Projects Card
          e.jsxs("div", {className: "rounded-2xl border border-border bg-card p-6 shadow-xs", children: [
            e.jsxs("div", {className: "flex items-center justify-between gap-3 mb-5 border-b border-border pb-4", children: [
              e.jsxs("div", {className: "flex items-center gap-2.5", children: [
                e.jsx("div", {className: "w-8 h-8 rounded-lg bg-primary/10 text-primary grid place-items-center", children: e.jsx(P, {className: "w-4 h-4"})}),
                e.jsxs("div", {children: [
                  e.jsx("h2", {className: "font-display text-base font-600 text-foreground", children: "Fieldwork & Excavation Projects"}),
                  e.jsx("p", {className: "text-[11px] text-muted-foreground", children: "Isolated trench units, Harris matrices, and artifact registries."})
                ]})
              ]}),
              e.jsxs(r, {to: "/projects", className: "text-xs font-600 text-primary hover:underline inline-flex items-center gap-1", children: [
                "All Projects (" + nonDefaultProjects.length + ")",
                e.jsx(S, {className: "w-3.5 h-3.5"})
              ]})
            ]}),

            nonDefaultProjects.length === 0 ? e.jsxs("div", {className: "py-10 px-4 rounded-xl border border-dashed border-border text-center", children: [
              e.jsx("div", {className: "w-10 h-10 rounded-xl bg-primary/10 text-primary grid place-items-center mx-auto mb-3", children: e.jsx(P, {className: "w-5 h-5"})}),
              e.jsx("h3", {className: "font-display text-sm font-600 text-foreground", children: "No excavation projects active yet"}),
              e.jsx("p", {className: "text-xs text-muted-foreground mt-1 max-w-sm mx-auto", children: "Initialize your first excavation or regional survey workspace to begin recording finds and stratigraphy."}),
              e.jsxs(r, {to: "/projects/new", className: "inline-flex items-center gap-1.5 mt-4 px-4 py-2 rounded-lg bg-primary text-primary-foreground text-xs font-600 hover:bg-primary/90 transition-all cursor-pointer shadow-xs", children: [
                e.jsx(B, {className: "w-3.5 h-3.5"}),
                "Create Excavation Project"
              ]})
            ]}) : e.jsx("div", {className: "space-y-3.5", children: nonDefaultProjects.map(proj => {
              return e.jsxs("div", {key: proj.id, className: "rounded-xl border border-border bg-muted/15 p-4 hover:border-primary/40 hover:bg-muted/25 transition-all group", children: [
                e.jsxs("div", {className: "flex items-start justify-between gap-3", children: [
                  e.jsxs("div", {className: "space-y-1 min-w-0", children: [
                    e.jsxs("div", {className: "flex items-center gap-2 flex-wrap", children: [
                      proj.is_demo ? e.jsx("span", {className: "inline-flex items-center gap-1 px-2 py-0.5 rounded text-[10px] font-700 uppercase bg-accent/15 text-accent", children: "Benchmark Dataset"}) : e.jsx("span", {className: "inline-flex items-center px-2 py-0.5 rounded text-[10px] font-700 uppercase bg-primary/10 text-primary font-mono", children: "Active Trench"}),
                      proj.research_period ? e.jsx("span", {className: "text-[11px] font-mono text-muted-foreground", children: proj.research_period}) : null,
                      proj.research_region ? e.jsxs("span", {className: "text-[11px] text-muted-foreground flex items-center gap-1", children: [
                        e.jsx(J, {className: "w-3 h-3 text-primary"}),
                        proj.research_region
                      ]}) : null
                    ]}),
                    e.jsx("h3", {className: "font-display text-base font-600 text-foreground group-hover:text-primary transition-colors truncate", children: proj.title}),
                    proj.research_question ? e.jsx("p", {className: "text-xs text-muted-foreground line-clamp-1 italic mt-0.5", children: "\\"" + proj.research_question + "\\""}) : null
                  ]}),
                  e.jsxs("div", {className: "flex items-center gap-2 shrink-0 pt-0.5", children: [
                    e.jsxs("button", {
                      type: "button",
                      onClick: () => enterProject(proj.id, "/dashboard"),
                      className: "inline-flex items-center gap-1 px-3 py-1.5 rounded-lg bg-primary text-primary-foreground text-xs font-600 hover:bg-primary/90 transition-all shadow-xs cursor-pointer",
                      children: [
                        "Enter",
                        e.jsx(S, {className: "w-3 h-3"})
                      ]
                    }),
                    e.jsx("button", {
                      type: "button",
                      onClick: () => enterProject(proj.id, "/thesis"),
                      className: "px-2.5 py-1.5 rounded-lg border border-border bg-background hover:bg-muted text-xs font-500 text-muted-foreground hover:text-foreground transition-all cursor-pointer",
                      title: "Thesis Studio for this project",
                      children: e.jsx(ee, {className: "w-3.5 h-3.5 text-accent"})
                    }),
                    e.jsx("button", {
                      type: "button",
                      onClick: () => enterProject(proj.id, "/ai-research"),
                      className: "px-2.5 py-1.5 rounded-lg border border-border bg-background hover:bg-muted text-xs font-500 text-muted-foreground hover:text-foreground transition-all cursor-pointer",
                      title: "AI Literature Research for this project",
                      children: e.jsx(d, {className: "w-3.5 h-3.5 text-primary"})
                    })
                  ]})
                ]})
              ]});
            })})
          ]}),

          // Doctoral Thesis & Dialectic Studio Card
          e.jsxs("div", {className: "rounded-2xl border border-border bg-card p-6 shadow-xs", children: [
            e.jsxs("div", {className: "flex items-center justify-between gap-3 mb-4 border-b border-border pb-3", children: [
              e.jsxs("div", {className: "flex items-center gap-2.5", children: [
                e.jsx("div", {className: "w-8 h-8 rounded-lg bg-accent/15 text-accent grid place-items-center", children: e.jsx(ee, {className: "w-4 h-4"})}),
                e.jsxs("div", {children: [
                  e.jsx("h2", {className: "font-display text-base font-600 text-foreground", children: "Doctoral Thesis Studio"}),
                  e.jsx("p", {className: "text-[11px] text-muted-foreground", children: "Dialectic argument mapping and monograph chapter drafting."})
                ]})
              ]}),
              e.jsx("span", {className: "text-[10px] font-600 uppercase tracking-wider text-muted-foreground bg-muted px-2 py-0.5 rounded", children: "Project Scoped"})
            ]}),
            e.jsx("p", {className: "text-xs text-muted-foreground leading-relaxed", children: "Integrates field findings directly into thesis arguments with automated citation audits and viva defensibility verification. Scoped to each active research project."}),
            e.jsxs("div", {className: "grid grid-cols-2 gap-3 mt-4 pt-1", children: [
              e.jsxs("div", {className: "p-3 rounded-xl bg-muted/20 border border-border/40", children: [
                e.jsx("div", {className: "text-[11px] font-600 text-muted-foreground uppercase tracking-wider", children: "Verified Citations"}),
                e.jsxs("div", {className: "text-lg font-700 text-foreground mt-0.5", children: [g.length, " Primary Volumes"]})
              ]}),
              e.jsxs("div", {className: "p-3 rounded-xl bg-muted/20 border border-border/40", children: [
                e.jsx("div", {className: "text-[11px] font-600 text-muted-foreground uppercase tracking-wider", children: "Dialectic Arguments"}),
                e.jsxs("div", {className: "text-lg font-700 text-foreground mt-0.5", children: [l.length, " Formulated Theses"]})
              ]})
            ]}),
            e.jsx("div", {className: "mt-4 pt-3 border-t border-border", children: [
              nonDefaultProjects.length > 0 ? e.jsxs("button", {
                type: "button",
                onClick: () => enterProject(nonDefaultProjects[0].id, "/thesis"),
                className: "inline-flex items-center justify-center gap-2 w-full py-2.5 rounded-lg bg-primary text-primary-foreground text-xs font-600 hover:bg-primary/90 transition-all shadow-xs cursor-pointer",
                children: [
                  "Open Thesis Studio (" + nonDefaultProjects[0].title + ")",
                  e.jsx(S, {className: "w-3.5 h-3.5"})
                ]
              }) : e.jsxs(r, {
                to: "/projects/new",
                className: "inline-flex items-center justify-center gap-2 w-full py-2.5 rounded-lg bg-primary text-primary-foreground text-xs font-600 hover:bg-primary/90 transition-all shadow-xs cursor-pointer",
                children: [
                  "Create Project to Start Thesis",
                  e.jsx(S, {className: "w-3.5 h-3.5"})
                ]
              })
            ]})
          ]})
        ]}),

        // RIGHT COLUMN: Scholar Desk & Literature Intelligence (5 cols)
        e.jsxs("div", {className: "lg:col-span-5 space-y-6", children: [
          // AI Literature Research Assistant Card
          e.jsxs("div", {className: "rounded-2xl border border-border bg-card p-6 shadow-xs", children: [
            e.jsxs("div", {className: "flex items-center justify-between gap-3 mb-3 border-b border-border pb-3", children: [
              e.jsxs("div", {className: "flex items-center gap-2.5", children: [
                e.jsx("div", {className: "w-8 h-8 rounded-lg bg-primary/10 text-primary grid place-items-center", children: e.jsx(d, {className: "w-4 h-4"})}),
                e.jsx("h2", {className: "font-display text-base font-600 text-foreground", children: "AI Literature Assistant"})
              ]}),
              e.jsx("span", {className: "text-[10px] font-700 uppercase tracking-wider text-primary bg-primary/10 px-2 py-0.5 rounded", children: "Local Neural Engine"})
            ]}),
            e.jsx("p", {className: "text-xs text-muted-foreground leading-relaxed", children: "Query across your catalogued monographs and excavation reports with verifiable citations and isolated project boundaries."}),
            e.jsxs("div", {className: "mt-4 space-y-2", children: [
              e.jsxs("button", {
                type: "button",
                onClick: () => {
                  if (nonDefaultProjects.length > 0) {
                    enterProject(nonDefaultProjects[0].id, "/ai-research?prompt=" + encodeURIComponent("Synthesize regional settlement patterns and chronological horizons"));
                  } else {
                    if (typeof h === "function") h("/projects/new");
                  }
                },
                className: "flex items-center justify-between w-full text-left text-xs p-2.5 rounded-lg bg-muted/20 hover:bg-muted/40 border border-border/40 transition-colors text-foreground group cursor-pointer",
                children: [
                  e.jsx("span", {className: "truncate font-500", children: "Synthesize regional settlement horizons"}),
                  e.jsx(S, {className: "w-3 h-3 text-primary opacity-0 group-hover:opacity-100 transition-opacity shrink-0 ml-2"})
                ]
              }),
              e.jsxs("button", {
                type: "button",
                onClick: () => {
                  if (nonDefaultProjects.length > 0) {
                    enterProject(nonDefaultProjects[0].id, "/ai-research?prompt=" + encodeURIComponent("Identify contradictions between excavation reports and pottery typologies"));
                  } else {
                    if (typeof h === "function") h("/projects/new");
                  }
                },
                className: "flex items-center justify-between w-full text-left text-xs p-2.5 rounded-lg bg-muted/20 hover:bg-muted/40 border border-border/40 transition-colors text-foreground group cursor-pointer",
                children: [
                  e.jsx("span", {className: "truncate font-500", children: "Identify contradictions in stratigraphic reports"}),
                  e.jsx(S, {className: "w-3 h-3 text-primary opacity-0 group-hover:opacity-100 transition-opacity shrink-0 ml-2"})
                ]
              })
            ]}),
            e.jsx("div", {className: "mt-4 pt-3 border-t border-border", children: [
              nonDefaultProjects.length > 0 ? e.jsxs("button", {
                type: "button",
                onClick: () => enterProject(nonDefaultProjects[0].id, "/ai-research"),
                className: "inline-flex items-center justify-center gap-1.5 w-full py-2 rounded-lg border border-border bg-background hover:bg-muted text-foreground text-xs font-600 transition-colors cursor-pointer",
                children: [
                  "Launch AI Research Console (" + nonDefaultProjects[0].title + ")",
                  e.jsx(S, {className: "w-3.5 h-3.5"})
                ]
              }) : e.jsxs(r, {
                to: "/projects/new",
                className: "inline-flex items-center justify-center gap-1.5 w-full py-2 rounded-lg border border-border bg-background hover:bg-muted text-foreground text-xs font-600 transition-colors cursor-pointer",
                children: [
                  "Create Project to Launch AI Research",
                  e.jsx(S, {className: "w-3.5 h-3.5"})
                ]
              })
            ]})
          ]}),

          // Primary Sources & Monographs Card
          e.jsxs("div", {className: "rounded-2xl border border-border bg-card p-6 shadow-xs", children: [
            e.jsxs("div", {className: "flex items-center justify-between mb-3 border-b border-border pb-3", children: [
              e.jsxs("div", {className: "flex items-center gap-2", children: [
                e.jsx(R, {className: "w-4 h-4 text-accent"}),
                e.jsx("h2", {className: "font-display text-base font-600 text-foreground", children: "Catalogued Monographs"}),
              ]}),
              e.jsx(r, {to: "/library", className: "text-xs font-600 text-primary hover:underline", children: "View All (" + g.length + ")"})
            ]}),
            g.length === 0 ? e.jsxs("div", {className: "py-6 text-center", children: [
              e.jsx(R, {className: "w-7 h-7 text-muted-foreground/30 mx-auto mb-1.5"}),
              e.jsx("p", {className: "text-xs text-muted-foreground", children: "No monographs catalogued yet."}),
              e.jsxs(r, {to: "/library", className: "inline-flex items-center gap-1 text-xs font-600 text-primary hover:underline mt-1.5", children: [
                "Upload Excavation Report or PDF",
                e.jsx(S, {className: "w-3 h-3"})
              ]})
            ]}) : e.jsx("div", {className: "space-y-1.5", children: g.slice(0, 4).map(s => {
              return e.jsxs(r, {key: s.id, to: "/library/" + s.id, className: "flex items-center justify-between group rounded-lg p-2 hover:bg-muted/30 transition-colors border border-transparent hover:border-border/60", children: [
                e.jsxs("div", {className: "min-w-0 pr-2", children: [
                  e.jsx("div", {className: "text-xs font-600 text-foreground group-hover:text-primary truncate", children: s.title}),
                  e.jsxs("div", {className: "text-[11px] text-muted-foreground truncate", children: [
                    s.author || "Survey Report",
                    s.year ? " - " + s.year : ""
                  ]})
                ]}),
                e.jsx(S, {className: "w-3 h-3 text-muted-foreground opacity-0 group-hover:opacity-100 transition-opacity shrink-0"})
              ]});
            })})
          ]}),

          // Scholar Fieldnotes Card
          e.jsxs("div", {className: "rounded-2xl border border-border bg-card p-6 shadow-xs", children: [
            e.jsxs("div", {className: "flex items-center justify-between mb-3 border-b border-border pb-3", children: [
              e.jsxs("div", {className: "flex items-center gap-2", children: [
                e.jsx(C, {className: "w-4 h-4 text-chart-5"}),
                e.jsx("h2", {className: "font-display text-base font-600 text-foreground", children: "Working Fieldnotes"})
              ]}),
              e.jsx(r, {to: "/notes", className: "text-xs font-600 text-primary hover:underline", children: "All Notes (" + $.length + ")"})
            ]}),
            $.length === 0 ? e.jsxs("div", {className: "py-6 text-center", children: [
              e.jsx(C, {className: "w-7 h-7 text-muted-foreground/30 mx-auto mb-1.5"}),
              e.jsx("p", {className: "text-xs text-muted-foreground", children: "No scholar notes recorded yet."}),
              e.jsxs(r, {to: "/notes", className: "inline-flex items-center gap-1 text-xs font-600 text-primary hover:underline mt-1.5", children: [
                "Record First Note or Hypothesis",
                e.jsx(S, {className: "w-3 h-3"})
              ]})
            ]}) : e.jsx("div", {className: "space-y-1.5", children: $.slice(0, 4).map(note => {
              return e.jsxs(r, {key: note.id, to: "/notes?focus=" + note.id, className: "flex items-center justify-between group rounded-lg p-2 hover:bg-muted/30 transition-colors border border-transparent hover:border-border/60", children: [
                e.jsxs("div", {className: "min-w-0 pr-2", children: [
                  e.jsx("div", {className: "text-xs font-600 text-foreground group-hover:text-primary truncate", children: note.title || "Untitled Fieldnote"}),
                  e.jsx("div", {className: "text-[11px] text-muted-foreground truncate", children: (note.content || "").replace(/<[^>]+>/g, "").slice(0, 75)})
                ]}),
                e.jsx(S, {className: "w-3 h-3 text-muted-foreground opacity-0 group-hover:opacity-100 transition-opacity shrink-0"})
              ]});
            })})
          ]})
        ]})
      ]}),

      // 4. Dialectic Disputes & Hypotheses Audit Stream
      e.jsxs("div", {className: "rounded-2xl border border-border bg-card p-6 shadow-xs", children: [
        e.jsxs("div", {className: "flex flex-col sm:flex-row sm:items-center justify-between gap-3 mb-5 border-b border-border pb-4", children: [
          e.jsxs("div", {className: "space-y-1", children: [
            e.jsxs("div", {className: "flex items-center gap-2", children: [
              e.jsx("h2", {className: "font-display text-base font-600 text-foreground", children: "Scholar Dialectic Claims & Active Disputes"}),
              l.length > 0 ? e.jsxs("span", {className: "text-[11px] font-600 px-2 py-0.5 rounded-full bg-accent/15 text-accent", children: [l.length, " Total Hypotheses"]}) : null
            ]}),
            e.jsx("p", {className: "text-xs text-muted-foreground", children: "Stratigraphic conflicts, dating disputes, and cross-source dialectics across active excavation units."})
          ]}),
          e.jsxs(r, {to: "/projects", className: "inline-flex items-center gap-1 px-3 py-1.5 rounded-lg bg-primary text-primary-foreground text-xs font-500 hover:bg-primary/90 transition-all shadow-xs self-start sm:self-center", children: [
            "View Project Claims",
            e.jsx(S, {className: "w-3.5 h-3.5"})
          ]})
        ]}),

        l.length === 0 ? e.jsxs("div", {className: "py-10 px-4 text-center rounded-xl bg-muted/15 border border-dashed border-border", children: [
          e.jsx(ee, {className: "w-8 h-8 text-muted-foreground/30 mx-auto mb-2.5"}),
          e.jsx("h3", {className: "text-xs font-600 text-foreground", children: "No dialectic disputes catalogued yet"}),
          e.jsx("p", {className: "text-[11px] text-muted-foreground max-w-sm mx-auto mt-1", children: "Select an excavation project to formulate claims and connect supporting or contradicting evidence."})
        ]}) : e.jsx("div", {className: "space-y-3", children: l.slice(0, 5).map(claim => {
          return e.jsxs("div", {key: claim.id, className: "p-4 rounded-xl border border-border/80 bg-muted/10 hover:bg-muted/20 transition-all flex flex-col md:flex-row md:items-center justify-between gap-4", children: [
            e.jsxs("div", {className: "space-y-1.5 min-w-0 flex-1", children: [
              e.jsxs("div", {className: "flex items-center gap-2 flex-wrap", children: [
                e.jsx("span", {className: "text-[10px] font-700 tracking-wider uppercase px-2 py-0.5 rounded bg-muted text-muted-foreground font-mono", children: claim.claim_type || "HYPOTHESIS"}),
                claim.confidence ? e.jsxs("span", {className: "text-[11px] text-muted-foreground flex items-center gap-1", children: [
                  "Confidence: ",
                  e.jsx("strong", {className: "text-foreground font-600", children: Math.round(claim.confidence * 100) + "%"})
                ]}) : null
              ]}),
              e.jsx("h4", {className: "text-sm font-600 text-foreground leading-snug", children: claim.claim_text || claim.title || "Archaeological Proposition"}),
              claim.epistemic_status ? e.jsxs("p", {className: "text-xs text-muted-foreground italic line-clamp-1", children: ["Status: ", claim.epistemic_status]}) : null
            ]}),
            e.jsxs("button", {
              type: "button",
              onClick: () => {
                var pId = claim.project_id || (nonDefaultProjects[0] && nonDefaultProjects[0].id);
                if (pId) enterProject(pId, "/claims/" + claim.id);
                else if (typeof h === "function") h("/projects");
              },
              className: "inline-flex items-center gap-1 px-3 py-1.5 rounded-lg border border-border bg-background hover:bg-muted text-xs font-500 text-foreground transition-all shrink-0 self-start md:self-center shadow-xs cursor-pointer",
              children: [
                "Audit Dispute",
                e.jsx(S, {className: "w-3.5 h-3.5"})
              ]
            })
          ]});
        })})
      ]})
    ]});
  }

  `;

code = code.slice(0, sIdx) + replacement + code.slice(eIdx);
fs.writeFileSync(targetFile, code, 'utf8');
console.log('[SUCCESS] Successfully applied senior-level Personal Dashboard to Dashboard-W5Q9sdSC.js!');
