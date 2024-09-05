#include <iostream>
#include <string>
#include <libxml/parser.h>
#include <libxml/tree.h>

// オブジェクトを検索する関数
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

void translate_svg_object(std::istream& input, const std::string& object_id, double translate_x, double translate_y) {
    // XMLパーサの初期化
    xmlDocPtr doc = xmlReadFile("-", nullptr, 0);  // 標準入力から読む
    if (doc == nullptr) {
        std::cerr << "SVGファイルの読み込みに失敗しました。" << std::endl;
        return;
    }

    // ルート要素の取得
    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (root == nullptr) {
        std::cerr << "ルート要素が見つかりません。" << std::endl;
        xmlFreeDoc(doc);
        return;
    }

    // 指定されたIDの要素を検索
    xmlNodePtr element = find_element_by_id(root, object_id);
    if (element == nullptr) {
        std::cerr << "ID '" << object_id << "' の要素が見つかりません。" << std::endl;
        xmlFreeDoc(doc);
        return;
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

    // 変更されたSVGを出力
    xmlSaveFormatFileEnc("-", doc, "UTF-8", 1);

    // メモリを解放
    xmlFreeDoc(doc);
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "使用法: " << argv[0] << " <object_id> <translate_x> <translate_y>" << std::endl;
        return 1;
    }

    std::string object_id = argv[1];
    double translate_x = std::stod(argv[2]);
    double translate_y = std::stod(argv[3]);

    // 標準入力からSVGを読み込み、指定のIDのオブジェクトを移動
    translate_svg_object(std::cin, object_id, translate_x, translate_y);

    return 0;
}

