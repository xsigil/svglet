#pragma once
#include <libxml/tree.h>
#include <string>

// 要素探索
xmlNodePtr find_element_by_id(xmlNodePtr node, const std::string& id);

// トランスフォーム
xmlNodePtr translate_svg_object(xmlNodePtr root, const std::string& object_id, double tx, double ty);
xmlNodePtr rotate_svg_object(xmlNodePtr root, const std::string& object_id, const std::string& center_object_id, double angle);

// 弧長計算
double compute_svg_path_length(const std::string& path_data);
double compute_svg_element_path_length(xmlNodePtr root, const std::string& object_id);
