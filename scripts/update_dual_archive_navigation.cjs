const fs = require('fs');
const path = require('path');

const filesToPatch = [
  path.resolve(__dirname, '../dist/assets/index-CAm4ELM4.js'),
  path.resolve(__dirname, '../release/dist/assets/index-CAm4ELM4.js'),
  path.resolve(__dirname, '../../desktop/dist/assets/index-CAm4ELM4.js'),
  path.resolve(__dirname, '../../desktop/release/dist/assets/index-CAm4ELM4.js')
];

let allSuccess = true;

for (const filePath of filesToPatch) {
  if (!fs.existsSync(filePath)) {
    console.error('File not found:', filePath);
    allSuccess = false;
    continue;
  }
  let content = fs.readFileSync(filePath, 'utf8');
  if (content.includes('Documentation & Literature')) {
    console.log('Already patched:', filePath);
    continue;
  }
  
  // Match group: "Project Documentation" block
  const regex = /group:\s*"Project Documentation",\s*items:\s*\[\r?\n\s*\{\s*to:\s*"\/library",\s*label:\s*"Field Reports & Sources",\s*icon:\s*gb\s*\},\r?\n\s*\{\s*to:\s*"\/analytics",\s*label:\s*"Excavation Analytics",\s*icon:\s*Pm\s*\}\r?\n\s*\]/g;
  
  if (!regex.test(content)) {
    console.error('Regex pattern not found in:', filePath);
    allSuccess = false;
    continue;
  }

  const isCRLF = content.includes('\r\n');
  const eol = isCRLF ? '\r\n' : '\n';

  const replacement = [
    'group: "Documentation & Literature", items: [',
    '      { to: "/library", label: "Field Reports & Library", icon: gb },',
    '      { to: "/literature-graph", label: "Citation Network", icon: b1 },',
    '      { to: "/export-center", label: "Export Center", icon: Wr },',
    '      { to: "/analytics", label: "Excavation Analytics", icon: Pm }',
    '    ]'
  ].join(eol);

  content = content.replace(regex, replacement);
  fs.writeFileSync(filePath, content, 'utf8');
  console.log('Successfully patched:', filePath);
}

if (!allSuccess) {
  process.exit(1);
}
console.log('All index bundle files patched successfully.');
