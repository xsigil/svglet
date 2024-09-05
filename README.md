以下は、`svglet` のREADMEファイルの英語版例です。

---

# svglet

`svglet` is a command-line tool that allows you to easily manipulate SVG images from the terminal. It provides several transformation commands to apply translations, rotations, scaling, and more to your SVG files.

## Features

- **translate**: Move an object by a specified x and y offset.
- **rotate**: Rotate an object by a given angle.
- **scale**: Scale an object by specified x and y scaling factors.
- **skewX**: Skew an object along the X-axis by a specified angle.
- **skewY**: Skew an object along the Y-axis by a specified angle.
- **matrix**: Apply a 2D transformation matrix to an object.

## Installation

To install `svglet`, clone the repository and build the tool using a C++ compiler.

```bash
git clone https://github.com/yourusername/svglet.git
cd svglet
g++ -std=c++11 -o svglet svglet.cpp
```

This command will create an executable file named `svglet`.

## Usage

`svglet` offers several subcommands for manipulating SVG elements. Below are the available subcommands and their respective options:

### translate

Move an object by the specified x and y offsets.

```bash
./svglet translate <x> <y>
```

Example:

```bash
./svglet translate 10 20
# Translates the object by 10 units in the X direction and 20 units in the Y direction.
```

### rotate

Rotate an object by a specified angle in degrees.

```bash
./svglet rotate <angle>
```

Example:

```bash
./svglet rotate 45
# Rotates the object by 45 degrees.
```

### scale

Scale an object by specified x and y scaling factors.

```bash
./svglet scale <sx> <sy>
```

Example:

```bash
./svglet scale 2 3
# Scales the object by 2x in the X direction and 3x in the Y direction.
```

### skewX

Skew an object along the X-axis by the specified angle.

```bash
./svglet skewX <angle>
```

Example:

```bash
./svglet skewX 30
# Skews the object along the X-axis by 30 degrees.
```

### skewY

Skew an object along the Y-axis by the specified angle.

```bash
./svglet skewY <angle>
```

Example:

```bash
./svglet skewY 20
# Skews the object along the Y-axis by 20 degrees.
```

### matrix

Apply a 2D transformation matrix to an object. You must provide six matrix values: a, b, c, d, e, and f.

```bash
./svglet matrix <a> <b> <c> <d> <e> <f>
```

Example:

```bash
./svglet matrix 1 0 0 1 10 20
# Applies the transformation matrix [1 0 0 1 10 20] to the object.
```

## Example

You can chain multiple transformations to manipulate an SVG element in different ways.

```bash
./svglet translate 10 20
./svglet rotate 45
./svglet scale 1.5 1.5
```

## License

`svglet` is licensed under the MIT License. See the `LICENSE` file for more details.

---

This README provides a clear overview of what `svglet` does, how to install and use it, and examples of each transformation. You can modify it further as per your project's needs.
