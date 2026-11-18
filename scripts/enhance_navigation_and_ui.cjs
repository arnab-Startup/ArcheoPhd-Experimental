const fs = require('fs');
const path = require('path');

const targetFile = path.resolve(__dirname, '../dist/assets/index-CAm4ELM4.js');
let code = fs.readFileSync(targetFile, 'utf8');

// 1. Add "Personal Dashboard" as the first option in the Active Project dropdown in BE
const targetDropdown = 'a.jsx(Nx,{children:r.map(h=>a.jsx(Ex,{value:h.id,children:a.jsxs("span",{className:"flex items-center gap-2",children:[h.is_demo&&a.jsx(Zg,{className:"w-3 h-3 text-accent"}),a.jsx("span",{className:"truncate",children:h.title})]})},h.id))})';
const replacementDropdown = 'a.jsxs(Nx,{children:[a.jsx(Ex,{value:"default",children:a.jsx("span",{className:"font-600 text-primary",children:"Personal Dashboard (Home)"})}),...r.map(h=>a.jsx(Ex,{value:h.id,children:a.jsxs("span",{className:"flex items-center gap-2",children:[h.is_demo&&a.jsx(Zg,{className:"w-3 h-3 text-accent"}),a.jsx("span",{className:"truncate",children:h.title})]})},h.id))]})';

if (code.includes(targetDropdown)) {
  code = code.replace(targetDropdown, replacementDropdown);
  console.log('[SUCCESS] Injected "Personal Dashboard" into sidebar project dropdown!');
} else {
  console.warn('[WARNING] Could not find target dropdown string in index-CAm4ELM4.js');
}

// 2. Enhance Wl (Logo) click to reset to Personal Dashboard ("default")
const targetLogo = 'function Wl({compact:n=!1,className:r="",to:o="/"}){return a.jsxs(Ye,{to:o,';
const replacementLogo = 'function Wl({compact:n=!1,className:r="",to:o="/"}){return a.jsxs(Ye,{to:o,onClick:()=>{try{localStorage.setItem("archaeo_active_project","default");window.location.assign("/dashboard");}catch(e){}},';

if (code.includes(targetLogo)) {
  code = code.replace(targetLogo, replacementLogo);
  console.log('[SUCCESS] Enhanced Logo click to cleanly return to Personal Dashboard!');
} else {
  console.warn('[WARNING] Could not find target logo string in index-CAm4ELM4.js');
}

fs.writeFileSync(targetFile, code, 'utf8');
console.log('Finished updating index-CAm4ELM4.js');
