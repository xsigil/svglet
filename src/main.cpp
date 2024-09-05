#include <CLI/CLI.hpp>
#include <iostream>
#include <svglet/svglet.h>
// 指定されたIDの要素を再帰的に検索する関数
xmlNodePtr find_element_by_id(xmlNodePtr node, const std::string& id) {
    for (xmlNodePtr cur = node; cur != nullptr; cur = cur->next) {
        if (cur->type == XML_ELEMENT_NODE) {
            xmlChar* prop = xmlGetProp(cur, (const xmlChar *)"id");
            if (prop && id == (const char *)prop) {
                xmlFree(prop);
                return cur;
            }
            xmlFree(prop);
        }
        xmlNodePtr found = find_element_by_id(cur->children, id);
        if (found) return found;
    }
    return nullptr;
}

int main(int argc, char **argv) {
    CLI::App app{"svglet - A simple tool for applying SVG transformations"};

    // translateサブコマンド: x, yのオプション
    std::string object_id;
    double translate_x, translate_y;
    auto translate = app.add_subcommand("translate", "Translate the object by x, y");
    translate->add_option("object_id", object_id, "")->required();
    translate->add_option("x", translate_x, "Translate x")->required();
    translate->add_option("y", translate_y, "Translate y")->required();

    std::string center_object_id;
    double angle;
    auto rotate = app.add_subcommand("rotate","Rotate the object around the center of center_object or around the point x, y.");
    rotate->add_option("object_id", object_id, "")->required();
    rotate->add_option("--cid", center_object_id, "");
    rotate->add_option("angle", angle, "")->required(); 

    // comand line option parsing
    CLI11_PARSE(app, argc, argv);

    try {
        // read svg from stdin
        std::ostringstream buffer;
        buffer << std::cin.rdbuf();
        std::string svg_content = buffer.str();

        // parse svg on the memory
        xmlDocPtr doc = xmlReadMemory(svg_content.c_str(), svg_content.size(), "noname.xml", NULL, 0);
        if (doc == nullptr) {
            throw std::runtime_error("Could not parse the SVG input");
        }

        // execute translate subcommand
        if (*translate) {
            doc = translate_svg_object(doc, object_id, translate_x, translate_y);
                    }

        if (*rotate) {
            doc = rotate_svg_object(doc, object_id, center_object_id, angle);

        }

        if (doc == nullptr) {
            throw std::runtime_error("Failed to apply translation");
        }

        // output processed svg
        xmlSaveFormatFileEnc("-", doc, "UTF-8", 1);

        // release memory
        xmlFreeDoc(doc);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}

