#include "svglet/svglet.h"
#include <2geom/2geom.h>
#include <2geom/circle.h>
#include <iostream>
#include <stdexcept>

xmlNodePtr find_element_by_id(xmlNodePtr node, const std::string& id) {
    for (xmlNodePtr cur = node; cur != nullptr; cur = cur->next) {
        if (cur->type == XML_ELEMENT_NODE) {
            xmlChar* prop = xmlGetProp(cur, (const xmlChar *)"id");
            if (prop) {
                bool match = (id == (const char *)prop);
                xmlFree(prop);
                if (match) return cur;
            }
        }
        xmlNodePtr found = find_element_by_id(cur->children, id);
        if (found) return found;
    }
    return nullptr;
}

static Geom::Point get_center_of_rect(xmlNodePtr element) {
    xmlChar* xAttr = xmlGetProp(element, (const xmlChar*) "x");
    xmlChar* yAttr = xmlGetProp(element, (const xmlChar*) "y");
    xmlChar* widthAttr  = xmlGetProp(element, (const xmlChar*) "width");
    xmlChar* heightAttr = xmlGetProp(element, (const xmlChar*) "height");

    if (!xAttr || !yAttr || !widthAttr || !heightAttr) {
        if (xAttr) xmlFree(xAttr);
        if (yAttr) xmlFree(yAttr);
        if (widthAttr) xmlFree(widthAttr);
        if (heightAttr) xmlFree(heightAttr);
        throw std::runtime_error("Missing attributes in rect element.");
    }

    double x = std::stod(reinterpret_cast<char *>(xAttr));
    double y = std::stod(reinterpret_cast<char *>(yAttr));
    double width = std::stod(reinterpret_cast<char *>(widthAttr));
    double height = std::stod(reinterpret_cast<char *>(heightAttr));

    xmlFree(xAttr);
    xmlFree(yAttr);
    xmlFree(widthAttr);
    xmlFree(heightAttr);

    return Geom::Point(x + width / 2.0, y + height / 2.0);
}

static Geom::Point get_center_of_circle(xmlNodePtr element) {
    xmlChar* cxAttr = xmlGetProp(element, (const xmlChar*) "cx");
    xmlChar* cyAttr = xmlGetProp(element, (const xmlChar*) "cy");
    if (!cxAttr || !cyAttr) {
        if (cxAttr) xmlFree(cxAttr);
        if (cyAttr) xmlFree(cyAttr);
        throw std::runtime_error("Missing attributes in circle element.");
    }

    double cx = std::stod(reinterpret_cast<char *>(cxAttr));
    double cy = std::stod(reinterpret_cast<char *>(cyAttr));
    xmlFree(cxAttr);
    xmlFree(cyAttr);

    return Geom::Circle(cx, cy, 0).center();
}

static Geom::Point get_center_of_element(xmlNodePtr root, const std::string& object_id) {
    xmlNodePtr element = find_element_by_id(root, object_id);
    if (!element) {
        throw std::runtime_error("ID '" + object_id + "' not found.");
    }
    if (xmlStrcmp(element->name, (const xmlChar*)"rect") == 0) {
        return get_center_of_rect(element);
    } else if (xmlStrcmp(element->name, (const xmlChar*)"circle") == 0) {
        return get_center_of_circle(element);
    }
    throw std::runtime_error("The center element type is not supported.");
}

xmlNodePtr rotate_svg_object(xmlNodePtr root, const std::string& object_id, const std::string& center_object_id, double angle) {
    xmlNodePtr element = find_element_by_id(root, object_id);
    if (!element) {
        std::cerr << "ID '" << object_id << "' is not found." << std::endl;
        return nullptr;
    }

    xmlChar* transform = xmlGetProp(element, (const xmlChar *)"transform");
    std::string new_transform = transform ? (const char *)transform : "";
    if (transform) xmlFree(transform);

    if (!center_object_id.empty()) {
        Geom::Point center = get_center_of_element(root, center_object_id);
        new_transform += " rotate(" + std::to_string(angle) + " " + std::to_string(center.x()) + " " + std::to_string(center.y()) + ")";
    } else {
        new_transform += " rotate(" + std::to_string(angle) + ")";
    }

    xmlSetProp(element, (const xmlChar *)"transform", (const xmlChar *)new_transform.c_str());
    return root;
}
