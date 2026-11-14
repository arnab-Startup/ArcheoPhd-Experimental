/**
 * ArchaeoPhD — AI Co-Arguer & Viva Counterargument Engine
 * 
 * Powered by local offline neural model (Qwen 2.5 7B) running 100% on-device.
 * Generates grounded examiner counter-interpretations and defense caveats.
 */

(function () {
  'use strict';

  var AiCoArguer = {
    generateCounterarguments: async function (hypothesisText, sources, projectId) {
      var bridge = window.APD_BRIDGE;
      if (!bridge) {
        return "Bridge client is not initialized.";
      }

      var librarySources = (sources || []).slice(0, 15).map(function (s) {
        return '"' + s.title + '" by ' + (s.author || 'Author') + ' (' + (s.year || 'n.d.') + ')';
      });

      var promptText = 
        "You are ArchaeoPhD AI, an expert archaeological research librarian and viva voce doctoral examiner.\n" +
        "Examine the following thesis hypothesis against the researcher's ingested monograph library:\n\n" +
        "WORKING HYPOTHESIS:\n\"" + hypothesisText + "\"\n\n" +
        "INGESTED PROJECT MONOGRAPHS:\n" + (librarySources.join('\n') || 'None recorded') + "\n\n" +
        "TASK:\n" +
        "1. Identify how a rival scholar or external viva examiner could challenge or dispute this hypothesis.\n" +
        "2. Cite any contradictory stratigraphy, rival ceramic dating, or chronometric disagreements present in academic literature.\n" +
        "3. Provide a defensible rebuttal caveat that the PhD candidate can include in their dissertation.";

      try {
        var res = await bridge.invokeLlm(promptText, projectId);
        return (res && res.answer) ? res.answer : (typeof res === 'string' ? res : JSON.stringify(res, null, 2));
      } catch (err) {
        return "Error during neural synthesis: " + err.message;
      }
    }
  };

  window.AiCoArguer = AiCoArguer;
})();

