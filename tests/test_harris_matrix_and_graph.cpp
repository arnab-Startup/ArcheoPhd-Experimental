#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <json.hpp>

#include "storage.hpp"
#include "analysis/harris_matrix.hpp"
#include "ipc/native_ipc_dispatcher.hpp"

using namespace archaeophd;
using json = nlohmann::json;
namespace fs = std::filesystem;

void run_test(const std::string& name, bool condition) {
    if (condition) {
        std::cout << "  [PASS] " << name << "\n";
    } else {
        std::cerr << "  [FAIL] " << name << "\n";
        exit(1);
    }
}

int main() {
    std::cout << "================================================================================\n";
    std::cout << "  ArchaeoPhD Engine — Hardened Phase 2 Stratigraphic DAG & Graph Suite          \n";
    std::cout << "================================================================================\n\n";

    std::string test_dir = "test_graph_data";
    std::error_code ec;
    fs::remove_all(test_dir, ec);

    // -------------------------------------------------------------------------
    // TEST 1: Normal Stratigraphic Sequence & Harris Matrix DAG
    // -------------------------------------------------------------------------
    std::cout << "[TEST 1] Stratigraphic DAG Construction & Topological Sequence...\n";
    {
        std::vector<Stratum> strata;
        
        Stratum s_bedrock;
        s_bedrock.id = "strat_bedrock";
        s_bedrock.stratum_name = "Bedrock & PPNA Tower";
        s_bedrock.site_id = "site_jericho";
        s_bedrock.date_start_bce = 8500;
        s_bedrock.date_end_bce = 7500;
        s_bedrock.harris_above = {"strat_ppnb"};
        strata.push_back(s_bedrock);

        Stratum s_ppnb;
        s_ppnb.id = "strat_ppnb";
        s_ppnb.stratum_name = "PPNB Plastered Skull Domestic Floors";
        s_ppnb.site_id = "site_jericho";
        s_ppnb.date_start_bce = 7500;
        s_ppnb.date_end_bce = 6000;
        s_ppnb.harris_above = {"strat_mba_city_iv"};
        s_ppnb.harris_below = {"strat_bedrock"};
        strata.push_back(s_ppnb);

        Stratum s_mba;
        s_mba.id = "strat_mba_city_iv";
        s_mba.stratum_name = "Middle Bronze Age City IV Domestic & Fortification";
        s_mba.site_id = "site_jericho";
        s_mba.date_start_bce = 1650;
        s_mba.date_end_bce = 1550;
        s_mba.harris_above = {"strat_lba_burn"};
        s_mba.harris_below = {"strat_ppnb"};
        strata.push_back(s_mba);

        Stratum s_lba;
        s_lba.id = "strat_lba_burn";
        s_lba.stratum_name = "Late Bronze Age Terminal Conflagration Horizon";
        s_lba.site_id = "site_jericho";
        s_lba.date_start_bce = 1450;
        s_lba.date_end_bce = 1400;
        s_lba.harris_below = {"strat_mba_city_iv"};
        strata.push_back(s_lba);

        auto matrix = HarrisMatrixEngine::build_matrix(strata);
        run_test("Matrix is a valid DAG (no cycles)", matrix.is_valid_dag);
        run_test("Topological sequence has 4 elements", matrix.chronological_sequence.size() == 4);
        run_test("First in sequence is earliest (strat_bedrock)", matrix.chronological_sequence[0] == "strat_bedrock");
        run_test("Second in sequence is strat_ppnb", matrix.chronological_sequence[1] == "strat_ppnb");
        run_test("Third in sequence is strat_mba_city_iv", matrix.chronological_sequence[2] == "strat_mba_city_iv");
        run_test("Fourth in sequence is latest (strat_lba_burn)", matrix.chronological_sequence[3] == "strat_lba_burn");

        run_test("Bedrock level is 0", matrix.stratigraphic_levels["strat_bedrock"] == 0);
        run_test("PPNB level is 1", matrix.stratigraphic_levels["strat_ppnb"] == 1);
        run_test("MBA level is 2", matrix.stratigraphic_levels["strat_mba_city_iv"] == 2);
        run_test("LBA level is 3", matrix.stratigraphic_levels["strat_lba_burn"] == 3);
        run_test("Zero inversions in coherent sequence", matrix.inversions.empty());
    }

    // -------------------------------------------------------------------------
    // TEST 2: Stratigraphic Paradox & Cycle Detection Suite
    // -------------------------------------------------------------------------
    std::cout << "\n[TEST 2A] Length-3 Cycle Detection (A -> B -> C -> A)...\n";
    {
        std::vector<Stratum> strata;
        Stratum s1; s1.id = "strat_A"; s1.harris_above = {"strat_B"}; strata.push_back(s1);
        Stratum s2; s2.id = "strat_B"; s2.harris_above = {"strat_C"}; strata.push_back(s2);
        Stratum s3; s3.id = "strat_C"; s3.harris_above = {"strat_A"}; strata.push_back(s3);

        auto matrix = HarrisMatrixEngine::build_matrix(strata);
        run_test("Length-3 cyclic sequence identified as NOT a valid DAG", !matrix.is_valid_dag);
        run_test("At least 1 cycle isolated by Tarjan SCC", !matrix.cycles.empty());
        run_test("Cycle contains 3 vertices", matrix.cycles[0].size() == 3);
        run_test("Topological sequence is strictly empty on cycle", matrix.chronological_sequence.empty());
    }

    std::cout << "\n[TEST 2B] Length-1 Self-Loop Detection (A -> A) & Downstream Abort...\n";
    {
        std::vector<Stratum> strata;
        Stratum s1;
        s1.id = "strat_self";
        s1.stratum_name = "Self-Referential Deposit";
        s1.harris_above = {"strat_self"}; // Erroneous self-loop: feature recorded above itself
        strata.push_back(s1);

        auto matrix = HarrisMatrixEngine::build_matrix(strata);
        run_test("Self-loop identified as NOT a valid DAG", !matrix.is_valid_dag);
        run_test("Self-loop cycle isolated in cycles array", matrix.cycles.size() == 1);
        run_test("Self-loop component contains exactly strat_self", matrix.cycles[0][0] == "strat_self");
        run_test("Downstream Kahn topological sort did NOT produce partial sequence", matrix.chronological_sequence.empty());
    }

    std::cout << "\n[TEST 2C] Length-2 Mutual Cycle Detection (A <-> B)...\n";
    {
        std::vector<Stratum> strata;
        Stratum s1; s1.id = "strat_X"; s1.harris_above = {"strat_Y"}; strata.push_back(s1);
        Stratum s2; s2.id = "strat_Y"; s2.harris_above = {"strat_X"}; strata.push_back(s2);

        auto matrix = HarrisMatrixEngine::build_matrix(strata);
        run_test("Length-2 mutual cycle identified as NOT a valid DAG", !matrix.is_valid_dag);
        run_test("Cycle isolated contains 2 vertices", matrix.cycles.size() == 1 && matrix.cycles[0].size() == 2);
        run_test("Topological sequence is empty", matrix.chronological_sequence.empty());
    }

    std::cout << "\n[TEST 2D] Multiple Disconnected Independent Cycles...\n";
    {
        std::vector<Stratum> strata;
        // Component 1: X <-> Y
        Stratum s1; s1.id = "strat_comp1_x"; s1.harris_above = {"strat_comp1_y"}; strata.push_back(s1);
        Stratum s2; s2.id = "strat_comp1_y"; s2.harris_above = {"strat_comp1_x"}; strata.push_back(s2);
        // Component 2: W -> Z -> W
        Stratum s3; s3.id = "strat_comp2_w"; s3.harris_above = {"strat_comp2_z"}; strata.push_back(s3);
        Stratum s4; s4.id = "strat_comp2_z"; s4.harris_above = {"strat_comp2_w"}; strata.push_back(s4);

        auto matrix = HarrisMatrixEngine::build_matrix(strata);
        run_test("Disconnected multi-cycle graph identified as NOT valid DAG", !matrix.is_valid_dag);
        run_test("Both independent cycles isolated by Tarjan SCC", matrix.cycles.size() == 2);
        run_test("Topological sequence is empty", matrix.chronological_sequence.empty());
    }

    // -------------------------------------------------------------------------
    // TEST 3: Law of Superposition & Astronomical Year Suite
    // -------------------------------------------------------------------------
    std::cout << "\n[TEST 3A] Classical BCE Law of Superposition Inversion...\n";
    {
        std::vector<Stratum> strata;
        Stratum s_lower;
        s_lower.id = "strat_lower";
        s_lower.stratum_name = "Lower Level Trench VI";
        s_lower.date_start_bce = 1400; // 1400 BCE
        s_lower.date_end_bce = 1350;
        s_lower.harris_above = {"strat_upper"};
        strata.push_back(s_lower);

        Stratum s_upper;
        s_upper.id = "strat_upper";
        s_upper.stratum_name = "Upper Level Trench VI";
        s_upper.date_start_bce = 1800; // Upper layer dated 1800 BCE (400 years older than lower layer!)
        s_upper.date_end_bce = 1750;
        strata.push_back(s_upper);

        auto matrix = HarrisMatrixEngine::build_matrix(strata);
        run_test("DAG topology remains structurally valid", matrix.is_valid_dag);
        run_test("Inversion detected", matrix.inversions.size() == 1);
        run_test("Inversion is marked as advisory review", matrix.inversions[0].is_advisory);
        run_test("Inversion flags upper stratum correctly", matrix.inversions[0].upper_stratum_id == "strat_upper");
        run_test("Inversion flags lower stratum correctly", matrix.inversions[0].lower_stratum_id == "strat_lower");
        run_test("Reason cites Stratigraphic date discrepancy", matrix.inversions[0].reason.find("Stratigraphic date discrepancy") != std::string::npos);
    }

    std::cout << "\n[TEST 3B] Astronomical Year 1 BCE / 1 CE Boundary Test (Zero-Year Trap Defense)...\n";
    {
        // Boundary Part 1: Stratum 1 (1 BCE) directly below Stratum 2 (1 CE) -> Valid sequence!
        {
            std::vector<Stratum> strata;
            Stratum s1;
            s1.id = "strat_1bce";
            s1.stratum_name = "Late Hellenistic / Hasmonaean";
            s1.date_start_bce = 1; // 1 BCE -> Y_astro = 0
            s1.date_end_bce = 1;
            s1.harris_above = {"strat_1ce"};
            strata.push_back(s1);

            Stratum s2;
            s2.id = "strat_1ce";
            s2.stratum_name = "Early Roman / Augustan";
            s2.date_start_ce = 1;  // 1 CE -> Y_astro = 1
            s2.date_end_ce = 1;
            strata.push_back(s2);

            auto matrix = HarrisMatrixEngine::build_matrix(strata);
            run_test("1 BCE below 1 CE is a valid sequence", matrix.is_valid_dag);
            run_test("No inversion on 1 BCE -> 1 CE boundary (astro 0 <= 1)", matrix.inversions.empty());
        }

        // Boundary Part 2: Stratum 1 (1 BCE) below Stratum 2 (2 BCE) -> Upper layer is 1 year older!
        {
            std::vector<Stratum> strata;
            Stratum s1;
            s1.id = "strat_1bce";
            s1.stratum_name = "Late 1 BCE Deposit";
            s1.date_start_bce = 1; // 1 BCE -> Y_astro = 0
            s1.date_end_bce = 1;
            s1.harris_above = {"strat_2bce"};
            strata.push_back(s1);

            Stratum s2;
            s2.id = "strat_2bce";
            s2.stratum_name = "2 BCE Fill Deposit";
            s2.date_start_bce = 2; // 2 BCE -> Y_astro = -1 (Older than 1 BCE!)
            s2.date_end_bce = 2;
            strata.push_back(s2);

            auto matrix = HarrisMatrixEngine::build_matrix(strata);
            run_test("1 BCE below 2 BCE correctly flags boundary inversion", matrix.inversions.size() == 1);
            run_test("Upper stratum flagged as 2 BCE", matrix.inversions[0].upper_stratum_id == "strat_2bce");
            run_test("Lower stratum flagged as 1 BCE", matrix.inversions[0].lower_stratum_id == "strat_1bce");
        }
    }

    std::cout << "\n[TEST 3C] Common Era (CE) Superposition Inversion...\n";
    {
        std::vector<Stratum> strata;
        Stratum s_lower;
        s_lower.id = "strat_late_roman";
        s_lower.stratum_name = "Constantinian Floor";
        s_lower.date_start_ce = 320; // 320 CE
        s_lower.date_end_ce = 350;
        s_lower.harris_above = {"strat_early_roman"};
        strata.push_back(s_lower);

        Stratum s_upper;
        s_upper.id = "strat_early_roman";
        s_upper.stratum_name = "Trajanic Sump";
        s_upper.date_start_ce = 110; // 110 CE (Older than 320 CE beneath it!)
        s_upper.date_end_ce = 130;
        strata.push_back(s_upper);

        auto matrix = HarrisMatrixEngine::build_matrix(strata);
        run_test("CE date inversion detected", matrix.inversions.size() == 1);
        run_test("Flags early Roman as upper inverted stratum", matrix.inversions[0].upper_stratum_id == "strat_early_roman");
    }

    // -------------------------------------------------------------------------
    // TEST 4: Radiometric C-14 Calibrated Range Suite
    // -------------------------------------------------------------------------
    std::cout << "\n[TEST 4A] C-14 Overlapping 2-Sigma Range (Valid Stratigraphic Prior)...\n";
    {
        std::vector<Stratum> strata;
        Stratum s_base; s_base.id = "strat_layer2"; s_base.stratum_name = "Layer 2"; s_base.harris_above = {"strat_layer3"}; strata.push_back(s_base);
        Stratum s_top; s_top.id = "strat_layer3"; s_top.stratum_name = "Layer 3"; strata.push_back(s_top);

        std::vector<Sample> samples;
        // Layer 2: 1520 - 1410 cal BCE
        Sample smp_lower;
        smp_lower.id = "smp_01";
        smp_lower.lab_code = "OXA-5011";
        smp_lower.stratum_id = "strat_layer2";
        smp_lower.date_cal_start_bce = 1520;
        smp_lower.date_cal_end_bce = 1410;
        smp_lower.cal_range_2sigma = "1520-1410 cal BCE";
        samples.push_back(smp_lower);

        // Layer 3: 1440 - 1380 cal BCE (Overlaps with Layer 2 between 1440 and 1410 BCE!)
        Sample smp_upper;
        smp_upper.id = "smp_02";
        smp_upper.lab_code = "OXA-5012";
        smp_upper.stratum_id = "strat_layer3";
        smp_upper.date_cal_start_bce = 1440;
        smp_upper.date_cal_end_bce = 1380;
        smp_upper.cal_range_2sigma = "1440-1380 cal BCE";
        samples.push_back(smp_upper);

        auto matrix = HarrisMatrixEngine::build_matrix(strata, samples);
        run_test("Overlapping 2-sigma calibrated ranges do NOT produce false positive inversion", matrix.inversions.empty());
    }

    std::cout << "\n[TEST 4B] C-14 Strict Non-Overlap (Advisory Review Flag)...\n";
    {
        std::vector<Stratum> strata;
        Stratum s_base; s_base.id = "strat_base"; s_base.stratum_name = "Basal Silts"; s_base.harris_above = {"strat_top"}; strata.push_back(s_base);
        Stratum s_top; s_top.id = "strat_top"; s_top.stratum_name = "Surface Fill"; strata.push_back(s_top);

        std::vector<Sample> samples;
        // Lower sample: 1500 - 1450 cal BCE
        Sample smp_lower;
        smp_lower.id = "smp_01";
        smp_lower.lab_code = "OXA-4021";
        smp_lower.stratum_id = "strat_base";
        smp_lower.date_cal_start_bce = 1500;
        smp_lower.date_cal_end_bce = 1450;
        smp_lower.cal_range_2sigma = "1500-1450 cal BCE";
        samples.push_back(smp_lower);

        // Upper sample: 2200 - 2100 cal BCE (Strictly older without overlap!)
        Sample smp_upper;
        smp_upper.id = "smp_02";
        smp_upper.lab_code = "OXA-4022";
        smp_upper.stratum_id = "strat_top";
        smp_upper.date_cal_start_bce = 2200;
        smp_upper.date_cal_end_bce = 2100;
        smp_upper.cal_range_2sigma = "2200-2100 cal BCE";
        samples.push_back(smp_upper);

        auto matrix = HarrisMatrixEngine::build_matrix(strata, samples);
        run_test("Strict non-overlap triggers inversion entry", matrix.inversions.size() == 1);
        run_test("Inversion is marked as advisory (is_advisory == true)", matrix.inversions[0].is_advisory);
        run_test("Cites upper lab code OXA-4022", matrix.inversions[0].sample_upper_code == "OXA-4022");
        run_test("Cites lower lab code OXA-4021", matrix.inversions[0].sample_lower_code == "OXA-4021");
        run_test("Reason contains 'Possible redeposition or context error — please review'", 
                 matrix.inversions[0].reason.find("Possible redeposition or context error — please review") != std::string::npos);
        run_test("DAG topology remains valid despite advisory C-14 inversion", matrix.is_valid_dag);
    }

    // -------------------------------------------------------------------------
    // TEST 5: Relational Cross-Referencing in NativeStorage
    // -------------------------------------------------------------------------
    std::cout << "\n[TEST 5A] Relational Cross-Referencing in NativeStorage...\n";
    {
        NativeStorage storage(test_dir);

        Site site;
        site.id = "site_jericho_01";
        site.site_name = "Tell es-Sultan (Jericho)";
        storage.put_site(site);

        Stratum st;
        st.id = "strat_jericho_city_iv";
        st.site_id = "site_jericho_01";
        st.stratum_name = "City IV Destruction Layer";
        storage.put_stratum(st);

        Artifact art;
        art.id = "art_bichrome_01";
        art.stratum_id = "strat_jericho_city_iv";
        art.artifact_name = "Cypriot Bichrome Wheel-made Jug";
        storage.put_artifact(art);

        Sample smp;
        smp.id = "smp_cereal_01";
        smp.stratum_id = "strat_jericho_city_iv";
        smp.lab_code = "RT-1340";
        storage.put_sample(smp);

        Claim c1;
        c1.id = "claim_wood_1990_01";
        c1.site_ids = {"site_jericho_01"};
        c1.strata_ids = {"strat_jericho_city_iv"};
        c1.claim_text = "City IV collapsed in Late Bronze I (ca. 1400 BC) based on Cypriot imports.";
        c1.scholar_name = "Bryant Wood";
        storage.put_claim(c1);

        EvidenceLink ev;
        ev.id = "ev_01";
        ev.claim_id = "claim_wood_1990_01";
        ev.physical_entity_type = "Artifact";
        ev.physical_entity_id = "art_bichrome_01";
        storage.put_evidence(ev);

        // Test cross-referencing getters
        auto claimsSite = storage.get_claims_by_site("site_jericho_01");
        run_test("get_claims_by_site returned 1 claim", claimsSite.size() == 1);
        run_test("Returned claim is claim_wood_1990_01", claimsSite[0].id == "claim_wood_1990_01");

        auto claimsStratum = storage.get_claims_by_stratum("strat_jericho_city_iv");
        run_test("get_claims_by_stratum returned 1 claim", claimsStratum.size() == 1);

        auto strataSite = storage.get_strata_by_site("site_jericho_01");
        run_test("get_strata_by_site returned 1 stratum", strataSite.size() == 1);

        auto artifactsStratum = storage.get_artifacts_by_stratum("strat_jericho_city_iv");
        run_test("get_artifacts_by_stratum returned 1 artifact", artifactsStratum.size() == 1);

        auto samplesStratum = storage.get_samples_by_stratum("strat_jericho_city_iv");
        run_test("get_samples_by_stratum returned 1 sample", samplesStratum.size() == 1);

        auto evidenceClaim = storage.get_evidence_by_claim("claim_wood_1990_01");
        run_test("get_evidence_by_claim returned 1 evidence link", evidenceClaim.size() == 1);

        // Subgraph extraction
        auto subSite = storage.get_entity_subgraph("site", "site_jericho_01");
        run_test("Site subgraph contains site record", subSite.contains("site"));
        run_test("Site subgraph contains strata array", subSite["strata"].is_array() && subSite["strata"].size() == 1);
        run_test("Site subgraph contains claims array", subSite["claims"].is_array() && subSite["claims"].size() == 1);

        auto subStratum = storage.get_entity_subgraph("stratum", "strat_jericho_city_iv");
        run_test("Stratum subgraph contains stratum record", subStratum.contains("stratum"));
        run_test("Stratum subgraph contains parent site", subStratum.contains("site"));
        run_test("Stratum subgraph contains artifacts array", subStratum["artifacts"].is_array() && subStratum["artifacts"].size() == 1);
        run_test("Stratum subgraph contains samples array", subStratum["samples"].is_array() && subStratum["samples"].size() == 1);
    }

    std::cout << "\n[TEST 5B] Schema Migration & Backward Compatibility (Legacy State Without CE Fields)...\n";
    {
        std::string legacy_dir = "test_legacy_schema_data";
        fs::remove_all(legacy_dir, ec);
        fs::create_directories(legacy_dir);

        // Construct legacy JSON payload omitting date_start_ce and date_end_ce
        json legacyJson;
        legacyJson["sites"] = json::array({
            {{"id", "site_legacy"}, {"project_id", "default"}, {"site_name", "Legacy Bronze Site"}}
        });
        legacyJson["strata"] = json::array({
            {
                {"id", "strat_legacy_01"},
                {"project_id", "default"},
                {"site_id", "site_legacy"},
                {"stratum_name", "Legacy Stratum Without CE Fields"},
                {"date_start_bce", 1200},
                {"date_end_bce", 1100}
                // Notice: date_start_ce and date_end_ce are intentionally omitted!
            }
        });
        legacyJson["samples"] = json::array({
            {
                {"id", "smp_legacy_01"},
                {"project_id", "default"},
                {"site_id", "site_legacy"},
                {"stratum_id", "strat_legacy_01"},
                {"lab_code", "LEG-101"},
                {"date_cal_start_bce", 1180},
                {"date_cal_end_bce", 1120}
                // date_cal_start_ce and date_cal_end_ce omitted!
            }
        });
        legacyJson["artifacts"] = json::array();
        legacyJson["claims"] = json::array();
        legacyJson["evidence"] = json::array();
        legacyJson["sources"] = json::array();
        legacyJson["notes"] = json::array();
        legacyJson["verification_items"] = json::array();

        std::ofstream outState(legacy_dir + "/relational_state.json");
        outState << legacyJson.dump(2);
        outState.close();

        // Load into NativeStorage and assert backward compatibility
        NativeStorage storage(legacy_dir);
        auto strata = storage.get_strata();
        run_test("Legacy state loaded successfully", strata.size() == 1);
        run_test("Legacy stratum name preserved", strata[0].stratum_name == "Legacy Stratum Without CE Fields");
        run_test("Legacy BCE date preserved", strata[0].date_start_bce == 1200);
        run_test("New date_start_ce defaulted safely to 0", strata[0].date_start_ce == 0);
        run_test("New date_end_ce defaulted safely to 0", strata[0].date_end_ce == 0);

        auto samples = storage.get_samples();
        run_test("Legacy sample loaded successfully", samples.size() == 1);
        run_test("New date_cal_start_ce defaulted safely to 0", samples[0].date_cal_start_ce == 0);

        // Test save round-trip
        storage.save_state();
        NativeStorage storageReload(legacy_dir);
        auto reloadedStrata = storageReload.get_strata();
        run_test("Round-trip save/load preserves legacy record", reloadedStrata.size() == 1);
        run_test("Round-trip preserves BCE date", reloadedStrata[0].date_start_bce == 1200);

        fs::remove_all(legacy_dir, ec);
    }

    // -------------------------------------------------------------------------
    // TEST 6: IPC Dispatcher Graph & Compound Query Suite
    // -------------------------------------------------------------------------
    std::cout << "\n[TEST 6A] IPC build_harris_matrix & get_entity_subgraph Endpoints...\n";
    {
        NativeStorage storage(test_dir);
        NativeIpcDispatcher dispatcher(&storage, nullptr, nullptr, test_dir);

        Stratum h1;
        h1.id = "strat_hazor_x";
        h1.site_id = "site_hazor";
        h1.stratum_name = "Stratum X (Solomonic Six-Chamber Gate)";
        h1.harris_above = {"strat_hazor_ix"};
        storage.put_stratum(h1);

        Stratum h2;
        h2.id = "strat_hazor_ix";
        h2.site_id = "site_hazor";
        h2.stratum_name = "Stratum IX (Ahab Casemate Reconstruction)";
        storage.put_stratum(h2);

        // Test build_harris_matrix over IPC
        json reqMatrix = {
            {"id", "req-hm-01"},
            {"action", "build_harris_matrix"},
            {"payload", {{"site_id", "site_hazor"}}}
        };
        std::string resMatrixStr = dispatcher.dispatch(reqMatrix.dump());
        json resMatrix = json::parse(resMatrixStr);
        run_test("IPC build_harris_matrix response has no error", !resMatrix.contains("error"));
        run_test("IPC build_harris_matrix is_valid_dag == true", resMatrix["result"]["is_valid_dag"] == true);
        run_test("IPC build_harris_matrix sequence length == 2", resMatrix["result"]["chronological_sequence"].size() == 2);
        run_test("IPC build_harris_matrix earliest is Stratum X", resMatrix["result"]["chronological_sequence"][0] == "strat_hazor_x");

        // Test get_entity_subgraph over IPC
        json reqSub = {
            {"id", "req-sub-01"},
            {"action", "get_entity_subgraph"},
            {"payload", {{"entity_type", "stratum"}, {"entity_id", "strat_hazor_x"}}}
        };
        std::string resSubStr = dispatcher.dispatch(reqSub.dump());
        json resSub = json::parse(resSubStr);
        run_test("IPC get_entity_subgraph response has no error", !resSub.contains("error"));
        run_test("IPC get_entity_subgraph contains stratum", resSub["result"].contains("stratum"));
        run_test("IPC get_entity_subgraph returned correct stratum name", resSub["result"]["stratum"]["stratum_name"] == "Stratum X (Solomonic Six-Chamber Gate)");
    }

    std::cout << "\n[TEST 6B] IPC query_knowledge_graph Uninitialized Model Failure Path...\n";
    {
        NativeStorage storage(test_dir);
        NativeIpcDispatcher dispatcher(&storage, nullptr, nullptr, test_dir);

#ifdef ARCHAEOPHD_ENABLE_TEST_STUB
        // Explicitly ensure mock mode is disabled to simulate uninitialized weights
        EmbeddingEngine::instance().enable_test_mock_mode(false);
#endif
        EmbeddingEngine::instance().shutdown();

        json reqQuery = {
            {"id", "req-qkg-uninit"},
            {"action", "query_knowledge_graph"},
            {"payload", {{"query", "Solomonic gate architecture"}, {"site_id", "site_hazor"}}}
        };
        std::string resQueryStr = dispatcher.dispatch(reqQuery.dump());
        json resQuery = json::parse(resQueryStr);

        run_test("IPC query_knowledge_graph surfaces error when model uninitialized", resQuery.contains("error"));
        run_test("IPC query_knowledge_graph returns error_code == 'MODEL_NOT_INITIALIZED'", 
                 resQuery.value("error_code", "") == "MODEL_NOT_INITIALIZED");
        run_test("Error message explains weights not loaded", 
                 resQuery["error"].get<std::string>().find("MODEL_NOT_INITIALIZED") != std::string::npos);
    }

    std::cout << "\n[TEST 6C] IPC query_knowledge_graph Compound Join & Site/Stratum Filtering...\n";
    {
        NativeStorage storage(test_dir);
        NativeIpcDispatcher dispatcher(&storage, nullptr, nullptr, test_dir);

#ifdef ARCHAEOPHD_ENABLE_TEST_STUB
        EmbeddingEngine::instance().enable_test_mock_mode(true);
#endif

        // Ingest sample text chunks into storage
        storage.store_compressed_chunk("chunk_hazor_01", "Monumental six-chambered gatehouse uncovered in Stratum X upper city.");
        storage.store_compressed_chunk("chunk_jericho_01", "Catastrophic collapse of the Middle Bronze Age City IV mudbrick wall.");
        
        auto emb1 = VectorIndex::embed_text("Monumental six-chambered gatehouse uncovered in Stratum X upper city.");
        auto emb2 = VectorIndex::embed_text("Catastrophic collapse of the Middle Bronze Age City IV mudbrick wall.");
        storage.vectors().insert("chunk_hazor_01", "doc_hazor_1972", 73, emb1);
        storage.vectors().insert("chunk_jericho_01", "doc_kenyon_1981", 102, emb2);
        storage.lexical().insert("chunk_hazor_01", "doc_hazor_1972", 73, "Monumental six-chambered gatehouse uncovered in Stratum X upper city.");
        storage.lexical().insert("chunk_jericho_01", "doc_kenyon_1981", 102, "Catastrophic collapse of the Middle Bronze Age City IV mudbrick wall.");

        // Add claims with site and stratum linkage
        Claim c_hazor;
        c_hazor.id = "claim_hazor_gate";
        c_hazor.site_ids = {"site_hazor"};
        c_hazor.strata_ids = {"strat_hazor_x"};
        c_hazor.claim_text = "Stratum X gate represents Solomonic royal administrative architecture.";
        storage.put_claim(c_hazor);

        Claim c_jericho;
        c_jericho.id = "claim_jericho_iv";
        c_jericho.site_ids = {"site_jericho"};
        c_jericho.strata_ids = {"strat_jericho_city_iv"};
        c_jericho.claim_text = "City IV wall collapsed due to earthquake prior to final incineration.";
        storage.put_claim(c_jericho);

        // Query 1: Query site_hazor
        json reqHazor = {
            {"id", "req-qkg-hazor"},
            {"action", "query_knowledge_graph"},
            {"payload", {{"query", "six-chambered gatehouse"}, {"site_id", "site_hazor"}}}
        };
        std::string resHazorStr = dispatcher.dispatch(reqHazor.dump());
        json resHazor = json::parse(resHazorStr);

        run_test("Compound query has no error when embedding ready", !resHazor.contains("error"));
        run_test("Compound query returns passages array", resHazor["result"]["passages"].is_array() && !resHazor["result"]["passages"].empty());
        run_test("Compound query filtered claims to exactly site_hazor", resHazor["result"]["claims"].size() == 1);
        run_test("Returned claim is claim_hazor_gate", resHazor["result"]["claims"][0]["id"] == "claim_hazor_gate");

        // Query 2: Query unrelated site (site_megiddo) -> should return passages from search, but 0 matching claims
        json reqMegiddo = {
            {"id", "req-qkg-megiddo"},
            {"action", "query_knowledge_graph"},
            {"payload", {{"query", "six-chambered gatehouse"}, {"site_id", "site_megiddo"}}}
        };
        std::string resMegiddoStr = dispatcher.dispatch(reqMegiddo.dump());
        json resMegiddo = json::parse(resMegiddoStr);
        run_test("Querying unrelated site returns zero claims", resHazor["result"]["claims"].is_array() && resMegiddo["result"]["claims"].empty());
    }

    fs::remove_all(test_dir, ec);

    std::cout << "\n================================================================================\n";
    std::cout << "  ALL HARDENED STRATIGRAPHIC DAG & KNOWLEDGE GRAPH TESTS PASSED (100%)!         \n";
    std::cout << "================================================================================\n";

    return 0;
}
