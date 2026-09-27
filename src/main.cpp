#include <2geom/2geom.h>
#include <CLI/CLI.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "svglet/svglet.h"

int main(int argc, char **argv) {
    CLI::App app{"svglet - A lightweight SVG geometry and transformation CLI"};

    // --- Subcommand: translate ---
    std::string trans_id;
    double translate_x = 0.0, translate_y = 0.0;
    auto translate = app.add_subcommand("translate", "Translate the object by x, y");
    translate->add_option("object_id", trans_id, "Target object ID")->required();
    translate->add_option("x", translate_x, "Translate x")->required();
    translate->add_option("y", translate_y, "Translate y")->required();

    // --- Subcommand: rotate ---
    std::string rot_id;
    std::string center_object_id;
    double angle = 0.0;
    auto rotate = app.add_subcommand("rotate", "Rotate the object around center_object or origin");
    rotate->add_option("object_id", rot_id, "Target object ID")->required();
    rotate->add_option("angle", angle, "Rotation angle in degrees")->required();
    rotate->add_option("--cid", center_object_id, "Center object ID to rotate around");

    // --- Subcommand: length ---
    std::string path_data;
    std::string length_id;
    auto length = app.add_subcommand("length", "Compute the total geometric length of a path");
    length->add_option("path", path_data, "Raw SVG path data string (e.g. 'M 0 0 L 100 100')");
    length->add_option("--id", length_id, "Target path element ID from stdin SVG");

    CLI11_PARSE(app, argc, argv);

    try {
        // 1. length サブコマンドで生文字列が直接渡された場合（XMLパース不要で高速終了）
        if (*length && !path_data.empty()) {
            double total = compute_svg_path_length(path_data);
            std::cout << total << std::endl;
            return 0;
        }

        // 2. それ以外は標準入力から SVG を受け取る
        std::ostringstream buffer;
        buffer << std::cin.rdbuf();
        std::string svg_content = buffer.str();

        if (svg_content.empty()) {
            std::cerr << "Error: No SVG input provided via stdin." << std::endl;
            return 1;
        }

        xmlDocPtr doc = xmlReadMemory(svg_content.c_str(), svg_content.size(), "noname.xml", nullptr, 0);
        if (!doc) {
            throw std::runtime_error("Could not parse SVG document from stdin.");
        }

        xmlNodePtr root = xmlDocGetRootElement(doc);
        if (!root) {
            xmlFreeDoc(doc);
            throw std::runtime_error("Root element not found in SVG document.");
        }

        if (*length) {
            if (length_id.empty()) {
                xmlFreeDoc(doc);
                throw std::runtime_error("Provide path data as argument or specify --id <element_id>.");
            }
            double total = compute_svg_element_path_length(root, length_id);
            std::cout << total << std::endl;
            xmlFreeDoc(doc);
            return 0;
        }

        if (*translate) {
            root = translate_svg_object(root, trans_id, translate_x, translate_y);
        } else if (*rotate) {
            root = rotate_svg_object(root, rot_id, center_object_id, angle);
        }

        // 変換後のSVGを出力
        xmlSaveFormatFileEnc("-", doc, "UTF-8", 1);
        xmlFreeDoc(doc);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
