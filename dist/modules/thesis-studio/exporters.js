/**
 * ArchaeoPhD — Thesis Studio Academic Exporters
 * 
 * Generates LaTeX (.tex), Markdown (.md), and Print-to-PDF output.
 */

(function () {
  'use strict';

  var ThesisExporters = {
    htmlToMarkdown: function (html) {
      var temp = document.createElement('div');
      temp.innerHTML = html;
      var md = '';
      for (var i = 0; i < temp.childNodes.length; i++) {
        var node = temp.childNodes[i];
        var tag = node.nodeName.toLowerCase();
        if (tag === 'h1') md += '# ' + node.textContent.trim() + '\n\n';
        else if (tag === 'h2') md += '## ' + node.textContent.trim() + '\n\n';
        else if (tag === 'h3') md += '### ' + node.textContent.trim() + '\n\n';
        else if (tag === 'p') md += node.textContent.trim() + '\n\n';
        else if (tag === 'blockquote') md += '> ' + node.textContent.trim() + '\n\n';
        else if (tag === 'figure') {
          var img = node.querySelector('img');
          var cap = node.querySelector('figcaption');
          var capText = cap ? cap.textContent.trim() : 'Archaeological Plate';
          var src = img ? img.getAttribute('src') : '';
          md += '![' + capText + '](' + src + ')\n*' + capText + '*\n\n';
        } else if (tag === 'ul') {
          var lis = node.querySelectorAll('li');
          lis.forEach(function(li) { md += '- ' + li.textContent.trim() + '\n'; });
          md += '\n';
        } else if (tag === 'ol') {
          var ols = node.querySelectorAll('li');
          ols.forEach(function(li, idx) { md += (idx + 1) + '. ' + li.textContent.trim() + '\n'; });
          md += '\n';
        } else if (node.textContent.trim()) {
          md += node.textContent.trim() + '\n\n';
        }
      }
      return md.trim();
    },

    htmlToLatex: function (html, chapterTitle) {
      var md = this.htmlToMarkdown(html);
      var body = md
        .replace(/^# (.*$)/gim, '\\chapter{$1}')
        .replace(/^## (.*$)/gim, '\\section{$1}')
        .replace(/^### (.*$)/gim, '\\subsection{$1}')
        .replace(/> (.*$)/gim, '\\begin{quote}\n$1\n\\end{quote}')
        .replace(/\*\*(.*?)\*\*/g, '\\textbf{$1}')
        .replace(/\*(.*?)\*/g, '\\textit{$1}')
        .replace(/_(.*?)_/g, '\\textit{$1}')
        .replace(/\[\^(.*?)\]/g, '\\footnote{$1}');

      return `\\documentclass[12pt,a4paper,oneside]{report}
\\usepackage[utf8]{inputenc}
\\usepackage[margin=1in]{geometry}
\\usepackage{setspace}
\\usepackage{booktabs}
\\usepackage{graphicx}
\\usepackage{natbib}
\\usepackage{microtype}
\\doublespacing

\\title{${chapterTitle}}
\\author{PhD Researcher}
\\date{\\today}

\\begin{document}
\\maketitle

${body}

\\end{document}`;
    },

    downloadFile: function (filename, content, mimeType) {
      var blob = new Blob([content], { type: mimeType });
      var url = URL.createObjectURL(blob);
      var a = document.createElement('a');
      a.href = url;
      a.download = filename;
      a.click();
      URL.revokeObjectURL(url);
    }
  };

  window.ThesisExporters = ThesisExporters;
})();

