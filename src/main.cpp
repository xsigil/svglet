#include <2geom/2geom.h>
#include <2geom/circle.h>
#include <CLI/CLI.hpp>
#include <iostream>
#include <regex>
#include <svglet/svglet.h>
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

// 変換行列をパースする関数
Geom::Affine parseTransform(const std::string &transformStr) {
    Geom::Affine transform = Geom::identity();  // 初期化：単位行列
    
    // 各変換操作に対応する正規表現
    std::regex translateRegex("translate\\(([\\d\\.\\-e]+)[,\\s]*([\\d\\.\\-e]*)?\\)");
    std::regex rotateRegex("rotate\\(([\\d\\.\\-e]+)[,\\s]*([\\d\\.\\-e]*)[,\\s]*([\\d\\.\\-e]*)?\\)");
    std::regex scaleRegex("scale\\(([\\d\\.\\-e]+)[,\\s]*([\\d\\.\\-e]*)?\\)");
    std::regex matrixRegex("matrix\\(([\\d\\.\\-e]+)[,\\s]+([\\d\\.\\-e]+)[,\\s]+([\\d\\.\\-e]+)[,\\s]+([\\d\\.\\-e]+)[,\\s]+([\\d\\.\\-e]+)[,\\s]+([\\d\\.\\-e]+)\\)");

    // transform属性全体を処理するための正規表現
    std::regex transformRegex("(translate|rotate|scale|matrix)\\([^\\)]*\\)");

    std::smatch match;
    std::string::const_iterator searchStart(transformStr.cbegin());

    // 全体のtransform属性を順次パースして適用
    for(std::sregex_iterator it(std::begin(transformStr), std::end(transformStr), transformRegex), end; it != end; ++it){
        auto && m = *it;
        std::string transformCommand = m[0];

        // translateのパース
        if (std::regex_search(transformCommand, match, translateRegex)) {
            double tx = std::stod(match[1].str());
            double ty = match[2].str().empty() ? 0 : std::stod(match[2].str());
            transform *= Geom::Translate(tx, ty);
        }

        // rotateのパース
        else if (std::regex_search(transformCommand, match, rotateRegex)) {
            double angle = Geom::rad_from_deg(std::stod(match[1].str())) ;  // 度をラジアンに変換
            double cx = match[2].str().empty() ? 0 : std::stod(match[2].str());
            double cy = match[3].str().empty() ? 0 : std::stod(match[3].str());
            transform *= Geom::Translate(cx, cy) * Geom::Rotate(angle) * Geom::Translate(-cx, -cy);
        }

        // scaleのパース
        else if (std::regex_search(transformCommand, match, scaleRegex)) {
            double sx = std::stod(match[1].str());
            double sy = match[2].str().empty() ? sx : std::stod(match[2].str());
            transform *= Geom::Scale(sx, sy);
        }

        // matrixのパース
        else if (std::regex_search(transformCommand, match, matrixRegex)) {
            double a = std::stod(match[1].str());
            double b = std::stod(match[2].str());
            double c = std::stod(match[3].str());
            double d = std::stod(match[4].str());
            double e = std::stod(match[5].str());
            double f = std::stod(match[6].str());
            transform *= Geom::Affine(a, b, c, d, e, f);
        }
    }
    return transform;
}

Geom::Point get_center_of_rect(xmlNodePtr element, Geom::Affine transform){
    xmlChar* xAttr = xmlGetProp(element,(const xmlChar*) "x");
    xmlChar* yAttr = xmlGetProp(element,(const xmlChar*) "y");
    xmlChar* widthAttr  = xmlGetProp(element,(const xmlChar*) "width");
    xmlChar* heightAttr = xmlGetProp(element,(const xmlChar*) "height");
    double x = std::stod(reinterpret_cast<char *>(xAttr));
    double y = std::stod(reinterpret_cast<char *>(yAttr));
    double width = std::stod(reinterpret_cast<char *>(widthAttr));
    double height = std::stod(reinterpret_cast<char *>(heightAttr));

    Geom::Rect rect(x,y,x+width, y+height); 


    Geom::Rect transformedRect = rect*transform;
    // 矩形の4つのコーナーを取得
    Geom::Point topLeft = transformedRect.corner(0);      // 左上
    Geom::Point topRight = transformedRect.corner(1);     // 右上
    Geom::Point bottomRight = transformedRect.corner(2);  // 右下
    Geom::Point bottomLeft = transformedRect.corner(3);   // 左下
    Geom::Point center = (topLeft + topRight + bottomRight + bottomLeft) / 4.0;
    xmlFree(xAttr);
    xmlFree(yAttr);
    xmlFree(widthAttr);
    xmlFree(heightAttr);
 
    return center;
}

Geom::Point get_center_of_circle(xmlNodePtr element, Geom::Affine transform){
    xmlChar* cxAttr = xmlGetProp(element,(const xmlChar*) "cx");
    xmlChar* cyAttr = xmlGetProp(element,(const xmlChar*) "cy");
    double cx = std::stod(reinterpret_cast<char *>(cxAttr));
    double cy = std::stod(reinterpret_cast<char *>(cyAttr));

    Geom::Circle circle(cx, cy, 0);
      // 中心にアフィン変換を適用
    Geom::Point transformedCenter = circle.center() * transform;

    // x方向とy方向のスケール要素を取得
    double scaleX = transform[0]; // x方向のスケーリング要素
    double scaleY = transform[3]; // y方向のスケーリング要素

    // 半径に対してスケーリング要素を適用（ここでは平均値を使用）
    double transformedRadius = circle.radius() * (scaleX + scaleY) / 2.0;

    // 変換後の円を返す
    return Geom::Circle(transformedCenter, transformedRadius).center();
}



Geom::Point get_center_of_element_after_transform(xmlNodePtr root, std::string object_id){
    xmlNodePtr element = find_element_by_id(root, object_id);
    if (element == nullptr) {
        std::cerr << "ID '" << object_id << "' の要素が見つかりません。" << std::endl;
        throw std::exception();
    }


    // iterate to root 
    xmlNodePtr current = element;
    Geom::Affine transform;
    while(current != nullptr){
        if (current->name != nullptr) {
            if(xmlStrcmp(current->name, (const xmlChar*)"g")==0){
                xmlChar* transformAttr=xmlGetProp(current, (const xmlChar*)"transform");
                if(transformAttr != nullptr){
                    transform *= parseTransform((char*) transformAttr);
                }
                xmlFree(transformAttr);
            }
        }
        current = current->parent;
    }
    if(xmlStrcmp(element->name, (const xmlChar*)"rect")==0){
        return get_center_of_rect(element, transform);
    }else if(xmlStrcmp(element->name, (const xmlChar*)"circle")==0){
        return get_center_of_circle(element, transform); 
    }
    throw std::runtime_error("The center element is not supported.");
}

xmlNodePtr rotate_svg_object(xmlNodePtr root, const std::string& object_id, std::string& center_object_id, double angle){
    // 指定されたIDの要素を検索
    xmlNodePtr element = find_element_by_id(root, object_id);
    if (element == nullptr) {
        std::cerr << "ID '" << object_id << "' の要素が見つかりません。" << std::endl;
        return nullptr;
    }

    Geom::Point center = get_center_of_element_after_transform(root, center_object_id);

    xmlChar* transform = xmlGetProp(element, (const xmlChar *)"transform");
    std::string new_transform;
    if (transform != nullptr) {
        new_transform = (const char *)transform;  // nullptrチェック
        xmlFree(transform);
    } else {
        new_transform = "";  // transform属性がない場合は空の文字列を使う
    }

    // translate(x, y)を追加
    new_transform += " rotate(" + std::to_string(angle) + " " + std::to_string(center.x()) + " " + std::to_string(center.y()) + ")";
    xmlSetProp(element, (const xmlChar *)"transform", (const xmlChar *)new_transform.c_str());

    return root;
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

        xmlNodePtr root = xmlDocGetRootElement(doc);
        if (root == nullptr) {
            std::cerr << "ルート要素が見つかりません。" << std::endl;
            xmlFreeDoc(doc);
            return 1;
        }


        // execute translate subcommand
        if (*translate) {
            root = translate_svg_object(root, object_id, translate_x, translate_y);
        }

        if (*rotate) {
            root = rotate_svg_object(root, object_id, center_object_id, angle);
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

