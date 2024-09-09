#include <2geom/2geom.h>
#include <2geom/circle.h>
#include <CLI/CLI.hpp>
#include <iostream>
#include <regex>
#include <svglet/svglet.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stack>

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

Geom::Affine parseTransform(const std::string &transformStr) {
    Geom::Affine transform = Geom::identity();  // 初期化：単位行列
    std::regex translateRegex("translate\\(([\\d\\.\\-e]+)[,\\s]*([\\d\\.\\-e]*)?\\)");
    std::regex rotateRegex("rotate\\(([\\d\\.\\-e]+)[,\\s]*([\\d\\.\\-e]*)[,\\s]*([\\d\\.\\-e]*)?\\)");
    std::regex scaleRegex("scale\\(([\\d\\.\\-e]+)[,\\s]*([\\d\\.\\-e]*)?\\)");
    std::regex matrixRegex("matrix\\(([\\d\\.\\-e]+)[,\\s]+([\\d\\.\\-e]+)[,\\s]+([\\d\\.\\-e]+)[,\\s]+([\\d\\.\\-e]+)[,\\s]+([\\d\\.\\-e]+)[,\\s]+([\\d\\.\\-e]+)\\)");

    std::regex transformRegex("(translate|rotate|scale|matrix)\\([^\\)]*\\)");

    std::smatch match;
    std::string::const_iterator searchStart(transformStr.cbegin());

    for(std::sregex_iterator it(std::begin(transformStr), std::end(transformStr), transformRegex), end; it != end; ++it){
        auto && m = *it;
        std::string transformCommand = m[0];

        if (std::regex_search(transformCommand, match, translateRegex)) {
            double tx = std::stod(match[1].str());
            double ty = match[2].str().empty() ? 0 : std::stod(match[2].str());
            transform *= Geom::Translate(tx, ty);
        }

        else if (std::regex_search(transformCommand, match, rotateRegex)) {
            double angle = Geom::rad_from_deg(std::stod(match[1].str())) ;  // 度をラジアンに変換
            double cx = match[2].str().empty() ? 0 : std::stod(match[2].str());
            double cy = match[3].str().empty() ? 0 : std::stod(match[3].str());
            transform *= Geom::Translate(cx, cy) * Geom::Rotate(angle) * Geom::Translate(-cx, -cy);
        }

        else if (std::regex_search(transformCommand, match, scaleRegex)) {
            double sx = std::stod(match[1].str());
            double sy = match[2].str().empty() ? sx : std::stod(match[2].str());
            transform *= Geom::Scale(sx, sy);
        }

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

Geom::Point get_center_of_rect(xmlNodePtr element){
    xmlChar* xAttr = xmlGetProp(element, (const xmlChar*) "x");
    xmlChar* yAttr = xmlGetProp(element, (const xmlChar*) "y");
    xmlChar* widthAttr  = xmlGetProp(element, (const xmlChar*) "width");
    xmlChar* heightAttr = xmlGetProp(element, (const xmlChar*) "height");

    if (xAttr == nullptr || yAttr == nullptr || widthAttr == nullptr || heightAttr == nullptr) {
        std::cerr << "Error: Missing attributes in rect element." << std::endl;
        throw std::runtime_error("Missing attributes in rect element.");
    }

    double x = std::stod(reinterpret_cast<char *>(xAttr));
    double y = std::stod(reinterpret_cast<char *>(yAttr));
    double width = std::stod(reinterpret_cast<char *>(widthAttr));
    double height = std::stod(reinterpret_cast<char *>(heightAttr));

    Geom::Point center = Geom::Point(x + width / 2.0, y + height / 2.0);

    xmlFree(xAttr);
    xmlFree(yAttr);
    xmlFree(widthAttr);
    xmlFree(heightAttr);

    return center;
}

Geom::Point get_center_of_circle(xmlNodePtr element){
    xmlChar* cxAttr = xmlGetProp(element,(const xmlChar*) "cx");
    xmlChar* cyAttr = xmlGetProp(element,(const xmlChar*) "cy");
    double cx = std::stod(reinterpret_cast<char *>(cxAttr));
    double cy = std::stod(reinterpret_cast<char *>(cyAttr));

    Geom::Circle circle(cx, cy, 0);
    return circle.center();
}


Geom::Affine collect_and_apply_transform(xmlNodePtr element) {
    std::stack<Geom::Affine> transformStack;  // 親の変換を保存するスタック
    Geom::Affine totalTransform = Geom::identity();  // 初期化

    xmlNodePtr current = element;
    while (current != nullptr) {
        if (current->name != nullptr) {
            if (xmlStrcmp(current->name, (const xmlChar*)"g") == 0) {
                xmlChar* transformAttr = xmlGetProp(current, (const xmlChar*)"transform");
                if (transformAttr != nullptr) {
                    Geom::Affine transform = parseTransform((char*) transformAttr);
                    transformStack.push(transform);  // 親から順にスタックに保存
                    xmlFree(transformAttr);
                }
            }
        }
        current = current->parent;
    }

    while (!transformStack.empty()) {
        totalTransform *= transformStack.top();  
        transformStack.pop();
    }

    return totalTransform;
}


Geom::Point get_center_of_element(xmlNodePtr root, std::string object_id){
    xmlNodePtr element = find_element_by_id(root, object_id);
    if (element == nullptr) {
        std::cerr << "ID '" << object_id << "' is not found" << std::endl;
        throw std::exception();
    }
    xmlNodePtr current = element;
    if(xmlStrcmp(element->name, (const xmlChar*)"rect")==0){
        return get_center_of_rect(element);
    }else if(xmlStrcmp(element->name, (const xmlChar*)"circle")==0){
        return get_center_of_circle(element); 
    }
    throw std::runtime_error("The center element is not supported.");
}

xmlNodePtr rotate_svg_object(xmlNodePtr root, const std::string& object_id, std::string& center_object_id, double angle){
    xmlNodePtr element = find_element_by_id(root, object_id);
    xmlChar* transform = xmlGetProp(element, (const xmlChar *)"transform");
    std::string new_transform;
    if (transform != nullptr) {
        new_transform = (const char *)transform;  
        xmlFree(transform);
    } else {
        new_transform = "";  
    }

    if(!center_object_id.empty()){
        if (element == nullptr) {
            std::cerr << "ID '" << object_id << "' is not found." << std::endl;
            return nullptr;
        }

        Geom::Point center = get_center_of_element(root, center_object_id);

        xmlChar* transform = xmlGetProp(element, (const xmlChar *)"transform");
        new_transform += " rotate(" + std::to_string(angle) + " " + std::to_string(center.x()) + " " + std::to_string(center.y()) + ")";
        xmlSetProp(element, (const xmlChar *)"transform", (const xmlChar *)new_transform.c_str());
    }else{
        new_transform += " rotate(" + std::to_string(angle) + ")";
        xmlSetProp(element, (const xmlChar *)"transform", (const xmlChar *)new_transform.c_str());
    }
    return root;
}

int main(int argc, char **argv) {
    CLI::App app{"svglet - A simple tool for applying SVG transformations"};

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

