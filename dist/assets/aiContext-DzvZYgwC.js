const d=globalThis.__B44_DB__||{entities:new Proxy({},{get:()=>({filter:async()=>[],get:async()=>null,create:async()=>({}),update:async()=>({}),delete:async()=>({})})}),integrations:{Core:{UploadFile:async()=>({file_url:""})}}};function u(r){const{sources:a=[],sites:o=[],artifacts:i=[],claims:s=[],evidence:c=[],notes:l=[]}=r||{},n=[];return a.length&&n.push(`SOURCES:
`+a.slice(0,80).map(e=>{var t;return`- ID:${e.id} | "${e.title}" by ${e.author||"Unknown"} (${e.year||"n.d."}), type: ${e.source_type}${e.publication?`, pub: ${e.publication}`:""}${e.journal?`, journal: ${e.journal}`:""}${e.pages?`, pages: ${e.pages}`:""}${(t=e.keywords)!=null&&t.length?`, keywords: ${e.keywords.join("; ")}`:""}`}).join(`
`)),o.length&&n.push(`SITES:
`+o.slice(0,50).map(e=>`- ID:${e.id} | ${e.site_name} (${e.country||"?"}${e.region?", "+e.region:""}), period: ${e.period||"?"}, type: ${e.site_type||"?"}`).join(`
`)),i.length&&n.push(`ARTIFACTS:
`+i.slice(0,50).map(e=>{var t;return`- ID:${e.id} | ${e.artifact_name} (${e.category}), material: ${e.material||"?"}, period: ${e.period||"?"}, date: ${e.date_range||"?"}${(t=e.site_ids)!=null&&t.length?`, sites: ${e.site_ids.join("; ")}`:""}`}).join(`
`)),s.length&&n.push(`CLAIMS:
`+s.slice(0,50).map(e=>`- ID:${e.id} | ${e.claim_text} [status: ${e.status}]`).join(`
`)),c.length&&n.push(`EVIDENCE:
`+c.slice(0,60).map(e=>{var t;return`- ID:${e.id} | (${e.evidence_type}) for claim ${e.claim_id}: ${e.evidence_text}${e.date_info?` [date: ${e.date_info}]`:""}${(t=e.source_ids)!=null&&t.length?` sources: ${e.source_ids.join("; ")}`:""}`}).join(`
`)),l.length&&n.push(`NOTES:
`+l.slice(0,20).map(e=>`- "${e.title}"`).join(`
`)),n.join(`

`)||"(The research library is currently empty.)"}const p=`ABSOLUTE RULES:
- Never fabricate citations, authors, dates, sites, artifacts, publications, page numbers, DOIs, or interpretations.
- Only use the research library provided below. If information is not present there, do not invent it.
- Clearly separate: (1) evidence found in the provided sources, (2) AI interpretation, (3) missing/uncertain information.
- Reference source IDs/titles when citing. If evidence is insufficient, respond exactly: "Insufficient evidence in your research library."`;async function $(r){try{if(window.__lastSpeechTranscript&&window.__lastSpeechTranscript.trim())return window.__lastSpeechTranscript.trim();if(d&&d.integrations&&d.integrations.Core&&typeof d.integrations.Core.TranscribeAudio==="function"){const a=new File([r],"voice.webm",{type:r.type||"audio/webm"});const up=await d.integrations.Core.UploadFile({file:a});const i=await d.integrations.Core.TranscribeAudio({audio_url:up?up.file_url:""});const txt=typeof i=="string"?i:(i==null?void 0:i.text)||(i==null?void 0:i.transcript)||"";if(txt)return txt;}}catch(e){console.warn(e);}return "";}export{p as G,u as b,$ as t};
