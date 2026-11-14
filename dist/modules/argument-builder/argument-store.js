/**
 * ArchaeoPhD — Argument Builder Data Store
 * 
 * 100% On-Device Local Storage for Research Questions and Hypotheses.
 */

(function () {
  'use strict';

  var ArgumentStore = {
    getQuestions: function (projectId) {
      try {
        var raw = localStorage.getItem('archaeophd_rqs_' + projectId);
        if (raw) {
          var arr = JSON.parse(raw);
          if (Array.isArray(arr) && arr.length > 0) return arr;
        }
      } catch(e) {}
      return [
        {
          id: 'rq_default_1',
          project_id: projectId,
          question: 'What was the chronological horizon and primary driver of the City IV destruction at Tell es-Sultan (Jericho)?'
        }
      ];
    },

    saveQuestions: function (projectId, list) {
      try {
        localStorage.setItem('archaeophd_rqs_' + projectId, JSON.stringify(list));
      } catch(e) {}
    },

    addQuestion: function (projectId, questionText) {
      var list = this.getQuestions(projectId);
      var newQ = {
        id: 'rq_' + Date.now(),
        project_id: projectId,
        question: questionText.trim()
      };
      list.push(newQ);
      this.saveQuestions(projectId, list);
      return newQ;
    },

    deleteQuestion: function (projectId, qId) {
      var list = this.getQuestions(projectId).filter(function (q) { return q.id !== qId; });
      this.saveQuestions(projectId, list);
      return list;
    },

    getHypotheses: function (projectId) {
      try {
        var raw = localStorage.getItem('archaeophd_hypotheses_' + projectId);
        if (raw) {
          var arr = JSON.parse(raw);
          if (Array.isArray(arr) && arr.length > 0) return arr;
        }
      } catch(e) {}
      return [
        {
          id: 'hyp_default_1',
          project_id: projectId,
          research_question_id: 'rq_default_1',
          text: 'The catastrophic destruction of City IV occurred in Late Bronze I (c. 1400 BCE), contemporaneous with continuous domestic storage jar assemblages, rather than an Egyptian Middle Bronze campaign (c. 1550 BCE).',
          status: 'contested',
          claim_ids: []
        }
      ];
    },

    saveHypotheses: function (projectId, list) {
      try {
        localStorage.setItem('archaeophd_hypotheses_' + projectId, JSON.stringify(list));
      } catch(e) {}
    },

    addHypothesis: function (projectId, data) {
      var list = this.getHypotheses(projectId);
      var newHyp = {
        id: 'hyp_' + Date.now(),
        project_id: projectId,
        research_question_id: data.research_question_id || undefined,
        text: data.text.trim(),
        status: data.status || 'unresolved',
        claim_ids: data.claim_ids || []
      };
      list.unshift(newHyp);
      this.saveHypotheses(projectId, list);
      return newHyp;
    },

    updateHypothesis: function (projectId, id, patch) {
      var list = this.getHypotheses(projectId);
      var found = list.find(function (h) { return h.id === id; });
      if (found) {
        Object.assign(found, patch);
        this.saveHypotheses(projectId, list);
      }
      return found;
    },

    deleteHypothesis: function (projectId, id) {
      var list = this.getHypotheses(projectId).filter(function (h) { return h.id !== id; });
      this.saveHypotheses(projectId, list);
      return list;
    },

    linkClaim: function (projectId, hypId, claimId) {
      var list = this.getHypotheses(projectId);
      var found = list.find(function (h) { return h.id === hypId; });
      if (found) {
        found.claim_ids = found.claim_ids || [];
        if (found.claim_ids.indexOf(claimId) === -1) {
          found.claim_ids.push(claimId);
          this.saveHypotheses(projectId, list);
        }
      }
      return found;
    },

    unlinkClaim: function (projectId, hypId, claimId) {
      var list = this.getHypotheses(projectId);
      var found = list.find(function (h) { return h.id === hypId; });
      if (found && found.claim_ids) {
        found.claim_ids = found.claim_ids.filter(function(id) { return id !== claimId; });
        this.saveHypotheses(projectId, list);
      }
      return found;
    }
  };

  window.ArgumentStore = ArgumentStore;
})();

