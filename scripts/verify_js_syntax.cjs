const fs = require('fs');
const path = require('path');
const vm = require('vm');

function checkFile(filePath) {
  const code = fs.readFileSync(filePath, 'utf8');
  try {
    new vm.Script(code, { filename: filePath });
    return true;
  } catch (err) {
    console.error('SYNTAX ERROR in', filePath, ':', err.message);
    return false;
  }
}

function checkDir(dir) {
  let failed = 0;
  const items = fs.readdirSync(dir);
  for (const item of items) {
    const full = path.join(dir, item);
    if (fs.statSync(full).isDirectory()) {
      failed += checkDir(full);
    } else if (item.endsWith('.js') && !item.endsWith('.min.js')) {
      if (!checkFile(full)) failed++;
    }
  }
  return failed;
}

const totalFailures = checkDir(path.resolve(__dirname, '../dist/modules')) +
                     checkDir(path.resolve(__dirname, '../dist/shared'));

console.log('Total syntax errors found:', totalFailures);
process.exit(totalFailures > 0 ? 1 : 0);
