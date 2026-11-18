const fs = require('fs');
const path = require('path');

const targetFile = path.resolve(__dirname, '../dist/assets/index-CAm4ELM4.js');
let code = fs.readFileSync(targetFile, 'utf8');

// Find start of FE and end of BE
const feStartStr = 'const FE=[';
const feStart = code.indexOf(feStartStr);
if (feStart === -1) {
  console.error('Cannot find const FE=');
  process.exit(1);
}

const beEndStr = 'var $E=Object.defineProperty';
const beEnd = code.indexOf(beEndStr, feStart);
if (beEnd === -1) {
  console.error('Cannot find end of BE');
  process.exit(1);
}

console.log('Original FE -> BE length:', beEnd - feStart);

// Replacement code with clean dual-mode navigation
const replacement = `const FE_personal = [
  { group: "Scholar Home", items: [
      { to: "/dashboard", label: "Overview", icon: pb },
      { to: "/projects", label: "Research Projects", icon: X1 },
      { to: "/thesis", label: "Thesis Studio", icon: Xg },
      { to: "/ai-research", label: "AI Literature Research", icon: Os },
      { to: "/notes", label: "Scholar Fieldnotes", icon: sv }
  ]},
  { group: "Resources & Archive", items: [
      { to: "/library", label: "Monograph Library", icon: gb },
      { to: "/literature-graph", label: "Citation Network", icon: b1 },
      { to: "/export-center", label: "Export Center", icon: Wr }
  ]}
];

const FE_project = [
  { group: "Excavation Workstation", items: [
      { to: "/dashboard", label: "Project Overview", icon: pb },
      { to: "/sites", label: "Sites & Trenches", icon: tf },
      { to: "/artifacts", label: "Artifacts & Finds", icon: Jg },
      { to: "/notes", label: "Trench Logs", icon: sv }
  ]},
  { group: "Stratigraphy & Evidence", items: [
      { to: "/claims", label: "Claims & Hypotheses", icon: $l },
      { to: "/evidence-graph", label: "Evidence Graph", icon: Ro },
      { to: "/contradiction-detector", label: "Contradictions", icon: ov },
      { to: "/chronology", label: "Chronology Engine", icon: Is },
      { to: "/matrix", label: "Harris Matrix", icon: Xb },
      { to: "/research-map", label: "Spatial GIS Map", icon: _b }
  ]},
  { group: "Documentation", items: [
      { to: "/library", label: "Field Reports & Sources", icon: gb },
      { to: "/analytics", label: "Excavation Analytics", icon: Pm }
  ]}
];

function BE({ onNavigate: n }) {
  const { projects: r, activeProjectId: o, setActive: i, loading: c } = Zv(),
        { user: u } = Fs(),
        f = Ls(),
        loc = Un();

  const isPersonalRoute = Boolean(loc && (loc.pathname === "/projects" || loc.pathname === "/projects/new" || loc.pathname === "/settings" || loc.pathname === "/help"));
  const isProj = Boolean(o && o !== "default" && o !== "null" && o !== "" && !isPersonalRoute);
  const currentProj = isProj ? r.find(h => h.id === o) : null;
  const navGroups = isProj ? FE_project : FE_personal;

  return a.jsxs("aside", {
    className: "hidden lg:flex w-64 shrink-0 flex-col border-r border-sidebar-border bg-sidebar h-full max-h-full sticky top-0",
    children: [
      // Top Logo Header
      a.jsx("div", {
        className: "px-5 h-16 flex items-center border-b border-sidebar-border",
        children: a.jsx(Wl, { to: "/" })
      }),

      // Mode-Aware Context Header
      isProj ? a.jsx("div", {
        className: "px-3 py-3 border-b border-sidebar-border bg-sidebar-accent/20",
        children: a.jsxs("div", {
          className: "px-1.5",
          children: [
            a.jsx("div", { className: "text-[10px] font-700 uppercase tracking-wider text-primary", children: "Active Project" }),
            a.jsx("div", { className: "text-sm font-600 text-foreground truncate mt-0.5", children: (currentProj && currentProj.title) ? currentProj.title : "Excavation Workspace" }),
            a.jsx("div", { className: "text-[11px] text-muted-foreground truncate", children: (currentProj && currentProj.research_region) ? currentProj.research_region : "Field Project" })
          ]
        })
      }) : a.jsxs("div", {
        className: "px-3 py-3 border-b border-sidebar-border",
        children: [
          a.jsx("div", { className: "px-1.5 text-[10px] font-700 uppercase tracking-wider text-muted-foreground", children: "Scholar Workspace" }),
          a.jsx("div", { className: "px-1.5 text-sm font-600 text-foreground truncate mt-0.5", children: (u && u.full_name) ? u.full_name : "Lead Researcher" }),
          a.jsxs(OE, {
            value: "",
            onValueChange: (val) => {
              if (val) {
                try { localStorage.setItem("archaeo_active_project", val); } catch(e){}
                i(val);
              }
            },
            disabled: c || r.length === 0,
            children: [
              a.jsx(kx, {
                className: "mt-2 h-8 text-xs bg-background border-sidebar-border",
                children: a.jsx(ME, { placeholder: "Open a Project Workspace…" })
              }),
              a.jsx(Nx, {
                children: r.map(h => a.jsx(Ex, {
                  value: h.id,
                  children: a.jsxs("span", {
                    className: "flex items-center gap-2",
                    children: [
                      h.is_demo && a.jsx(Zg, { className: "w-3 h-3 text-accent" }),
                      a.jsx("span", { className: "truncate", children: h.title })
                    ]
                  })
                }, h.id))
              })
            ]
          })
        ]
      }),

      // Navigation Items
      a.jsxs("nav", {
        className: "flex-1 overflow-y-auto scrollbar-thin px-3 py-4 space-y-5",
        children: [
          navGroups.map(h => a.jsxs("div", {
            key: h.group,
            children: [
              a.jsx("div", { className: "px-2 mb-1.5 text-[11px] font-600 uppercase tracking-wider text-muted-foreground", children: h.group }),
              a.jsx("div", {
                className: "space-y-0.5",
                children: h.items.map(g => a.jsxs(td, {
                  key: g.to,
                  to: g.to,
                  onClick: n,
                  className: ({ isActive: v }) => "flex items-center gap-3 px-2.5 py-2 rounded-md text-sm transition-colors " + (v ? "bg-sidebar-primary text-sidebar-primary-foreground font-500" : "text-sidebar-foreground/80 hover:bg-sidebar-accent hover:text-sidebar-accent-foreground"),
                  children: [
                    a.jsx(g.icon, { className: "w-4 h-4 shrink-0" }),
                    g.label
                  ]
                }))
              })
            ]
          })),

          // Workspace utilities
          a.jsxs("div", {
            children: [
              a.jsx("div", { className: "px-2 mb-1.5 text-[11px] font-600 uppercase tracking-wider text-muted-foreground", children: "System" }),
              a.jsx("div", {
                className: "space-y-0.5",
                children: [
                  a.jsxs(td, {
                    to: "/settings",
                    onClick: n,
                    className: ({ isActive: h }) => "flex items-center gap-3 px-2.5 py-2 rounded-md text-sm transition-colors " + (h ? "bg-sidebar-primary text-sidebar-primary-foreground font-500" : "text-sidebar-foreground/80 hover:bg-sidebar-accent hover:text-sidebar-accent-foreground"),
                    children: [
                      a.jsx(rv, { className: "w-4 h-4 shrink-0" }),
                      "Settings"
                    ]
                  }),
                  a.jsxs(td, {
                    to: "/help",
                    onClick: n,
                    className: ({ isActive: h }) => "flex items-center gap-3 px-2.5 py-2 rounded-md text-sm transition-colors " + (h ? "bg-sidebar-primary text-sidebar-primary-foreground font-500" : "text-sidebar-foreground/80 hover:bg-sidebar-accent hover:text-sidebar-accent-foreground"),
                    children: [
                      a.jsx(di, { className: "w-4 h-4 shrink-0" }),
                      "Help & Guide"
                    ]
                  })
                ]
              })
            ]
          })
        ]
      }),

      // Profile footer
      a.jsxs("div", {
        className: "p-3 border-t border-sidebar-border",
        children: [
          a.jsxs("button", {
            onClick: () => { n == null || n(); f("/settings"); },
            className: "w-full inline-flex items-center justify-center gap-2 h-9 rounded-md bg-muted hover:bg-muted/80 text-foreground text-sm font-500 transition-colors border border-border cursor-pointer",
            children: [
              a.jsx(sS, { className: "w-4 h-4" }),
              (u && u.full_name) ? u.full_name : "Researcher Profile"
            ]
          })
        ]
      })
    ]
  });
}
`;

code = code.slice(0, feStart) + replacement + code.slice(beEnd);
fs.writeFileSync(targetFile, code, 'utf8');
console.log('[SUCCESS] Successfully applied senior-level dual-mode navigation to index-CAm4ELM4.js!');
