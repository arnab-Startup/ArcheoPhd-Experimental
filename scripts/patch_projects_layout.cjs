const fs = require('fs');
const path = require('path');

const files = [
  path.resolve(__dirname, '../dist/assets/Projects-DhMOSLrW.js'),
  path.resolve(__dirname, '../dist/assets/Projects-DKKRru7w.js')
];

for (const f of files) {
  if (!fs.existsSync(f)) continue;
  let code = fs.readFileSync(f, 'utf8');
  
  // Make sure delete button is not rendered for demo projects (or positioned so it never overlaps the DEMO badge)
  const oldDel = 'e.jsx("button",{onClick:r=>m(r,s),title:"Delete project",className:"absolute top-4 right-4 p-1.5 rounded-md opacity-0 group-hover:opacity-100 hover:bg-destructive/10 text-destructive transition-all z-10",children:e.jsx(w,{className:"w-4 h-4"})})';
  const newDel = '(!s.is_demo)&&e.jsx("button",{onClick:r=>m(r,s),title:"Delete project",className:"absolute top-4 right-4 p-1.5 rounded-md opacity-0 group-hover:opacity-100 hover:bg-destructive/10 text-destructive transition-all z-10",children:e.jsx(w,{className:"w-4 h-4"})})';
  
  if (code.includes(oldDel)) {
    code = code.replace(oldDel, newDel);
    fs.writeFileSync(f, code, 'utf8');
    console.log('Patched delete button collision in:', path.basename(f));
  } else {
    console.log('Delete button pattern not matched in:', path.basename(f));
  }
}
