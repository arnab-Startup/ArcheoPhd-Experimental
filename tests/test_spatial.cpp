#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <cmath>

#include "models.hpp"
#include "storage.hpp"
#include "analysis/spatial_engine.hpp"
#include "ipc/native_ipc_dispatcher.hpp"

using namespace archaeophd;

int main() {
    std::cout << "================================================================================\n";
    std::cout << "  ArchaeoPhD — Phase 3 Step 2: Spatial Intelligence & GIS Engine Suite          \n";
    std::cout << "================================================================================\n\n";

    // -------------------------------------------------------------
    // TEST 1: Geodesic Haversine Distance & Azimuth Bearing Precision
    // -------------------------------------------------------------
    std::cout << "[TEST 1] Testing WGS84 Geodesic Distance & Compass Bearings...\n";

    // Known archaeological benchmark coordinates:
    // Jerusalem: 31.7767° N, 35.2345° E, 754m elevation
    // Tell es-Sultan (Jericho): 31.8711° N, 35.4444° E, -258m elevation
    // Tel Megiddo: 32.5856° N, 35.1847° E, 160m elevation
    // Khirbet Qeiyafa: 31.6964° N, 34.9572° E, 320m elevation

    double d_jeru_jericho = NativeSpatialEngine::haversine_distance_km(31.7767, 35.2345, 31.8711, 35.4444);
    std::cout << "  ✓ Distance Jerusalem ➔ Jericho: " << d_jeru_jericho << " km\n";
    assert(d_jeru_jericho > 21.0 && d_jeru_jericho < 24.0); // ~22.4 km

    double b_jeru_jericho = NativeSpatialEngine::initial_bearing_degrees(31.7767, 35.2345, 31.8711, 35.4444);
    std::string c_jeru_jericho = NativeSpatialEngine::compass_direction(b_jeru_jericho);
    std::cout << "  ✓ Bearing Jerusalem ➔ Jericho: " << b_jeru_jericho << "° (" << c_jeru_jericho << ")\n";
    assert(b_jeru_jericho >= 55.0 && b_jeru_jericho <= 75.0);
    assert(c_jeru_jericho == "ENE" || c_jeru_jericho == "NE");

    double d_jeru_megiddo = NativeSpatialEngine::haversine_distance_km(31.7767, 35.2345, 32.5856, 35.1847);
    std::cout << "  ✓ Distance Jerusalem ➔ Megiddo: " << d_jeru_megiddo << " km\n";
    assert(d_jeru_megiddo > 88.0 && d_jeru_megiddo < 92.0); // ~90.0 km

    double b_jeru_megiddo = NativeSpatialEngine::initial_bearing_degrees(31.7767, 35.2345, 32.5856, 35.1847);
    std::string c_jeru_megiddo = NativeSpatialEngine::compass_direction(b_jeru_megiddo);
    std::cout << "  ✓ Bearing Jerusalem ➔ Megiddo: " << b_jeru_megiddo << "° (" << c_jeru_megiddo << ")\n";
    assert(c_jeru_megiddo == "N" || c_jeru_megiddo == "NNW");

    // Invalid coordinate defense
    double d_invalid = NativeSpatialEngine::haversine_distance_km(0.0, 0.0, 31.7767, 35.2345);
    assert(d_invalid < 0.0);
    std::cout << "  ✓ Coordinate validation: unlocated (0,0) safely rejected with error code.\n";
    std::cout << "  [PASS] Geodesic distance, azimuth, and compass heading verified!\n\n";

    // -------------------------------------------------------------
    // Set up Local Test Database with 5 Regional Archaeological Sites
    // -------------------------------------------------------------
    std::string test_db = "test_step2_spatial_storage";
    NativeStorage storage(test_db);
    std::string pid = "proj_regional_gis";

    Site s_jeru;
    s_jeru.id = "site_jerusalem";
    s_jeru.project_id = pid;
    s_jeru.site_name = "Jerusalem (City of David)";
    s_jeru.latitude = 31.7767;
    s_jeru.longitude = 35.2345;
    s_jeru.elevation = 754.0;
    s_jeru.period = "EB to Ottoman";
    s_jeru.region = "Judean Highlands";
    storage.put_site(s_jeru);

    Site s_jericho;
    s_jericho.id = "site_jericho";
    s_jericho.project_id = pid;
    s_jericho.site_name = "Tell es-Sultan (Jericho)";
    s_jericho.latitude = 31.8711;
    s_jericho.longitude = 35.4444;
    s_jericho.elevation = -258.0;
    s_jericho.period = "Natufian to MB II";
    s_jericho.region = "Jordan Valley";
    storage.put_site(s_jericho);

    Site s_qeiyafa;
    s_qeiyafa.id = "site_qeiyafa";
    s_qeiyafa.project_id = pid;
    s_qeiyafa.site_name = "Khirbet Qeiyafa";
    s_qeiyafa.latitude = 31.6964;
    s_qeiyafa.longitude = 34.9572;
    s_qeiyafa.elevation = 320.0;
    s_qeiyafa.period = "Iron Age IIA";
    s_qeiyafa.region = "Shephelah";
    storage.put_site(s_qeiyafa);

    Site s_megiddo;
    s_megiddo.id = "site_megiddo";
    s_megiddo.project_id = pid;
    s_megiddo.site_name = "Tel Megiddo";
    s_megiddo.latitude = 32.5856;
    s_megiddo.longitude = 35.1847;
    s_megiddo.elevation = 160.0;
    s_megiddo.period = "EB to Persian";
    s_megiddo.region = "Jezreel Valley";
    storage.put_site(s_megiddo);

    Site s_hazor;
    s_hazor.id = "site_hazor";
    s_hazor.project_id = pid;
    s_hazor.site_name = "Tel Hazor";
    s_hazor.latitude = 33.0175;
    s_hazor.longitude = 35.5683;
    s_hazor.elevation = 230.0;
    s_hazor.period = "EB to Iron II";
    s_hazor.region = "Upper Galilee";
    storage.put_site(s_hazor);

    // Add some strata
    Stratum st1;
    st1.id = "strat_jer_1";
    st1.site_id = "site_jerusalem";
    st1.project_id = pid;
    st1.stratum_name = "Stratum 12 (Iron IIA)";
    storage.put_stratum(st1);

    Stratum st2;
    st2.id = "strat_jer_2";
    st2.site_id = "site_jerusalem";
    st2.project_id = pid;
    st2.stratum_name = "Stratum 10 (Persian)";
    storage.put_stratum(st2);

    NativeSpatialEngine spatial(storage);

    // -------------------------------------------------------------
    // TEST 2: Radius Queries (Territorial Site Filtering)
    // -------------------------------------------------------------
    std::cout << "[TEST 2] Testing Territorial Radius Query (within 35 km of Jerusalem)...\n";
    auto radius_matches = spatial.get_sites_within_radius(31.7767, 35.2345, 35.0, pid);
    std::cout << "  ✓ Found " << radius_matches.size() << " sites within 35 km radius.\n";

    // Should include: Jerusalem (0 km), Jericho (~22.4 km), Qeiyafa (~27.6 km)
    // Should NOT include: Megiddo (~90 km) or Hazor (~145 km)
    assert(radius_matches.size() == 3);
    assert(radius_matches[0].site.id == "site_jerusalem");
    assert(radius_matches[0].distance_km < 0.1);
    assert(radius_matches[1].site.id == "site_jericho" || radius_matches[1].site.id == "site_qeiyafa");
    assert(radius_matches[2].site.id == "site_jericho" || radius_matches[2].site.id == "site_qeiyafa");
    std::cout << "    [1] " << radius_matches[0].site.site_name << " (" << radius_matches[0].distance_km << " km)\n";
    std::cout << "    [2] " << radius_matches[1].site.site_name << " (" << radius_matches[1].distance_km << " km)\n";
    std::cout << "    [3] " << radius_matches[2].site.site_name << " (" << radius_matches[2].distance_km << " km)\n";
    std::cout << "  [PASS] Territorial radius filtering verified!\n\n";

    // -------------------------------------------------------------
    // TEST 3: K-Nearest Neighbor Discovery & Elevation Differential
    // -------------------------------------------------------------
    std::cout << "[TEST 3] Testing K-Nearest Neighbors for Tell es-Sultan (Jericho)...\n";
    auto knn = spatial.compute_nearest_neighbors("site_jericho", 2, pid);
    assert(knn.size() == 2);
    // Nearest to Jericho is Jerusalem (~22.4 km)
    assert(knn[0].site.id == "site_jerusalem");
    assert(knn[0].distance_km > 21.0 && knn[0].distance_km < 24.0);
    // Elevation diff: Jerusalem (754) - Jericho (-258) = +1012 meters climb
    assert(knn[0].elevation_diff_meters > 1000.0);
    std::cout << "  ✓ Nearest neighbor to Jericho: " << knn[0].site.site_name << " (" << knn[0].distance_km << " km, " << knn[0].compass_direction << ")\n";
    std::cout << "    Vertical climb: +" << knn[0].elevation_diff_meters << " m\n";
    std::cout << "  [PASS] KNN discovery & topographic delta verified!\n\n";

    // -------------------------------------------------------------
    // TEST 4: Geodesic DBSCAN Spatial Clustering
    // -------------------------------------------------------------
    std::cout << "[TEST 4] Testing Regional Spatial Clustering (Epsilon = 60 km, MinPts = 2)...\n";
    // With 60km epsilon:
    // Cluster 1 (Southern/Central): Jerusalem, Jericho, Qeiyafa
    // Cluster 2 (Northern): Megiddo, Hazor (distance Megiddo to Hazor is ~60 km)
    double d_megiddo_hazor = NativeSpatialEngine::haversine_distance_km(32.5856, 35.1847, 33.0175, 35.5683);
    std::cout << "  ✓ Distance Megiddo ➔ Hazor: " << d_megiddo_hazor << " km\n";

    auto clusters = spatial.compute_spatial_clusters(60.0, 2, pid);
    std::cout << "  ✓ Detected " << clusters.size() << " regional territorial clusters.\n";
    assert(clusters.size() >= 2);
    for (const auto& cl : clusters) {
        std::cout << "    " << cl.label << " Centroid: (" << cl.centroid_latitude << ", " << cl.centroid_longitude << ")\n";
        assert(cl.site_count >= 2);
    }
    std::cout << "  [PASS] Geodesic DBSCAN clustering verified!\n\n";

    // -------------------------------------------------------------
    // TEST 5: RFC 7946 GeoJSON FeatureCollection Generation
    // -------------------------------------------------------------
    std::cout << "[TEST 5] Testing RFC 7946 GeoJSON FeatureCollection...\n";
    auto geojson = spatial.to_geojson_feature_collection(pid);
    assert(geojson["type"] == "FeatureCollection");
    assert(geojson.contains("features"));
    assert(geojson["features"].size() == 5);
    assert(geojson.contains("bbox"));
    // Bbox should be [min_lon, min_lat, max_lon, max_lat]
    assert(geojson["bbox"].size() == 4);
    assert(geojson["bbox"][0] < geojson["bbox"][2]); // west < east
    assert(geojson["bbox"][1] < geojson["bbox"][3]); // south < north

    // Verify coordinate order: [lon, lat, elevation]
    const auto& f0 = geojson["features"][0];
    assert(f0["type"] == "Feature");
    assert(f0["geometry"]["type"] == "Point");
    const auto& coords = f0["geometry"]["coordinates"];
    // Longitude in Levant is ~34.9 to 35.6, latitude is ~31.6 to 33.1
    assert(coords[0].get<double>() > 34.0 && coords[0].get<double>() < 36.0); // longitude first!
    assert(coords[1].get<double>() > 31.0 && coords[1].get<double>() < 34.0); // latitude second!
    std::cout << "  ✓ Coordinates adhere strictly to RFC 7946 Section 3.1.1: [lon, lat, ele]\n";
    std::cout << "  ✓ GeoJSON bounding box: [" << geojson["bbox"][0] << ", " << geojson["bbox"][1] 
              << ", " << geojson["bbox"][2] << ", " << geojson["bbox"][3] << "]\n";
    std::cout << "  [PASS] RFC 7946 GeoJSON schema compliance verified!\n\n";

    // -------------------------------------------------------------
    // TEST 6: Native IPC Dispatcher Endpoints
    // -------------------------------------------------------------
    std::cout << "[TEST 6] Testing Native IPC Dispatcher Spatial Endpoints...\n";
    NativeIpcDispatcher dispatcher(&storage, nullptr, nullptr, "");

    // 1. get_spatial_geojson
    std::string req_geo = "{\"id\": \"msg-geo-1\", \"action\": \"get_spatial_geojson\", \"projectId\": \"" + pid + "\"}";
    std::string res_geo_raw = dispatcher.dispatch(req_geo);
    auto res_geo = json::parse(res_geo_raw);
    assert(res_geo.contains("result"));
    assert(res_geo["result"]["type"] == "FeatureCollection");
    std::cout << "  ✓ get_spatial_geojson returned FeatureCollection with " 
              << res_geo["result"]["features"].size() << " features.\n";

    // 2. query_sites_radius
    std::string req_rad = "{\"id\": \"msg-geo-2\", \"action\": \"query_sites_radius\", \"projectId\": \"" + pid + "\", \"payload\": {\"latitude\": 31.7767, \"longitude\": 35.2345, \"radius_km\": 30.0}}";
    std::string res_rad_raw = dispatcher.dispatch(req_rad);
    auto res_rad = json::parse(res_rad_raw);
    assert(res_rad.contains("result"));
    assert(res_rad["result"].size() == 3);
    std::cout << "  ✓ query_sites_radius returned 3 sites within 30 km.\n";

    // 3. get_site_neighbors
    std::string req_knn = "{\"id\": \"msg-geo-3\", \"action\": \"get_site_neighbors\", \"projectId\": \"" + pid + "\", \"payload\": {\"site_id\": \"site_megiddo\", \"k\": 2}}";
    std::string res_knn_raw = dispatcher.dispatch(req_knn);
    auto res_knn = json::parse(res_knn_raw);
    assert(res_knn.contains("result"));
    assert(res_knn["result"].size() == 2);
    std::cout << "  ✓ get_site_neighbors returned " << res_knn["result"].size() << " neighbors for Tel Megiddo.\n";

    // 4. get_spatial_clusters
    std::string req_cl = "{\"id\": \"msg-geo-4\", \"action\": \"get_spatial_clusters\", \"projectId\": \"" + pid + "\", \"payload\": {\"epsilon_km\": 60.0, \"min_pts\": 2}}";
    std::string res_cl_raw = dispatcher.dispatch(req_cl);
    auto res_cl = json::parse(res_cl_raw);
    assert(res_cl.contains("result"));
    assert(res_cl["result"].size() >= 2);
    std::cout << "  ✓ get_spatial_clusters returned " << res_cl["result"].size() << " territorial clusters.\n";

    std::cout << "  [PASS] All 4 Spatial IPC endpoints operating with 100% fidelity!\n\n";

    std::cout << "================================================================================\n";
    std::cout << "  ALL PHASE 3 STEP 2 SPATIAL INTELLIGENCE TESTS PASSED (100%)!                  \n";
    std::cout << "================================================================================\n";
    return 0;
}
