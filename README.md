# svglet

A lightweight CLI toolkit for SVG geometry computation and stream transformation.

Powered by `lib2geom` (Inkscape's geometry engine) and `libxml2`, `svglet` is designed to be chained in UNIX pipes (`sed`/`jq` style) to manipulate SVG structures or calculate exact curve lengths without spinning up heavy headless browser environments.

## Features

- **Geometric Path Length Computation**: Computes exact arc lengths of Bezier curves and polyline paths.
- **Stream Operations**: Reads from `stdin`, applies affine transformations (translate, rotate), and outputs to `stdout`.
- **Targeted Operations**: Applies transformations around element centers (circles, rectangles) via ID selection.

## Dependencies

- C++17 compiler
- `lib2geom` (>= 1.3)
- `libxml2`
- `boost`
- `cli11`
- `cmake` (>= 3.16)

### Arch Linux
```bash
sudo pacman -S lib2geom libxml2 boost cli11 cmake

```

### Ubuntu / Debian

```bash
sudo apt install lib2geom-dev libxml2-dev libboost-dev libcli11-dev cmake

```

## Build

```bash
cmake -B build -S .
cmake --build build
sudo cmake --install build

```

## Usage

### 1. Compute Path Length

Pass raw path strings directly:

```bash
svglet length "M 0 0 L 100 100"
# Output: 141.421

```

Or extract and compute the length of a specific `<path>` element from an SVG pipe:

```bash
cat drawing.svg | svglet length --id main_wire

```

### 2. Stream Transformations

Translate an element by (x, y):

```bash
cat input.svg | svglet translate target_id 10 -20 > output.svg

```

Rotate an element around another element's center:

```bash
cat input.svg | svglet rotate gear_1 45 --cid pivot_circle > output.svg

```

## License

MIT
