const fs = require('fs');
const path = require('path');

const dir = path.resolve(__dirname, '../dist/assets');
const allFiles = fs.readdirSync(dir);

const pages = [
  'Onboarding-VLDP_GQU.js',
  'Dashboard-W5Q9sdSC.js',
  'Projects-DhMOSLrW.js',
  'CreateProject-CN7WyzuW.js',
  'Library-Cb_z35xS.js',
  'SourceReader-a5KpWJa1.js',
  'AIResearch-DOXJDKD0.js',
  'Claims-mjqd1ajI.js',
  'ClaimDetail-LiIeFb9W.js',
  'EvidenceGraph-Cu3n-egu.js',
  'Sites-SFt3yj3b.js',
  'SiteDetail-CPz6R0rP.js',
  'Artifacts-DiTpKEte.js',
  'Chronology-CSm9kFUB.js',
  'ResearchMap-BTAiZ-3T.js',
  'Thesis-DnYRuPCF.js',
  'Notes-nQP7GZiS.js',
  'Settings-I7FVGIWz.js',
  'DemoProject-CMll_CGB.js',
  'ResearchInbox-6J2IZLzS.js',
  'SiteComparison-C3MeyDfb.js',
  'ContradictionDetector-Ch_RuSNf.js',
  'ResearchGapExplorer-BR0imvKZ.js',
  'ArgumentBuilder-DoZSBFMh.js',
  'LiteratureGraph-BwbmQgqs.js',
  'ArchaeologicalMatrix-DZPWDBeg.js',
  'VisualizationCenter--gEiW6Ev.js',
  'ResearchAnalytics-BdFd-JJM.js',
  'ActivitySummary-D1Dr08RM.js',
  'ExportCenter-DJJMcP3Y.js',
  'Notifications-tvsUb81I.js',
  'HelpCenter-bXJhmygY.js',
  'Blog-BHZC5K1h.js',
  'BlogPost-LW717IAn.js'
];

const pageSet = new Set(pages);
const core = ['index-CAm4ELM4.js', 'index-CeTrMfRQ.css'];
const coreSet = new Set(core);

const componentsAndIcons = [];
const others = [];

for (const f of allFiles) {
  if (pageSet.has(f) || coreSet.has(f)) continue;
  if (f.endsWith('.png') || f.endsWith('.jpg') || f.endsWith('.svg')) {
    others.push(f);
  } else {
    componentsAndIcons.push(f);
  }
}

console.log('Pages (' + pages.length + '):');
console.log(pages);
console.log('\nComponents & Shared Chunks (' + componentsAndIcons.length + '):');
console.log(componentsAndIcons);
console.log('\nCore & Assets (' + (core.length + others.length) + '):');
console.log([...core, ...others]);
