#include "2geom/rect.h"
#include "2geom/transforms.h"
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


//int main() {
//    // 複数の変換が含まれるtransform属性の文字列例
//    std::string transformStr = "translate(30,40) rotate(45) scale(2,2) matrix(1, 0, 0, 1, 20, 30)";
//
//    // transform属性をパースして変換行列を取得
//    Geom::Affine transform = parseTransform(transformStr);
//
//    // 変換後の結果を確認
//    Geom::Point p(10, 10);  // 座標点 (10, 10)
//    Geom::Point transformedP = p * transform;  // 変換行列を適用
//
//    // 結果を表示
//    std::cout << "Original point: " << p << std::endl;
//    std::cout << "Transformed point: " << transformedP << std::endl;
//
//    return 0;
//}



//// 変換行列をパースする関数 Geom::Affine parseTransform(const std::string &transformStr) { Geom::Affine transform = Geom::identity();  // 初期化：単位行列 // 各変換操作に対応する正規表現 std::regex translateRegex("translate\\(([\\d\\.\\-e]+)[,\\s]*([\\d\\.\\-e]*)?\\)"); std::regex rotateRegex("rotate\\(([\\d\\.\\-e]+)[,\\s]*([\\d\\.\\-e]*)[,\\s]*([\\d\\.\\-e]*)?\\)"); std::regex scaleRegex("scale\\(([\\d\\.\\-e]+)[,\\s]*([\\d\\.\\-e]*)?\\)");
//    std::regex matrixRegex("matrix\\(([\\d\\.\\-e]+)[,\\s]+([\\d\\.\\-e]+)[,\\s]+([\\d\\.\\-e]+)[,\\s]+([\\d\\.\\-e]+)[,\\s]+([\\d\\.\\-e]+)[,\\s]+([\\d\\.\\-e]+)\\)");
//
//    // transform属性全体を処理するための正規表現
//    std::regex transformRegex("(translate|rotate|scale|matrix)\\([^\\)]*\\)");
//
//    std::smatch match;
//    std::string::const_iterator searchStart(transformStr.cbegin());
//
//    // 全体のtransform属性を順次パースして適用
//    while (std::regex_search(searchStart, transformStr.cend(), match, transformRegex)) {
//        std::cout << match.str() << std::endl;
//        //std::string transformCommand = match.str();
//        //
//        //// translateのパース
//        //if (std::regex_search(transformCommand, match, translateRegex)) {
//        //    double tx = std::stod(match[1].str());
//        //    double ty = match[2].str().empty() ? 0 : std::stod(match[2].str());
//        //    transform *= Geom::Translate(tx, ty);
//        //}
//
//        //// rotateのパース
//        //else if (std::regex_search(transformCommand, match, rotateRegex)) {
//        //    double angle = Geom::rad_from_deg(std::stod(match[1].str())) ;  // 度をラジアンに変換
//        //    double cx = match[2].str().empty() ? 0 : std::stod(match[2].str());
//        //    double cy = match[3].str().empty() ? 0 : std::stod(match[3].str());
//        //    transform *= Geom::Translate(cx, cy) * Geom::Rotate(angle) * Geom::Translate(-cx, -cy);
//        //}
//
//        //// scaleのパース
//        //else if (std::regex_search(transformCommand, match, scaleRegex)) {
//        //    double sx = std::stod(match[1].str());
//        //    double sy = match[2].str().empty() ? sx : std::stod(match[2].str());
//        //    transform *= Geom::Scale(sx, sy);
//        //}
//
//        //// matrixのパース
//        //else if (std::regex_search(transformCommand, match, matrixRegex)) {
//        //    double a = std::stod(match[1].str());
//        //    double b = std::stod(match[2].str());
//        //    double c = std::stod(match[3].str());
//        //    double d = std::stod(match[4].str());
//        //    double e = std::stod(match[5].str());
//        //    double f = std::stod(match[6].str());
//        //    transform *= Geom::Affine(a, b, c, d, e, f);
//        //}
//
//        //// 次のtransformを探すためにイテレータを更新
//        searchStart = match.suffix().first;
//    }
//
//    return transform;
//}
//
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
            xmlNodePtr element = find_element_by_id(root, object_id);
            if (element == nullptr) {
                std::cerr << "ID '" << object_id << "' の要素が見つかりません。" << std::endl;
                xmlFreeDoc(doc);
                return 1;
            }


            xmlChar* xAttr = xmlGetProp(element,(const xmlChar*) "x");
            xmlChar* yAttr = xmlGetProp(element,(const xmlChar*) "y");
            xmlChar* widthAttr  = xmlGetProp(element,(const xmlChar*) "width");
            xmlChar* heightAttr = xmlGetProp(element,(const xmlChar*) "height");
            double x = std::stod(reinterpret_cast<char *>(xAttr));
            double y = std::stod(reinterpret_cast<char *>(yAttr));
            double width = std::stod(reinterpret_cast<char *>(widthAttr));
            double height = std::stod(reinterpret_cast<char *>(heightAttr));

            Geom::Rect rect(x,y,x+width, y+height); 

            //    // iterate to root 
            xmlNodePtr current = element;
            Geom::Affine transform;
            while(current != nullptr){
                if (current->name != nullptr) {
                    if(xmlStrcmp(current->name, (const xmlChar*)"g")==0){
                        xmlChar* transformAttr=xmlGetProp(current, (const xmlChar*)"transform");
                        if(transformAttr != nullptr){
                            std::cout << transformAttr;
                            transform *= parseTransform((char*) transformAttr);
                        }
                        xmlFree(transformAttr);
                    }
                }
                current = current->parent;
            }
            Geom::Rect transformedRect = rect*transform;
            std::cout << transformedRect << std::endl;
            xmlFree(xAttr);
            xmlFree(yAttr);
            xmlFree(widthAttr);
            xmlFree(heightAttr);
            
        }
        //    doc = translate_svg_object(doc, object_id, translate_x, translate_y);

        //if (*rotate) {
        //    doc = rotate_svg_object(doc, object_id, center_object_id, angle);

        //}

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

