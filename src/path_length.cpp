#include "svglet/svglet.h"
#include <2geom/path.h>
#include <2geom/pathvector.h>
#include <2geom/piecewise.h>
#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/svg-path-parser.h>
#include <iostream>
#include <stdexcept>
#include <string>

// パス文字列から総延長を計算する純粋計算ロジック
double compute_svg_path_length(const std::string& path_data) {
    if (path_data.empty()) {
        return 0.0;
    }

    try {
        Geom::PathVector pathv = Geom::parse_svg_path(path_data.c_str());
        double total_length = 0.0;
        for (const auto& path : pathv) {
            Geom::Piecewise<Geom::D2<Geom::SBasis>> piecewise = path.toPwSb();
            total_length += Geom::length(piecewise);
        }
        return total_length;
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to parse SVG path: ") + e.what());
    }
}

// SVGドキュメント内から指定IDのpathのd属性を探して長さを計算
double compute_svg_element_path_length(xmlNodePtr root, const std::string& object_id) {
    xmlNodePtr element = find_element_by_id(root, object_id);
    if (!element) {
        throw std::runtime_error("ID '" + object_id + "' not found in SVG document.");
    }

    if (xmlStrcmp(element->name, (const xmlChar*)"path") != 0) {
        throw std::runtime_error("Element with ID '" + object_id + "' is not a <path> element.");
    }

    xmlChar* d_attr = xmlGetProp(element, (const xmlChar*)"d");
    if (!d_attr) {
        throw std::runtime_error("Element with ID '" + object_id + "' has no 'd' attribute.");
    }

    std::string d_str(reinterpret_cast<char*>(d_attr));
    xmlFree(d_attr);

    return compute_svg_path_length(d_str);
}
