#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <svglet/svglet.h>
// SVGオブジェクトに対してtranslate変換を行う関数
xmlNodePtr translate_svg_object(xmlNodePtr root, const std::string& object_id, double translate_x, double translate_y) {
    // 指定されたIDの要素を検索
    xmlNodePtr element = find_element_by_id(root, object_id);
    if (element == nullptr) {
        std::cerr << "ID '" << object_id << "' の要素が見つかりません。" << std::endl;
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

    return root;
}


