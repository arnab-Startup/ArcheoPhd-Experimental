#pragma once

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <queue>
#include <map>
#include <set>
#include <sstream>
#include <iomanip>
#include <json.hpp>
#include "models.hpp"
#include "storage.hpp"

namespace archaeophd {

// =============================================================================
// Native Spatial Intelligence & GIS Engine — Phase 3 Step 2
// =============================================================================
// Provides:
// 1. WGS84 Geodetic modeling & Haversine great-circle distance calculations
// 2. Initial bearing (azimuth 0-360 deg) and 16-point cardinal compass direction
// 3. Territorial radius queries (e.g. sites within X km of coordinate/city)
// 4. K-Nearest Neighbor archaeological site spatial discovery
// 5. Geodesic DBSCAN spatial clustering for regional horizon territories
// 6. RFC 7946 compliant GeoJSON FeatureCollection generation for Leaflet/QGIS
// =============================================================================

struct SpatialSiteMatch {
    Site site;
    double distance_km = 0.0;
    double bearing_degrees = 0.0;
    std::string compass_direction;
    double elevation_diff_meters = 0.0;
};

struct SpatialCluster {
    int cluster_id = -1;
    std::string label;
    double centroid_latitude = 0.0;
    double centroid_longitude = 0.0;
    double min_latitude = 0.0;
    double max_latitude = 0.0;
    double min_longitude = 0.0;
    double max_longitude = 0.0;
    std::vector<std::string> site_ids;
    std::vector<std::string> site_names;
    int site_count = 0;
};

class NativeSpatialEngine {
public:
    static constexpr double EARTH_RADIUS_KM = 6371.0088; // WGS84 mean earth radius
    static constexpr double PI = 3.14159265358979323846;
    static constexpr double DEG_TO_RAD = PI / 180.0;
    static constexpr double RAD_TO_DEG = 180.0 / PI;

private:
    const NativeStorage& storage_;

public:
    explicit NativeSpatialEngine(const NativeStorage& storage)
        : storage_(storage) {}

    // -------------------------------------------------------------
    // Coordinate Validity Check (WGS84 Domain)
    // -------------------------------------------------------------
    static bool is_valid_coordinate(double lat, double lon) {
        if (std::isnan(lat) || std::isnan(lon)) return false;
        if (lat == 0.0 && lon == 0.0) return false; // Default placeholder
        return (lat >= -90.0 && lat <= 90.0 && lon >= -180.0 && lon <= 180.0);
    }

    // -------------------------------------------------------------
    // Haversine Great-Circle Distance (in kilometers)
    // -------------------------------------------------------------
    static double haversine_distance_km(double lat1, double lon1, double lat2, double lon2) {
        if (!is_valid_coordinate(lat1, lon1) || !is_valid_coordinate(lat2, lon2)) {
            return -1.0;
        }

        double phi1 = lat1 * DEG_TO_RAD;
        double phi2 = lat2 * DEG_TO_RAD;
        double delta_phi = (lat2 - lat1) * DEG_TO_RAD;
        double delta_lambda = (lon2 - lon1) * DEG_TO_RAD;

        double a = std::sin(delta_phi / 2.0) * std::sin(delta_phi / 2.0) +
                   std::cos(phi1) * std::cos(phi2) *
                   std::sin(delta_lambda / 2.0) * std::sin(delta_lambda / 2.0);
        
        // Prevent numerical domain error in sqrt due to floating-point rounding
        a = std::max(0.0, std::min(1.0, a));
        double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
        return EARTH_RADIUS_KM * c;
    }

    // -------------------------------------------------------------
    // Initial Bearing (Azimuth in degrees [0, 360))
    // -------------------------------------------------------------
    static double initial_bearing_degrees(double lat1, double lon1, double lat2, double lon2) {
        if (!is_valid_coordinate(lat1, lon1) || !is_valid_coordinate(lat2, lon2)) {
            return 0.0;
        }

        double phi1 = lat1 * DEG_TO_RAD;
        double phi2 = lat2 * DEG_TO_RAD;
        double delta_lambda = (lon2 - lon1) * DEG_TO_RAD;

        double y = std::sin(delta_lambda) * std::cos(phi2);
        double x = std::cos(phi1) * std::sin(phi2) -
                   std::sin(phi1) * std::cos(phi2) * std::cos(delta_lambda);

        double theta = std::atan2(y, x) * RAD_TO_DEG;
        double bearing = std::fmod(theta + 360.0, 360.0);
        return bearing;
    }

    // -------------------------------------------------------------
    // 16-Point Cardinal Compass Heading
    // -------------------------------------------------------------
    static std::string compass_direction(double bearing_deg) {
        static const char* directions[] = {
            "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
            "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"
        };
        int idx = static_cast<int>(std::round(bearing_deg / 22.5)) % 16;
        if (idx < 0) idx += 16;
        return directions[idx];
    }

    // -------------------------------------------------------------
    // Radius Query: Sites within radius_km of (center_lat, center_lon)
    // -------------------------------------------------------------
    std::vector<SpatialSiteMatch> get_sites_within_radius(
        double center_lat, double center_lon, double radius_km,
        const std::string& project_id = "default") const {

        std::vector<SpatialSiteMatch> results;
        if (!is_valid_coordinate(center_lat, center_lon) || radius_km <= 0.0) {
            return results;
        }

        auto sites = storage_.get_sites(project_id);
        for (const auto& s : sites) {
            if (!is_valid_coordinate(s.latitude, s.longitude)) continue;

            double dist = haversine_distance_km(center_lat, center_lon, s.latitude, s.longitude);
            if (dist >= 0.0 && dist <= radius_km) {
                SpatialSiteMatch match;
                match.site = s;
                match.distance_km = dist;
                match.bearing_degrees = initial_bearing_degrees(center_lat, center_lon, s.latitude, s.longitude);
                match.compass_direction = compass_direction(match.bearing_degrees);
                results.push_back(std::move(match));
            }
        }

        std::sort(results.begin(), results.end(), [](const SpatialSiteMatch& a, const SpatialSiteMatch& b) {
            return a.distance_km < b.distance_km;
        });

        return results;
    }

    // -------------------------------------------------------------
    // K-Nearest Neighbors for a Specific Archaeological Site
    // -------------------------------------------------------------
    std::vector<SpatialSiteMatch> compute_nearest_neighbors(
        const std::string& site_id, int k = 5,
        const std::string& project_id = "default") const {

        std::vector<SpatialSiteMatch> results;
        if (k <= 0) return results;

        auto opt_site = storage_.get_site(site_id);
        if (!opt_site.has_value()) return results;
        const auto& origin = opt_site.value();

        if (!is_valid_coordinate(origin.latitude, origin.longitude)) return results;

        auto sites = storage_.get_sites(project_id);
        for (const auto& s : sites) {
            if (s.id == site_id) continue; // Exclude self
            if (!is_valid_coordinate(s.latitude, s.longitude)) continue;

            double dist = haversine_distance_km(origin.latitude, origin.longitude, s.latitude, s.longitude);
            if (dist >= 0.0) {
                SpatialSiteMatch match;
                match.site = s;
                match.distance_km = dist;
                match.bearing_degrees = initial_bearing_degrees(origin.latitude, origin.longitude, s.latitude, s.longitude);
                match.compass_direction = compass_direction(match.bearing_degrees);
                match.elevation_diff_meters = (s.elevation - origin.elevation);
                results.push_back(std::move(match));
            }
        }

        std::sort(results.begin(), results.end(), [](const SpatialSiteMatch& a, const SpatialSiteMatch& b) {
            return a.distance_km < b.distance_km;
        });

        if (static_cast<int>(results.size()) > k) {
            results.resize(k);
        }

        return results;
    }

    // -------------------------------------------------------------
    // Geodesic DBSCAN Spatial Clustering
    // Groups excavation sites into regional territories based on distance
    // -------------------------------------------------------------
    std::vector<SpatialCluster> compute_spatial_clusters(
        double epsilon_km = 35.0, int min_pts = 1,
        const std::string& project_id = "default") const {

        std::vector<SpatialCluster> clusters;
        auto sites = storage_.get_sites(project_id);

        // Filter valid coordinate sites
        std::vector<Site> geo_sites;
        for (const auto& s : sites) {
            if (is_valid_coordinate(s.latitude, s.longitude)) {
                geo_sites.push_back(s);
            }
        }

        if (geo_sites.empty()) return clusters;

        int n = static_cast<int>(geo_sites.size());
        std::vector<int> labels(n, 0); // 0 = unvisited, -1 = noise, >0 = cluster_id
        int current_cluster = 0;

        auto region_query = [&](int idx) -> std::vector<int> {
            std::vector<int> neighbors;
            for (int j = 0; j < n; ++j) {
                double dist = haversine_distance_km(
                    geo_sites[idx].latitude, geo_sites[idx].longitude,
                    geo_sites[j].latitude, geo_sites[j].longitude);
                if (dist >= 0.0 && dist <= epsilon_km) {
                    neighbors.push_back(j);
                }
            }
            return neighbors;
        };

        for (int i = 0; i < n; ++i) {
            if (labels[i] != 0) continue;

            auto neighbors = region_query(i);
            if (static_cast<int>(neighbors.size()) < min_pts) {
                labels[i] = -1; // Noise
            } else {
                current_cluster++;
                labels[i] = current_cluster;

                std::queue<int> q;
                for (int nb : neighbors) {
                    if (nb != i) q.push(nb);
                }

                while (!q.empty()) {
                    int p = q.front();
                    q.pop();

                    if (labels[p] == -1) {
                        labels[p] = current_cluster;
                    }
                    if (labels[p] != 0) continue;

                    labels[p] = current_cluster;
                    auto p_neighbors = region_query(p);
                    if (static_cast<int>(p_neighbors.size()) >= min_pts) {
                        for (int pn : p_neighbors) {
                            if (labels[pn] == 0 || labels[pn] == -1) {
                                q.push(pn);
                            }
                        }
                    }
                }
            }
        }

        // Aggregate clusters
        std::map<int, std::vector<int>> cluster_map;
        for (int i = 0; i < n; ++i) {
            if (labels[i] > 0) {
                cluster_map[labels[i]].push_back(i);
            }
        }

        for (const auto& [cid, indices] : cluster_map) {
            SpatialCluster sc;
            sc.cluster_id = cid;
            sc.site_count = static_cast<int>(indices.size());

            double sum_lat = 0.0;
            double sum_lon = 0.0;
            double min_lat = 90.0, max_lat = -90.0;
            double min_lon = 180.0, max_lon = -180.0;

            for (int idx : indices) {
                const auto& s = geo_sites[idx];
                sum_lat += s.latitude;
                sum_lon += s.longitude;
                min_lat = std::min(min_lat, s.latitude);
                max_lat = std::max(max_lat, s.latitude);
                min_lon = std::min(min_lon, s.longitude);
                max_lon = std::max(max_lon, s.longitude);
                sc.site_ids.push_back(s.id);
                sc.site_names.push_back(s.site_name);
            }

            sc.centroid_latitude = sum_lat / sc.site_count;
            sc.centroid_longitude = sum_lon / sc.site_count;
            sc.min_latitude = min_lat;
            sc.max_latitude = max_lat;
            sc.min_longitude = min_lon;
            sc.max_longitude = max_lon;

            std::ostringstream oss;
            oss << "Regional Zone #" << cid << " (" << sc.site_count << " sites)";
            sc.label = oss.str();

            clusters.push_back(std::move(sc));
        }

        return clusters;
    }

    // -------------------------------------------------------------
    // RFC 7946 GeoJSON FeatureCollection Generation
    // Direct feed for Leaflet / OpenLayers / QGIS GIS bridge
    // -------------------------------------------------------------
    json to_geojson_feature_collection(const std::string& project_id = "default") const {
        json root;
        root["type"] = "FeatureCollection";
        json features = json::array();

        auto sites = storage_.get_sites(project_id);
        auto strata = storage_.get_strata(project_id);
        auto artifacts = storage_.get_artifacts(project_id);

        std::map<std::string, int> site_strata_count;
        for (const auto& st : strata) site_strata_count[st.site_id]++;

        // Bounding box accumulators
        bool has_bbox = false;
        double min_lon = 180.0, max_lon = -180.0;
        double min_lat = 90.0, max_lat = -90.0;

        for (const auto& s : sites) {
            if (!is_valid_coordinate(s.latitude, s.longitude)) continue;

            has_bbox = true;
            min_lon = std::min(min_lon, s.longitude);
            max_lon = std::max(max_lon, s.longitude);
            min_lat = std::min(min_lat, s.latitude);
            max_lat = std::max(max_lat, s.latitude);

            json feat;
            feat["type"] = "Feature";
            feat["id"] = s.id;

            // RFC 7946 Section 3.1.1: [longitude, latitude, elevation]
            json coords = json::array({s.longitude, s.latitude});
            if (s.elevation != 0.0) {
                coords.push_back(s.elevation);
            }

            feat["geometry"] = {
                {"type", "Point"},
                {"coordinates", coords}
            };

            int sc = site_strata_count[s.id];
            feat["properties"] = {
                {"site_name", s.site_name},
                {"country", s.country},
                {"region", s.region},
                {"period", s.period},
                {"site_type", s.site_type},
                {"elevation", s.elevation},
                {"stratum_count", sc},
                {"excavation_history", s.excavation_history}
            };

            features.push_back(feat);
        }

        root["features"] = features;
        if (has_bbox) {
            // RFC 7946 Section 5: [west, south, east, north]
            root["bbox"] = json::array({min_lon, min_lat, max_lon, max_lat});
        }

        return root;
    }
};

} // namespace archaeophd
