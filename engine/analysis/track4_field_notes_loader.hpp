#pragma once

#include "epistemic_logic.hpp"
#include <json.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

namespace archaeophd {

struct UnstructuredFieldNote {
    std::string id;
    std::string site;
    std::string raw_text;
    std::string premise;
    std::string hypothesis;
    std::string gold_relation;
    std::string gold_admission;
    double p_support = 0.0;
    double p_contradiction = 0.0;
    double p_unrelated = 0.0;
};

class Track4DatasetLoader {
public:
    static std::vector<UnstructuredFieldNote> load_json(const std::string& path) {
        std::vector<UnstructuredFieldNote> notes;
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to open field notes JSON: " << path << std::endl;
            return notes;
        }

        nlohmann::json j;
        try {
            file >> j;
            for (const auto& item : j) {
                UnstructuredFieldNote n;
                n.id = item.value("id", "");
                n.site = item.value("site", "");
                n.raw_text = item.value("raw_text", "");
                n.premise = item.value("premise", "");
                n.hypothesis = item.value("hypothesis", "");
                n.gold_relation = item.value("gold_relation", "");
                n.gold_admission = item.value("gold_admission", "");
                n.p_support = item.value("p_sup", 0.0);
                n.p_contradiction = item.value("p_con", 0.0);
                n.p_unrelated = item.value("p_unr", 0.0);
                notes.push_back(n);
            }
        } catch (const std::exception& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }
        return notes;
    }
};

} // namespace archaeophd
