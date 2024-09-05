# コンパイラの設定
CXX = g++
CXXFLAGS = $(shell pkg-config --cflags 2geom) $(shell xml2-config --cflags) -Isrc/include
LDFLAGS = $(shell pkg-config --libs 2geom) $(shell xml2-config --libs)

# ターゲットとソース
TARGET = bin/svglet
#SRCS = src/path_length.cpp src/svg_translate.cpp
SRCS = $(wildcard src/*.cpp)

# オブジェクトファイル
OBJS = $(SRCS:.cpp=.o)

# デフォルトのターゲット
all: $(TARGET)

# ターゲットのビルドルール
$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LDFLAGS)

# .cppファイルを.oにコンパイルするルール
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# クリーンアップ
clean:
	rm -f $(OBJS) $(TARGET)

