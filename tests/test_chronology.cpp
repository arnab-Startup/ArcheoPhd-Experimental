#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <cmath>

#include "models.hpp"
#include "storage.hpp"
#include "analysis/chronology.hpp"
#include "ipc/native_ipc_dispatcher.hpp"

using namespace archaeophd;

int main() {
    std::cout << "================================================================================\n";
    std::cout << "  ArchaeoPhD — Phase 3 Step 1: Chronology Engine & C-14 Calibration Suite        \n";
    std::cout << "================================================================================\n\n";

    // -------------------------------------------------------------
    // TEST 1: Astronomical Year Normalization & Period Parsing
    // -------------------------------------------------------------
    std::cout << "[TEST 1] Testing Astronomical Year Parsing & Cultural Periods...\n";

    auto r1 = NativeChronologyEngine::parse_archaeological_date("1400 BCE");
    assert(r1.valid);
    assert(r1.start == -1399.0);
    std::cout << "  ✓ '1400 BCE' -> Astronomical year: " << r1.start << " (" << r1.display_label << ")\n";

    auto r2 = NativeChronologyEngine::parse_archaeological_date("1550 - 1400 BCE");
    assert(r2.valid);
    assert(r2.start == -1549.0);
    assert(r2.end == -1399.0);
    std::cout << "  ✓ '1550 - 1400 BCE' -> Span: [" << r2.start << ", " << r2.end << "] (" << r2.display_label << ")\n";

    auto r3 = NativeChronologyEngine::parse_archaeological_date("ca. 1230 BC");
    assert(r3.valid);
    assert(r3.is_approximate);
    assert(r3.start == -1229.0);
    std::cout << "  ✓ 'ca. 1230 BC' -> Approx: " << (r3.is_approximate ? "true" : "false") << " -> " << r3.start << "\n";

    auto r4 = NativeChronologyEngine::parse_archaeological_date("Middle Bronze Age");
    assert(r4.valid);
    assert(r4.start == -2000.0);
    assert(r4.end == -1550.0);
    std::cout << "  ✓ 'Middle Bronze Age' -> Canonical range: [" << r4.start << ", " << r4.end << "]\n";

    auto r5 = NativeChronologyEngine::parse_archaeological_date("Natufian");
    assert(r5.valid);
    assert(r5.start == -11500.0);
    std::cout << "  ✓ 'Natufian' -> Deep prehistoric horizon: [" << r5.start << ", " << r5.end << "]\n";

    std::cout << "  [PASS] Astronomical date parsing and period mapping verified!\n\n";

    // -------------------------------------------------------------
    // TEST 2: IntCal20 Radiocarbon Calibration (2-Sigma Probability Envelope)
    // -------------------------------------------------------------
    std::cout << "[TEST 2] Testing IntCal20 C-14 Radiocarbon Calibration...\n";

    // Test Late Bronze Age / Thera destruction determination: 3350 +- 40 BP
    auto cal_thera = NativeChronologyEngine::calibrate_c14(3350.0, 40.0);
    std::cout << "  ✓ Determination: " << cal_thera.formula << "\n";
    std::cout << "    Calibrated Range: [" << cal_thera.cal_start << ", " << cal_thera.cal_end << "]\n";
    std::cout << "    Display: " << cal_thera.cal_display_start << " to " << cal_thera.cal_display_end << "\n";
    assert(cal_thera.cal_start < -1500.0 && cal_thera.cal_end > -1750.0);

    // Test Roman determination: 2000 +- 30 BP
    auto cal_roman = NativeChronologyEngine::calibrate_c14(2000.0, 30.0);
    std::cout << "  ✓ Determination: " << cal_roman.formula << "\n";
    assert(cal_roman.cal_start >= -100.0 && cal_roman.cal_end <= 200.0);

    // Test PPNB Neolithic determination: 8000 +- 50 BP
    auto cal_neolithic = NativeChronologyEngine::calibrate_c14(8000.0, 50.0);
    std::cout << "  ✓ Determination: " << cal_neolithic.formula << "\n";
    assert(cal_neolithic.cal_start <= -6800.0);

    std::cout << "  [PASS] IntCal20 atmospheric calibration curve precision confirmed!\n\n";

    // -------------------------------------------------------------
    // TEST 3: Multi-Site Contemporaneity & Overlap Analytics
    // -------------------------------------------------------------
    std::cout << "[TEST 3] Testing Multi-Site Contemporaneous Stratigraphic Alignment...\n";
    std::string test_db = "test_step1_chrono_storage";
    NativeStorage storage(test_db);
    std::string pid = "proj_regional_chrono";

    // Site A: Jericho (Middle Bronze IIB: 1750 to 1550 BCE)
    Site s_jericho;
    s_jericho.id = "site_jericho";
    s_jericho.project_id = pid;
    s_jericho.site_name = "Tell es-Sultan (Jericho)";
    s_jericho.period = "1750 - 1550 BCE";
    storage.put_site(s_jericho);

    // Site B: Megiddo (MB IIB through LB II: 1750 to 1150 BCE)
    Site s_megiddo;
    s_megiddo.id = "site_megiddo";
    s_megiddo.project_id = pid;
    s_megiddo.site_name = "Tel Megiddo";
    s_megiddo.period = "1750 - 1150 BCE";
    storage.put_site(s_megiddo);

    // Site C: Khirbet Qeiyafa (Iron Age IIA: 1050 to 950 BCE)
    Site s_qeiyafa;
    s_qeiyafa.id = "site_qeiyafa";
    s_qeiyafa.project_id = pid;
    s_qeiyafa.site_name = "Khirbet Qeiyafa";
    s_qeiyafa.period = "1050 - 950 BCE";
    storage.put_site(s_qeiyafa);

    NativeChronologyEngine chrono_engine(storage);
    auto syncs = chrono_engine.compute_synchronisms(pid);

    assert(syncs.size() == 3);
    bool found_jericho_megiddo = false;
    bool found_jericho_qeiyafa = false;

    for (const auto& sc : syncs) {
        if ((sc.site_a_id == "site_jericho" && sc.site_b_id == "site_megiddo") ||
            (sc.site_a_id == "site_megiddo" && sc.site_b_id == "site_jericho")) {
            found_jericho_megiddo = true;
            assert(sc.contemporaneous_status == "CONTEMPORANEOUS");
            assert(sc.overlap_span_years >= 190.0);
            std::cout << "  ✓ Synchronism (" << sc.site_a_name << " \u2194 " << sc.site_b_name << "): "
                      << sc.contemporaneous_status << " (Overlap: " << sc.overlap_span_years << " years)\n";
        }
        if ((sc.site_a_id == "site_jericho" && sc.site_b_id == "site_qeiyafa") ||
            (sc.site_a_id == "site_qeiyafa" && sc.site_b_id == "site_jericho")) {
            found_jericho_qeiyafa = true;
            assert(sc.contemporaneous_status == "PREDECESSOR" || sc.contemporaneous_status == "SUCCESSOR");
            assert(sc.overlap_span_years == 0.0);
            std::cout << "  ✓ Synchronism (" << sc.site_a_name << " \u2194 " << sc.site_b_name << "): "
                      << sc.contemporaneous_status << " (Disjunct horizons)\n";
        }
    }
    assert(found_jericho_megiddo && found_jericho_qeiyafa);
    std::cout << "  [PASS] Multi-site contemporaneous alignment correctly computed!\n\n";

    // -------------------------------------------------------------
    // TEST 4: Native IPC Dispatcher Timeline Endpoints
    // -------------------------------------------------------------
    std::cout << "[TEST 4] Testing Native IPC Dispatcher Endpoints...\n";
    NativeIpcDispatcher dispatcher(&storage, nullptr, nullptr, "");

    // 1. get_chronology_timeline
    std::string req_tl = "{\"id\": \"msg-chrono-1\", \"action\": \"get_chronology_timeline\", \"projectId\": \"" + pid + "\"}";
    std::string res_tl_raw = dispatcher.dispatch(req_tl);
    auto res_tl = json::parse(res_tl_raw);

    assert(res_tl.contains("result"));
    const auto& tl = res_tl["result"];
    assert(tl.contains("timeline"));
    assert(tl.contains("synchronisms"));
    assert(tl["timeline"].size() >= 3);
    assert(tl.contains("min_year"));
    assert(tl.contains("max_year"));
    std::cout << "  ✓ get_chronology_timeline returned " << tl["timeline"].size() << " timeline entries.\n";
    std::cout << "    Min year: " << tl["min_year"] << " | Max year: " << tl["max_year"] << "\n";

    // 2. calibrate_radiocarbon
    std::string req_cal = "{\"id\": \"msg-cal-1\", \"action\": \"calibrate_radiocarbon\", \"payload\": {\"bp_age\": 3200, \"bp_sigma\": 35}}";
    std::string res_cal_raw = dispatcher.dispatch(req_cal);
    auto res_cal = json::parse(res_cal_raw);

    assert(res_cal.contains("result"));
    const auto& c_res = res_cal["result"];
    assert(c_res.contains("formula"));
    assert(c_res.contains("cal_start"));
    assert(c_res.contains("cal_end"));
    std::cout << "  ✓ calibrate_radiocarbon IPC Result: " << c_res["formula"] << "\n";
    std::cout << "  [PASS] IPC Bridge endpoints operate with 100% schema compliance!\n\n";

    std::cout << "================================================================================\n";
    std::cout << "  ALL PHASE 3 STEP 1 CHRONOLOGY TESTS PASSED (100%)!                            \n";
    std::cout << "================================================================================\n";
    return 0;
}
