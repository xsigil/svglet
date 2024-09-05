#include <CLI/CLI.hpp>
#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>

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

// SVGオブジェクトに対してtranslate変換を行う関数
xmlDocPtr translate_svg_object(xmlDocPtr doc, const std::string& object_id, double translate_x, double translate_y) {
    if (doc == nullptr) {
        std::cerr << "SVGファイルの読み込みに失敗しました。" << std::endl;
        return nullptr;
    }

    // ルート要素の取得
    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (root == nullptr) {
        std::cerr << "ルート要素が見つかりません。" << std::endl;
        xmlFreeDoc(doc);
        return nullptr;
    }

    // 指定されたIDの要素を検索
    xmlNodePtr element = find_element_by_id(root, object_id);
    if (element == nullptr) {
        std::cerr << "ID '" << object_id << "' の要素が見つかりません。" << std::endl;
        xmlFreeDoc(doc);
        return nullptr;
    }

    // 現在のtransform属性を取得
    xmlChar* transform = xmlGetProp(element, (const xmlChar *)"transform");
    std::string new_transform;
    if (transform) {
        new_transform = (const char *)transform;
        xmlFree(transform);
    }

    // translate(x, y)を追加
    new_transform += " translate(" + std::to_string(translate_x) + "," + std::to_string(translate_y) + ")";
    xmlSetProp(element, (const xmlChar *)"transform", (const xmlChar *)new_transform.c_str());

    return doc;
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
            if (doc == nullptr) {
                throw std::runtime_error("Failed to apply translation");
            }
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

