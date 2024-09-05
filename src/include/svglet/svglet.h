#ifndef SVG_TRANSFORM_H
#define SVG_TRANSFORM_H

#include <libxml/parser.h>
#include <2geom/2geom.h>
#include <string>

// translate_svg_object 関数の宣言
xmlDocPtr translate_svg_object(xmlDocPtr doc, const std::string& object_id, double translate_x, double translate_y);

xmlDocPtr rotate_svg_object(xmlDocPtr doc, const std::string& object_id, const std::string& center_object_id, double angle);

xmlNodePtr find_element_by_id(xmlNodePtr node, const std::string& id) ;

std::pair<double, double> get_rect_center_with_transform(xmlNode* node, const Geom::Affine& transform);

std::pair<double, double> get_circle_center_with_transform(xmlNode* node, const Geom::Affine& transform);
#endif // SVG_TRANSFORM_H

