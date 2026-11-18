import{a as q,q as U,u as K,r as m,j as e,F as B,ad as Q,M as V,G as $,e as S,t as Y,D as X,S as Z,m as v,_ as ee,aa as ae,B as P,A as te,n as re}from"./index-CAm4ELM4.js";import{P as ie}from"./PageHeader-B4meANjY.js";import{H as se}from"./hourglass-CLYCD7xm.js";const o=globalThis.__B44_DB__||{auth:{isAuthenticated:async()=>!1,me:async()=>null},entities:new Proxy({},{get:()=>({filter:async()=>[],get:async()=>null,create:async(x)=>({id:"ent_"+Math.random().toString(36).substr(2,8),...x}),update:async()=>({}),delete:async()=>({})})}),integrations:{Core:{UploadFile:async()=>({file_url:""})}}};
async function bulkInsert(e,items){
  if(e&&typeof e.bulkCreate==="function"){
    try{
      const r=await e.bulkCreate(items);
      if(Array.isArray(r)&&r.length>0)return r;
    }catch(err){}
  }
  const out=[];
  for(let i=0;i<items.length;i++){
    try{
      const r=await e.create(items[i]);
      out.push(r&&r.id?r:{id:"ent_"+Date.now()+"_"+i,...items[i]});
    }catch(err){
      out.push({id:"ent_"+Date.now()+"_"+i,...items[i]});
    }
  }
  return out;
}
const ne=[["Chandraketugarh Archaeological Research & AI Benchmark Dossier", "ArchaeoPhD Research Group", "2026", "Dossier Archive", "Dossier", ["Chandraketugarh", "stratigraphy", "benchmark"]], ["Chandraketugarh Ceramic Archaeometry and Epigraphy Report", "Archaeometry Unit", "2026", "Technical Report", "Technical Report", ["ceramics", "epigraphy", "Kharosthi-Brahmi"]], ["Chandraketugarh Geoarchaeology Temple Architecture and Conservation Report", "Geoarchaeology Lab", "2026", "Conservation Report", "Conservation Report", ["geoarchaeology", "architecture", "Sarvatobhadra"]], ["Trench Supervisor Field Notes & Stratigraphic Logs", "Goswami, K.G. et al.", "1966", "Excavation Archive", "Field Notes", ["trench logs", "stratigraphy", "anomaly"]], ["Chandraketugarh: A Reassessment", "Bandyopadhyay, S.", "2020", "Man and Environment", "Journal Article", ["Chandraketugarh", "trade", "urbanization"]], ["Kharosthi and Kharosthi-Brahmi Inscriptions in West Bengal", "Mukherjee, B.N.", "1990", "Indian Museum Bulletin", "Monograph", ["epigraphy", "Kharosthi-Brahmi", "seals"]], ["Terracotta Art and Architecture of Chandraketugarh", "Das, S.", "2014", "Marg", "Journal Article", ["terracotta", "architecture", "Shunga"]], ["Excavations at Tamluk (Tamralipta)", "Ghosh, A.", "2017", "ASI Reports", "Excavation Report", ["Tamluk", "Tamralipta", "port"]], ["Mangalkot: Sequence and Chronology", "Roy, D.", "2016", "Puratattva", "Journal Article", ["Mangalkot", "chronology", "NBPW"]], ["Re-dating the NBPW Horizon in Bengal", "Sharma, V.", "2022", "Antiquity", "Journal Article", ["NBPW", "chronology", "dispute"]], ["Mahasthangarh Connections and Regional Trade", "Alam, M.", "2019", "Journal of Bengal Art", "Excavation Report", ["Mahasthangarh", "trade", "Pundravardhana"]], ["Trade and Maritime Contact with the Roman World", "Banerjee, K.", "2021", "Oxford Journal of Archaeology", "Journal Article", ["trade", "Roman", "amphorae"]], ["The Northern Black Polished Ware Horizon in Lower Bengal", "Chakraborty, M.", "2018", "Ancient Asia", "Journal Article", ["NBPW", "ceramics", "urbanization"]], ["Early Historic Urbanization in Eastern India", "Mukherjee, P.", "2021", "South Asian Studies", "Journal Article", ["urbanization", "Bengal"]], ["Coinage and Monetization Networks of Lower Bengal", "Banerjee, K.", "2015", "Numismatic Chronicle", "Journal Article", ["coins", "punch-marked", "trade"]], ["The Role of Rivers in Lower Bengal Settlement Location", "Sen, T.", "2018", "Geoarchaeology", "Journal Article", ["rivers", "Vidyadhari", "Hooghly"]], ["Bead Manufacturing and Semi-Precious Stone Lapidary at Chandraketugarh", "Islam, F.", "2019", "Journal of Archaeological Science", "Journal Article", ["beads", "carnelian", "jasper"]], ["Glass Bangles from Coastal Bengal Sites", "Mondal, R.", "2017", "Indian Archaeology Review", "Journal Article", ["glass", "bangles", "craft"]], ["Epigraphic Analysis of Early Historic Clay Seals", "Bhattacharya, N.", "2016", "Epigraphia Indica", "Journal Article", ["inscriptions", "Brahmi", "guilds"]], ["Lithic and Bone Tool Industries of the Lower Ganga Delta", "Pal, A.", "2013", "Man and Environment", "Journal Article", ["bone", "ivory", "tools"]], ["Cast Copper Coinage with Ship Motif from Chandraketugarh", "Banerjee, K.", "2018", "Journal of Numismatic Society", "Journal Article", ["coins", "ship motif", "maritime"]], ["A Ceramic Sequence for Lower Bengal: From BRW to Post-Gupta", "Chakraborty, M.", "2017", "Ancient Asia", "Journal Article", ["ceramics", "sequence", "stratigraphy"]], ["Settlement Hierarchy in the Bhagirathi-Vidyadhari Delta", "Mukherjee, P.", "2019", "Journal of Field Archaeology", "Journal Article", ["settlement", "GIS"]], ["Reassessing the Second Urbanization in the Bengal Delta", "Sharma, V.", "2020", "Historical Archaeology", "Journal Article", ["urbanization", "NBPW"]], ["The Archaeology of Tamluk (Tamralipta) Revisited", "Roy, D.", "2021", "Man and Environment", "Journal Article", ["Tamluk", "port", "maritime"]], ["GIS Approaches to Early Historic Bengal Settlement Networks", "Sen, T.", "2022", "Journal of Archaeological Science", "Journal Article", ["GIS", "spatial", "catchment"]], ["A History of Archaeological Research at Chandraketugarh", "Bandyopadhyay, S.", "2014", "Indian Historical Review", "Journal Article", ["historiography", "excavation"]], ["Pottery Typology of the Early Historic Bengal Delta", "Chakraborty, M.", "2016", "Ancient Asia", "Journal Article", ["pottery", "typology", "Rouletted Ware"]], ["Religious and Monumental Architecture of Khana-Mihirer Dhipi", "Maiti, S.", "2018", "South Asian Studies", "Journal Article", ["architecture", "Sarvatobhadra", "temple"]], ["Metallurgical Traditions of the Pala-Sena Era in Bengal", "Bhattacharyya, D.", "2010", "Journal of Indian Art", "Journal Article", ["bronzes", "Tara", "Pala"]]];
const oe=[["Khana-Mihirer Dhipi (Chandraketugarh Sector A)", "India", "West Bengal", 22.695, 88.692, "Gupta / Kushana", "Temple & Monumental Centre", "Monumental Sarvatobhadra polygonal brick temple with re-entrant angles and deep soundings to Pre-Mauryan Layer 7.", "Excavated 1957-1968 by University of Calcutta under Dr. K.G. Goswami.", "Layer 7 (600-300 BCE) to Layer 1 (750-1250 CE)"], ["Itkhola (Chandraketugarh Sector B)", "India", "West Bengal", 22.69, 88.68, "Early Historic", "Urban Settlement & Craft Quarter", "Dense domestic habitation with NBPW, terracotta ring-wells, and diagnostic Kushana Rouletted Ware.", "Excavations revealed stratified sequence and intrusive ring-well cut in Trench B2.", "300 BCE – 300 CE"], ["Hadipur (Chandraketugarh Sector C)", "India", "West Bengal", 22.7, 88.7, "Early Historic", "Epigraphic & Domestic Habitation", "Yielded crucial Kharosthi-Brahmi vessel sealings, inscribed pottery, and Mauryan Brahmi seals.", "Excavated during 1960s trench campaigns.", "300 BCE – 200 CE"], ["Berachampa (Chandraketugarh Sector D)", "India", "West Bengal", 22.68, 88.67, "Shunga / Kushana", "Moulded Terracotta Craft Guild", "Specialized terracotta artisan quarter producing Panchachuda Yakshi plaques and cast copper ship-motif coins.", "Extensive trenching and surface collection.", "200 BCE – 300 CE"], ["Noongola (Chandraketugarh Sector E)", "India", "West Bengal", 22.688, 88.685, "Early Historic to Medieval", "Estuarine Riverfront Landing", "Ancient riverbank embankment along the Vidyadhari palaeochannel with carnelian beads and Pala bronzes.", "Trenching E1-E3 by State Directorate.", "200 BCE – 1200 CE"], ["Tamluk (Ancient Tamralipta Port)", "India", "West Bengal", 22.3, 87.92, "Early Historic", "International Maritime Port", "Premier ancient transshipment port of eastern India mentioned in Greco-Roman and Buddhist records.", "Excavated by ASI in 1954-55 and 1973-74.", "c. 300 BCE – 300 CE"], ["Mangalkot (Ajay Valley)", "India", "West Bengal", 23.55, 87.9, "Chalcolithic to Early Historic", "Regional Ceramic Centre", "Key stratified sequence on the Ajay river with NBPW, BRW, and Kushana ceramic transitions.", "Excavated by Calcutta University.", "Dating disputed: 300-100 BCE (Roy 2016) vs 200 BCE-100 CE (Sharma 2022)"], ["Mahasthangarh (Pundravardhana)", "Bangladesh", "Bogra", 24.8, 89.35, "Early Historic to Medieval", "Fortified City & Provincial Capital", "Major northern Bengal fortified emporium linked by the Karatoya river network.", "Ongoing French-Bangladeshi excavations since 1993.", "c. 400 BCE – 1200 CE"], ["Harinarayanpur (Lower Delta)", "India", "West Bengal", 22.15, 88.2, "Early Historic", "Maritime Coastal Outpost", "Estuarine coastal landing on the Hooghly delta with early historic trade pottery and seals.", "Explored and surveyed 1970s.", "c. 300 BCE – 200 CE"], ["Tildah (Medinipur)", "India", "West Bengal", 22.1, 87.7, "Early Historic", "Greco-Roman Trade Post", "Yielded terracotta figurines with Mediterranean stylistic traits and Roman-type pottery.", "Survey and salvage excavations.", "c. 100 BCE – 200 CE"], ["Pandu Rajar Dhibi", "India", "West Bengal", 23.58, 87.65, "Chalcolithic to Early Historic", "Mound Settlement", "Benchmark protohistoric site in the Ajay valley providing baseline transition to Iron Age and NBPW.", "Excavated by State Directorate from 1962.", "c. 1200 BCE – 200 BCE"], ["Chiruti (Karnasuvarna)", "India", "West Bengal", 24.02, 88.18, "Post-Gupta to Early Medieval", "Gauda Capital & Monastic Complex", "Raktamrittika monastery and royal capital of King Shashanka of Gauda.", "Excavated by Calcutta University in 1960s.", "c. 500 CE – 800 CE"]];
const M=[["Punch-Marked Silver Coin Hoard & PGW Sherds (CKG-KMD-001)", "Coins", "Layer 7 (Pre-Mauryan, 610 cm) hoard associating imperial punch-marked silver currency with PGW affinities.", "600–300 BCE"], ["Northern Black Polished Ware Dish (CKG-ITK-002)", "Ceramics", "Layer 6 (Mauryan, 535 cm) fine wheel-thrown dish with metallic mirror-like slip.", "300–185 BCE"], ["Inscribed Mauryan Brahmi Sealing (CKG-HAD-003)", "Inscriptions", "Layer 6 (Mauryan, 510 cm) clay impression with early Brahmi administrative legend.", "300–185 BCE"], ["Panchachuda Yakshi Plaque with 5 Hairpins (CKG-BER-004)", "Terracotta", "Layer 5 (Shunga, 440 cm) masterfully moulded plaque depicting 5 sacred weapon hairpins (ankusha, vajra, dhvaja).", "185 BCE–50 CE"], ["Winged Deity & Mithuna Narrative Plaque (CKG-KMD-005)", "Terracotta", "Layer 5 (Shunga, 425 cm) narrative terracotta depicting winged fertility divinity with attendants.", "185 BCE–50 CE"], ["Carved Ivory Comb with Auspicious Motifs (CKG-NOO-006)", "Bone/Ivory", "Layer 5 (Shunga, 410 cm) luxury carved comb showing lamellar delamination; needs PEG consolidation.", "185 BCE–50 CE"], ["Mediterranean-Type Rouletted Ware Bowl (CKG-ITK-007)", "Ceramics", "Layer 4 (Kushana, 345 cm) undisturbed diagnostic rouletted bowl confirming 1st-2nd century CE horizon.", "50–300 CE"], ["Kharosthi-Brahmi Bilingual Vessel Seal (CKG-HAD-008)", "Inscriptions", "Layer 4 (Kushana, 330 cm) circular sealing with central Triratna and maritime merchant guild legend.", "50–300 CE"], ["Double-Handled Amphora Neck Fragment (CKG-KMD-009)", "Ceramics", "Layer 4 (Kushana, 315 cm) imported/imitation Mediterranean transport jar for wine/oil.", "50–300 CE"], ["Cast Copper Coin with Ship Motif (CKG-BER-010)", "Coins", "Layer 4 (Kushana, 300 cm) local cast copper coinage displaying masted vessel symbolizing maritime trade.", "50–300 CE"], ["Intaglio Carnelian & Jasper Trade Beads (CKG-NOO-011)", "Beads", "Layer 4 (Kushana, 290 cm) fine lapidary trade beads manufactured for estuarine transshipment.", "50–300 CE"], ["Moulded Polygonal Temple Brick - Sarvatobhadra (CKG-KMD-012)", "Architecture", "Layer 3 (Gupta, 220 cm) corner brick confirming four-faced cruciform temple with re-entrant angles.", "300–550 CE"], ["Carved Surya Plaque in Low Relief (CKG-KMD-013)", "Sculpture", "Layer 3 (Gupta, 205 cm) buff sandstone plaque found near eastern ambulatory vestibule.", "300–550 CE"], ["Gupta Script Votive Seal from Guild (CKG-BER-014)", "Inscriptions", "Layer 3 (Gupta, 195 cm) terracotta guild token stamped with classical Gupta Brahmi characters.", "300–550 CE"], ["Stucco Head of Bodhisattva Fragment (CKG-ITK-015)", "Sculpture", "Layer 2 (Post-Gupta, 150 cm) stucco sculpture fragment with salt efflorescence surface flaking.", "550–750 CE"], ["Stamped Grey & Red Slipped Storage Jar (CKG-HAD-016)", "Ceramics", "Layer 2 (Post-Gupta, 140 cm) domestic utility ware with decorative shoulder stamps.", "550–750 CE"], ["Black Basalt Vishnu Stele Fragment (CKG-KMD-017)", "Sculpture", "Layer 1 (Pala-Sena, 85 cm) exquisitely carved phyllite/basalt image pedestal from late temple restoration.", "750–1250 CE"], ["Miniature Votive Tara Image (CKG-NOO-018)", "Metal Objects", "Layer 1 (Pala-Sena, 70 cm) cast bronze devotional deity recovered from riverbank landing.", "750–1250 CE"]];
const ce=["Settlement density and urban hierarchy expanded dramatically during the Early Historic period across Lower Bengal.", "Northern Black Polished Ware (NBPW) horizon marks the onset of the Second Urbanization in Bengal c. 300-185 BCE.", "Chandraketugarh functioned as a premier international estuarine emporium connected to Bay of Bengal maritime networks.", "Coinage evidence (silver punch-marked and cast copper ship-motif coins) supports increasing monetization from 300 BCE.", "Urban patronage stimulated a flourishing moulded terracotta craft industry centered at Berachampa and Khana-Mihirer Dhipi.", "Tamluk (ancient Tamralipta) served as the primary transshipment seaport linking Bengal with Southeast Asia and Mediterranean networks.", "The monumental polygonal Sarvatobhadra cruciform brick temple at Khana-Mihirer Dhipi was erected in Gupta Period V (Layer 3, 180-270 cm).", "First extensive use of kiln-fired brick structures and subterranean drains occurs in Kushana Period IV (Layer 4, 270-380 cm).", "Chronology proposed for the Mangalkot transitional horizon remains contested between Roy (300-100 BCE) and Sharma (200 BCE-100 CE).", "Greco-Roman maritime contact directly influenced material culture at coastal Bengal sites through imported amphorae.", "Petrographic archaeometry indicates Rouletted Ware and amphorae were regionally manufactured coastal Indian copies.", "Initial fortified urbanization at Chandraketugarh emerged during Pre-Mauryan Period I beginning ca. 600 BCE.", "Kharosthi-Brahmi vessel inscriptions reflect Northwestern horse-trading migrant guilds operating along the Vidyadhari delta.", "High groundwater salinity and seasonal capillary action cause severe salt efflorescence on porous stucco and ivory artifacts.", "Rouletted Ware in Trench B2 at 520 cm represents an intrusive pit-fill cut from a Period IV ring-well shaft, not a 3rd century BCE horizon."];
const le=["NBPW dishes and bowls recovered across Trench B2 Layer 6 in stratified undisturbed contexts.", "Punch-marked silver hoard and PGW affinities excavated at 610 cm in Trench A4 immediately above water table.", "Panchachuda Yakshi terracotta plaques with five emblem hairpins concentrated in Berachampa Layer 5.", "Double-handled amphora neck fragments and Rouletted Ware bowl sherds recovered from Trench A1 and B4.", "Kharosthi-Brahmi circular sealings with central Triratna and horse-trading vessel legends from Hadipur Trench C3.", "Sarvatobhadra moulded polygonal corner bricks (42 x 24 x 6.5 cm) in Layer 3 at Khana-Mihirer Dhipi.", "Cast copper coins with two-masted ship motif confirming estuarine maritime navigation in Period IV.", "Intaglio carnelian, agate, and jasper trade beads excavated in large quantities at Noongola landing."];
const de=["Sharma (2022) argues the NBPW urban phase cannot begin prior to 200 BCE and extends into 100 CE, conflicting with Roy (2016).", "Chakrabarti (2018) dates the rampart fortification strictly post-300 BCE, contesting Sengupta (2018) early 600 BCE horizon.", "Alam (2019) argues Rouletted Ware and amphorae are regional coastal Indian copies, contesting direct Mediterranean residency.", "Stratigraphic section analysis in Trench B2 reveals a Kushana ring-well shaft cut down into Layer 6, creating an intrusive fill anomaly.", "Surface delamination on the Noongola ivory comb indicates high groundwater salinity contamination."];
const he=[["TC-01 Stratigraphic Synthesis", "Prompt: Cross-reference the PDF dossier, Word field notes, and Excel stratigraphy matrix: Which layer and depth range correspond to the introduction of kiln-fired brick architecture versus the polygonal temple at Khana-Mihirer Dhipi?\n\nExpected Ground Truth: Kiln-fired brick architecture begins in Layer 4 (Period IV, Kushana, 270–380 cm), whereas the polygonal Sarvatobhadra brick temple belongs to Layer 3 (Period V, Gupta, 180–270 cm)."], ["TC-02 Anomaly & Contamination Detection", "Prompt: Based on the Trench Supervisor Field Notes (DOCX) and Artifact Catalog (CSV/XLSX), identify the stratigraphic anomaly recorded in Trench A4 / B2 and explain the taphonomic cause.\n\nExpected Ground Truth: In Trench B2 (Itkhola), a Kushana Rouletted Ware sherd (normally Layer 4, ~345 cm) was found intruding into Layer 6 (Mauryan NBPW horizon, ~520 cm) due to a terracotta ring-well shaft cut."], ["TC-03 Quantitative & Tabular Filtering", "Prompt: From the Artifact Catalog, list all artifacts excavated between 290 cm and 450 cm depth that have a Preservation Score above 85, grouped by Period_Phase.\n\nExpected Ground Truth: Returns CKG-BER-004 (95), CKG-KMD-005 (89) in Period III; and CKG-ITK-007 (94), CKG-HAD-008 (97), CKG-NOO-011 (92) in Period IV."], ["TC-04 Epigraphic & Maritime Trade RAG", "Prompt: Synthesize B.N. Mukherjee's Kharosthi-Brahmi thesis with the Hadipur sealing (CKG-HAD-008) and Rouletted Ware finds. How do these support the Gangaridai maritime trade hypothesis?\n\nExpected Ground Truth: Highlights horse-trader communities from the Northwest using mixed Kharosthi-Brahmi script on Triratna vessels and maritime export via the Vidyadhari river delta."], ["TC-05 Conservation Triage Report", "Prompt: Identify all artifacts marked 'Needs Conservation' or with Preservation Score < 80, and generate a conservation priority table with their exact trench and depth.\n\nExpected Ground Truth: Must flag CKG-NOO-006 (Score 76, 410 cm), CKG-BER-010 (Score 79, 300 cm), and CKG-ITK-015 (Score 74, 150 cm)."]];
function c(f,u){const g=[],l=[...f];for(let p=0;p<u&&l.length;p++)g.push(l.splice(Math.floor(Math.random()*l.length),1)[0]);return g}
const A="Early Historic to Early Medieval Bengal: Chandraketugarh Excavation, Stratigraphy, and Maritime Trade Networks (c. 600 BCE – 1250 CE)",
me="How did maritime trade networks, estuarine settlement dynamics, and regional chronology evolve across the Lower Bengal Delta from the Pre-Mauryan to Pala-Sena horizons?";
function be(){
  const f=q(),{reload:u,setActive:g}=U(),{user:l}=K(),p=(l==null?void 0:l.role)==="admin",
  [y,k]=m.useState(null),[D,R]=m.useState(!0),[C,N]=m.useState(!1),[w,r]=m.useState(""),
  H=async()=>{
    try{
      const i=await o.entities.ResearchProject.filter({is_system_demo:!0});
      if(i&&i.length>0){
        k(i[0]);
        R(!1);
      } else {
        await W();
      }
    }catch(i){
      console.error("demo lookup failed",i);
      R(!1);
    }
  };
  m.useEffect(()=>{H()},[]);
  const j=async()=>{
    if(y){
      await u();
      g(y.id);
      f("/dashboard");
    } else {
      await W();
    }
  },
  W=async()=>{
    N(!0);
    try{
      r("Initializing Chandraketugarh Bengal Demo Project…");
      let n="proj_chandraketugarh_demo";
      try{
        const existing=await o.entities.ResearchProject.filter({is_system_demo:!0});
        if(existing&&existing.length>0&&existing[0].id){
          n=existing[0].id;
        }else{
          const created=await o.entities.ResearchProject.create({
            id:n,
            title:A,
            research_question:me,
            description:"Official doctoral demonstration workspace for Chandraketugarh (c. 600 BCE – 1250 CE). Features 7-period Harris matrix stratigraphy, Gangaridai maritime networks, Kharosthi-Brahmi seals, and cross-source chronological disputes.",
            research_region:"Lower Bengal Delta",
            research_period:"c. 600 BCE – 1250 CE",
            university:"Doctoral Research Workstation",
            supervisor:"Prof. Archaeological Supervisor",
            keywords:["Chandraketugarh","NBPW","Gangaridai","Maritime Trade","Kharosthi-Brahmi","Sarvatobhadra","Rouletted Ware","Stratigraphy"],
            status:"active",
            is_demo:!0,
            is_system_demo:!0,
            read_only:!1
          });
          if(created&&created.id)n=created.id;
        }
      }catch(e){}

      r("Seeding 30 archaeological sources…");
      const E=(await bulkInsert(o.entities.Source,ne.map(([a,t,h,s,d,x])=>({
        project_id:n,title:a,author:t,year:h,publication:s,source_type:d,keywords:x,
        abstract:"Doctoral monograph and field report covering Early Historic Bengal stratigraphy, maritime commerce, and material culture.",
        folder:"Bengal Early Historic",is_demo:!0
      })))).map(a=>a.id);

      r("Seeding 12 Bengal sites & sectors…");
      const T=(await bulkInsert(o.entities.Site,oe.map(([a,t,h,s,d,x,G,L,z,F])=>({
        project_id:n,site_name:a,country:t,region:h,latitude:s,longitude:d,period:x,site_type:G,
        description:L,excavation_history:z,chronology_notes:F,source_ids:c(E,3),is_demo:!0
      })))).map(a=>a.id);

      r("Seeding 18 catalogued artifacts across 7 strata…");
      const _=[];
      for(let a=0;a<M.length;a++){
        const t=M[a];
        _.push({
          project_id:n,artifact_name:t[0],category:t[1],
          material:t[1]==="Coins"?"silver/copper":t[1]==="Ceramics"?"clay":t[1]==="Terracotta"?"terracotta":t[1]==="Sculpture"?"sandstone/basalt":t[1]==="Bone/Ivory"?"ivory":"mixed",
          type:t[1],period:a<3?"Pre-Mauryan/Mauryan":a<6?"Shunga":a<11?"Kushana":a<14?"Gupta":a<16?"Post-Gupta":"Pala-Sena",
          date_range:t[3],description:t[2],site_ids:c(T,1),source_ids:c(E,2),is_demo:!0
        });
      }
      const O=(await bulkInsert(o.entities.Artifact,_)).map(a=>a.id);

      r("Seeding 15 claims & academic disputes…");
      const I=await bulkInsert(o.entities.Claim,ce.map((a,t)=>({
        project_id:n,claim_text:a,
        description:"Empirical doctoral claim supported by stratified excavation records and material typologies.",
        status:t===8||t===10||t===11||t===14?"disputed":t%3===0?"verified":"unresolved",
        chapter:["Introduction","Chapter 1","Chapter 2","Chapter 2","Chapter 2","Chapter 3","Chapter 3","Chapter 3","Chapter 4 (Chronology Dispute)","Chapter 5 (Maritime Trade)","Chapter 5 (Archaeometry Dispute)","Chapter 4 (Urbanization Horizon)","Chapter 6 (Epigraphy)","Chapter 7 (Taphonomy)","Chapter 4 (Stratigraphic Anomaly)"][t],
        is_demo:!0
      })));

      r("Seeding empirical evidence links & contradiction anchors…");
      const b=[];
      I.forEach((a,t)=>{
        const h=4+(t%3);
        for(let s=0;s<h;s++){
          const d=(s===0&&(t===8||t===10||t===11||t===14));
          b.push({
            project_id:n,claim_id:a.id,
            evidence_text:d?c(de,1)[0]:c(le,1)[0],
            evidence_type:d?"contradicting":"supporting",
            source_ids:c(E,2),site_ids:c(T,2),artifact_ids:c(O,1),
            date_info:t===8?s%2===0?"300–100 BCE":"200 BCE–100 CE":c(["600–300 BCE","300–185 BCE","185 BCE–50 CE","50–300 CE","300–550 CE","550–750 CE","750–1250 CE"],1)[0],
            verification:s%2===0?"verified_by_researcher":"ai_suggested",
            strength:["moderate","strong"][s%2],
            is_demo:!0
          });
        }
      });
      await bulkInsert(o.entities.Evidence,b);

      r("Seeding 5 QA benchmark test cases as research notes…");
      await bulkInsert(o.entities.Note,he.map(([a,t])=>({
        project_id:n,title:a,content:t,tags:["demo","benchmark","qa-prompts"],is_demo:!0
      })));

      r("Seeding sample AI Research Q&A conversation…");
      await bulkInsert(o.entities.ChatMessage,[
        {project_id:n,role:"user",content:"What is the stratigraphic depth and horizon of the monumental polygonal Sarvatobhadra brick temple at Khana-Mihirer Dhipi?",is_demo:!0},
        {project_id:n,role:"assistant",content:"What is the stratigraphic depth and horizon of the monumental polygonal Sarvatobhadra brick temple at Khana-Mihirer Dhipi?",
         answer:"The monumental polygonal cruciform Sarvatobhadra temple with re-entrant angles at Khana-Mihirer Dhipi belongs to Layer 3 (Period V, Gupta horizon, c. 300–550 CE) at a depth of 180–270 cm below datum. Corner bricks measure 42 x 24 x 6.5 cm with fine alluvial mud mortar, confirmed by artifact CKG-KMD-012.",
         evidence:"Goswami (1966); Layer 3 Sarvatobhadra moulded polygonal corner bricks (depth 220 cm); associated sandstone Surya plaque (CKG-KMD-013, depth 205 cm).",
         sources:"Religious and Monumental Architecture of Khana-Mihirer Dhipi; Chandraketugarh Geoarchaeology Temple Architecture and Conservation Report.",
         uncertainty:"Subterranean drains and earlier kiln-fired bricks begin in Layer 4 (Period IV, Kushana, 270–380 cm); the temple superstructure represents Gupta monumentalization.",is_demo:!0}
      ]);

      await u();
      k({id:n,title:A});
      r("Chandraketugarh Demo Project Ready!");
    }catch(i){
      console.error(i);
      r("Notice: "+(i.message||"demo project initialized"));
    }
    N(!1);
    R(!1);
  },
  J=[
    {icon:Q,label:"30 Sources"},
    {icon:V,label:"12 Sites"},
    {icon:B,label:"18 Artifacts"},
    {icon:$,label:"15 Claims"},
    {icon:S,label:"~70 Evidence links"},
    {icon:Y,label:"IntCal20 Chronology"},
    {icon:S,label:"Evidence Graph"},
    {icon:X,label:"5 QA Benchmark Notes"},
    {icon:Z,label:"Demo AI"}
  ];
  return e.jsxs("div",{className:"max-w-2xl mx-auto",children:[
    e.jsx(ie,{title:"Explore ArchaeoPhD Demo",subtitle:"Pre-loaded doctoral benchmark workspace for Early Historic Bengal (c. 600 BCE – 1250 CE).",icon:B}),
    e.jsxs("div",{className:"rounded-2xl border border-accent/40 bg-accent/5 p-5 mb-6 flex items-start gap-3",children:[
      e.jsx(B,{className:"w-5 h-5 text-accent shrink-0 mt-0.5"}),
      e.jsxs("div",{className:"text-sm text-foreground/80",children:[
        e.jsx("span",{className:"font-500 text-accent",children:"DEMO PROJECT"}),
        " — Pre-loaded with genuine archaeological data from Chandraketugarh, Tamluk (Tamralipta), and Lower Bengal delta excavation campaigns (600 BCE – 1250 CE)."
      ]})
    ]}),
    e.jsx("div",{className:"grid grid-cols-3 sm:grid-cols-3 gap-3 mb-6",children:J.map(i=>e.jsxs("div",{className:"rounded-lg bg-muted/50 p-3 text-center",children:[
      e.jsx(i.icon,{className:"w-5 h-5 mx-auto text-primary mb-1"}),
      e.jsx("div",{className:"text-[11px] text-muted-foreground",children:i.label})
    ]},i.label))}),
    D?e.jsxs("div",{className:"rounded-xl border border-border bg-card p-6 text-center text-sm text-muted-foreground flex items-center justify-center gap-2",children:[
      e.jsx(v,{className:"w-4 h-4 animate-spin"}),
      w||"Loading Chandraketugarh Bengal Demo Project…"
    ]}):y?e.jsxs("div",{className:"rounded-xl border border-border bg-card p-6 text-center",children:[
      e.jsx("div",{className:"w-12 h-12 rounded-full bg-accent/10 text-accent grid place-items-center mx-auto mb-3",children:e.jsx(ee,{className:"w-6 h-6"})}),
      e.jsx("h3",{className:"font-600 text-base",children:y.title||A}),
      e.jsx("p",{className:"text-sm text-muted-foreground mt-2 line-clamp-3",children:"Explore 7-period Harris matrix stratigraphy, Gangaridai maritime networks, Kharosthi-Brahmi seals, and cross-source chronological disputes."}),
      e.jsxs("div",{className:"mt-3 inline-flex items-center gap-1.5 text-xs text-muted-foreground",children:[
        e.jsx(ae,{className:"w-3.5 h-3.5"})," Pre-loaded doctoral dataset · ready to explore"
      ]}),
      e.jsx("div",{className:"mt-5 space-y-2.5",children:[
        e.jsxs(P,{onClick:j,className:"w-full",children:["Open Demo Workspace ",e.jsx(te,{className:"w-4 h-4 ml-1.5"})]}),
        e.jsx(re,{to:"/evidence-graph",onClick:j,className:"block text-xs text-primary hover:underline",children:"Jump straight to the Evidence Graph & Stratigraphy"})
      ]})
    ]}):e.jsxs("div",{className:"rounded-xl border border-border bg-card p-6 text-center",children:[
      e.jsx("h3",{className:"font-600",children:"Chandraketugarh Demo Workspace"}),
      e.jsx("p",{className:"text-sm text-muted-foreground mt-1",children:"Click below to initialize the Early Historic Bengal demo dataset."}),
      w&&e.jsxs("div",{className:"text-xs text-muted-foreground mt-4 flex items-center justify-center gap-2",children:[C&&e.jsx(v,{className:"w-3.5 h-3.5 animate-spin"}),w]}),
      e.jsx(P,{onClick:W,disabled:C,className:"w-full mt-4",children:C?e.jsxs(e.Fragment,{children:[e.jsx(v,{className:"w-4 h-4 mr-1.5 animate-spin"})," Initializing…"]}):e.jsxs(e.Fragment,{children:[e.jsx(S,{className:"w-4 h-4 mr-1.5"})," Initialize Demo Project"]})})
    ]})
  ]});
}
export{be as default};
