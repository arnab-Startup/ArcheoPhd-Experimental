const fs = require('fs');
const path = require('path');

const targetFile = path.resolve(__dirname, '../dist/assets/index-CAm4ELM4.js');
let code = fs.readFileSync(targetFile, 'utf8');

console.log('[1/3] Exposing React Router navigator in Hk...');
const hkTarget = 'function Hk(n){let{basename:r,children:o,future:i,window:c}=n,u=m.useRef();u.current==null&&(u.current=H2({window:c,v5Compat:!0}));let f=u.current,';
const hkReplace = 'function Hk(n){let{basename:r,children:o,future:i,window:c}=n,u=m.useRef();u.current==null&&(u.current=H2({window:c,v5Compat:!0}));let f=u.current;try{window.__historyNavigator=f;window.__appNavigate=(t=>f.push(t));}catch(e){}let ';

if (!code.includes(hkTarget)) {
  console.error('Target Hk pattern not found!');
  process.exit(1);
}
code = code.replace(hkTarget, hkReplace);

console.log('[2/3] Adding /index.html and /welcome routes to zP...');
const zpTarget = 'zP=()=>a.jsxs(Dk,{children:[a.jsx(Ne,{path:"/",element:a.jsx(Ok,{to:"/dashboard",replace:!0})}),';
const zpReplace = 'zP=()=>a.jsxs(Dk,{children:[a.jsx(Ne,{path:"/",element:a.jsx(Ok,{to:"/dashboard",replace:!0})}),a.jsx(Ne,{path:"/index.html",element:a.jsx(Ok,{to:"/dashboard",replace:!0})}),a.jsx(Ne,{path:"/welcome",element:a.jsx(Ok,{to:"/dashboard",replace:!0})}),';

if (!code.includes(zpTarget)) {
  console.error('Target zP pattern not found!');
  process.exit(1);
}
code = code.replace(zpTarget, zpReplace);

console.log('[3/3] Fixing Wl logo navigation to use SPA routing...');
const wlTarget = 'function Wl({compact:n=!1,className:r="",to:o="/"}){return a.jsxs(Ye,{to:o,onClick:()=>{try{localStorage.setItem("archaeo_active_project","default");window.location.assign("/dashboard");}catch(e){}}';
const wlReplace = 'function Wl({compact:n=!1,className:r="",to:o="/"}){return a.jsxs(Ye,{to:o,onClick:(e)=>{e&&e.preventDefault&&e.preventDefault();try{localStorage.setItem("archaeo_active_project","default");}catch(e){}if(typeof window.__appNavigate==="function"){window.__appNavigate("/dashboard");}else if(window.__historyNavigator&&window.__historyNavigator.push){window.__historyNavigator.push("/dashboard");}}';

if (!code.includes(wlTarget)) {
  console.error('Target Wl pattern not found!');
  process.exit(1);
}
code = code.replace(wlTarget, wlReplace);

fs.writeFileSync(targetFile, code, 'utf8');
console.log('[SUCCESS] Successfully patched index-CAm4ELM4.js for seamless SPA navigation!');
