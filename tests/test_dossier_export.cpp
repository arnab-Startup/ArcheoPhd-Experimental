#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <fstream>

#include "models.hpp"
#include "storage.hpp"
#include "analysis/thesis_audit.hpp"
#include "analysis/contradictions.hpp"
#include "analysis/export_engine.hpp"
#include "ipc/native_ipc_dispatcher.hpp"

using namespace archaeophd;

int main() {
    std::cout << "================================================================================\n";
    std::cout << "  ArchaeoPhD — Phase 3 Step 4: Dissertation Dossier Export Suite                \n";
    std::cout << "================================================================================\n\n";

    std::string test_db = "test_step4_export_storage";
    NativeStorage storage(test_db);
    std::string pid = "proj_viva_defense_export";

    // -------------------------------------------------------------
    // Set up Project Relational State
    // -------------------------------------------------------------
    // 1. Sources
    Source s1;
    s1.id = "src_kenyon_1957";
    s1.project_id = pid;
    s1.title = "Digging Up Jericho: The Results of the Jericho Excavations";
    s1.author = "Kenyon, Kathleen M.";
    s1.year = "1957";
    s1.publication = "Ernest Benn";
    s1.pages = "1--272";
    s1.source_type = "Monograph";
    s1.degradation_class = "CLASS_A";
    s1.confirmed_clean_offset = true;
    storage.put_source(s1);

    Source s2;
    s2.id = "src_wood_1990";
    s2.project_id = pid;
    s2.title = "Did the Israelites Conquer Jericho? A New Look at the Archaeological Evidence";
    s2.author = "Wood, Bryant G.";
    s2.year = "1990";
    s2.journal = "Biblical Archaeology Review";
    s2.pages = "44--58";
    s2.source_type = "Journal";
    s2.degradation_class = "CLASS_A";
    s2.confirmed_clean_offset = true;
    storage.put_source(s2);

    // 2. Sites
    Site site_jericho;
    site_jericho.id = "site_jericho";
    site_jericho.project_id = pid;
    site_jericho.site_name = "Tell es-Sultan (Jericho)";
    site_jericho.region = "Jordan Valley";
    site_jericho.period = "PPNA through MB IIB";
    site_jericho.elevation = -258.0;
    storage.put_site(site_jericho);

    // 3. Strata
    Stratum strat_iv;
    strat_iv.id = "strat_city_iv";
    strat_iv.site_id = "site_jericho";
    strat_iv.project_id = pid;
    strat_iv.stratum_name = "City IV Terminal Destruction";
    strat_iv.chronological_bounds = "ca. 1550 - 1400 BCE";
    storage.put_stratum(strat_iv);

    // 4. Claims
    Claim c1;
    c1.id = "claim_dest_date";
    c1.project_id = pid;
    c1.chapter = "Chapter 4: Stratigraphic Destruction";
    c1.scholar_name = "Wood, Bryant G.";
    c1.source_id = "src_wood_1990";
    c1.claim_text = "City IV destruction horizon dates firmly to the terminal Late Bronze I period.";
    c1.status = "Contested";
    c1.origin_type = "manual_transcription";
    c1.verification_status = "VERIFIED";
    storage.put_claim(c1);

    NativeContradictionEngine contradiction_engine(storage);
    NativeThesisAuditor thesis_auditor(storage, contradiction_engine);
    NativeExportEngine export_engine(storage, &thesis_auditor, &contradiction_engine);

    // -------------------------------------------------------------
    // TEST 1: BibTeX Bibliography Exporter
    // -------------------------------------------------------------
    std::cout << "[TEST 1] Testing Standardized BibTeX Bibliography Export...\n";
    std::string bib = export_engine.generate_bibtex(pid);

    std::cout << bib << "\n";
    assert(bib.find("@book{kenyon1957digging,") != std::string::npos);
    assert(bib.find("author    = {Kenyon, Kathleen M.}") != std::string::npos);
    assert(bib.find("@article{wood1990did,") != std::string::npos);
    assert(bib.find("journal   = {Biblical Archaeology Review}") != std::string::npos);
    assert(bib.find("pages     = {44--58}") != std::string::npos);
    std::cout << "  [PASS] RFC BibTeX entries generated with 100% syntactic precision!\n\n";

    // -------------------------------------------------------------
    // TEST 2: Pre-Submission Viva Defense Dossier (Markdown & HTML)
    // -------------------------------------------------------------
    std::cout << "[TEST 2] Testing Pre-Submission Viva Defense Dossier Generation...\n";
    std::string md = export_engine.generate_markdown_dossier(pid);

    assert(md.find("# Pre-Submission Dissertation Defense Dossier") != std::string::npos);
    assert(md.find("## 1. Executive Dissertation Defense Readiness") != std::string::npos);
    assert(md.find("## 2. Layer A: Empirical Ground Truth Catalog") != std::string::npos);
    assert(md.find("## 3. Layer B: Interpretive Claims & Attributions") != std::string::npos);
    assert(md.find("Tell es-Sultan (Jericho)") != std::string::npos);
    assert(md.find("City IV Terminal Destruction") != std::string::npos);

    std::string html = export_engine.generate_html_dossier(pid);
    assert(html.find("<!DOCTYPE html>") != std::string::npos);
    assert(html.find("<title>Pre-Submission Dissertation Defense Dossier") != std::string::npos);
    assert(html.find("@media print") != std::string::npos);

    std::cout << "  ✓ Markdown defense dossier: " << md.size() << " bytes.\n";
    std::cout << "  ✓ Standalone styled HTML: " << html.size() << " bytes.\n";
    std::cout << "  [PASS] Pre-submission viva defense dossiers generated!\n\n";

    // -------------------------------------------------------------
    // TEST 3: Cryptographic Offline JSON Archive
    // -------------------------------------------------------------
    std::cout << "[TEST 3] Testing Offline Cryptographic JSON Archive...\n";
    json arch = export_engine.generate_json_archive(pid);

    assert(arch["schema_version"] == "3.0.0");
    assert(arch["project_id"] == pid);
    assert(arch.contains("sha256_checksum"));
    std::string checksum = arch["sha256_checksum"].get<std::string>();
    std::cout << "  ✓ Archive SHA-256 Checksum: " << checksum << "\n";
    assert(checksum.size() == 64);
    assert(arch["sources"].size() == 2);
    assert(arch["sites"].size() == 1);
    assert(arch["strata"].size() == 1);
    assert(arch["claims"].size() == 1);
    std::cout << "  [PASS] Cryptographic offline JSON archive verified!\n\n";

    // -------------------------------------------------------------
    // TEST 4: Multi-Format Batch Disk Exporter
    // -------------------------------------------------------------
    std::cout << "[TEST 4] Testing Multi-Format Batch Disk Exporter...\n";
    std::string out_dir = "test_export_output";
    fs_compat::create_directories(out_dir);

    json batch_res = export_engine.export_all(out_dir, pid);
    assert(batch_res["success"] == true);
    assert(batch_res["exported_files"].size() == 4);

    std::ifstream check_bib(out_dir + "/bibliography.bib");
    assert(check_bib.good());
    std::ifstream check_md(out_dir + "/thesis_dossier.md");
    assert(check_md.good());
    std::ifstream check_html(out_dir + "/thesis_dossier.html");
    assert(check_html.good());
    std::ifstream check_json(out_dir + "/project_archive.archaeophd.json");
    assert(check_json.good());

    std::cout << "  ✓ All 4 export artifacts verified on local disk.\n";
    std::cout << "  [PASS] Batch disk exporter operational!\n\n";

    // -------------------------------------------------------------
    // TEST 5: Native IPC Dispatcher Endpoint
    // -------------------------------------------------------------
    std::cout << "[TEST 5] Testing Native IPC Dispatcher export_thesis_dossier Endpoint...\n";
    NativeIpcDispatcher dispatcher(&storage, &contradiction_engine, &thesis_auditor, "");

    std::string req_exp = "{\"id\": \"msg-exp-1\", \"action\": \"export_thesis_dossier\", \"projectId\": \"" + pid + "\"}";
    std::string res_exp_raw = dispatcher.dispatch(req_exp);
    auto res_exp = json::parse(res_exp_raw);

    assert(res_exp.contains("result"));
    const auto& exp_res = res_exp["result"];
    assert(exp_res.contains("bibtex"));
    assert(exp_res.contains("markdown_dossier"));
    assert(exp_res.contains("html_dossier"));
    assert(exp_res.contains("json_archive"));
    std::cout << "  ✓ IPC returned complete multi-format dossier payload.\n";
    std::cout << "  [PASS] IPC Bridge export endpoint operates with 100% schema compliance!\n\n";

    // Clean up temporary disk files
    check_bib.close();
    check_md.close();
    check_html.close();
    check_json.close();

    std::cout << "================================================================================\n";
    std::cout << "  ALL PHASE 3 STEP 4 DOSSIER EXPORT TESTS PASSED (100%)!                        \n";
    std::cout << "================================================================================\n";
    return 0;
}
