#include <2geom/path.h>
#include <2geom/pathvector.h>
#include <2geom/svg-path-parser.h>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    // 引数の数をチェック
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <SVG path data>" << std::endl;
        return 1;
    }

    // 引数からパスデータを取得
    std::string path_data = argv[1];

    try {
        // SVGパス文字列をパースしてPathVectorを作成
        Geom::PathVector pathv = Geom::parse_svg_path(path_data.c_str());

        // PathVectorをPiecewiseに変換して長さを計算
        for (const auto& path : pathv) {
            Geom::Piecewise<Geom::D2<Geom::SBasis>> piecewise = path.toPwSb();
            double length = Geom::length(piecewise);
            std::cout << "Path length: " << length << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing SVG path: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}



