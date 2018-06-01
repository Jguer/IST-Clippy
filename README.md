# Clippy

Clippy is a fault resistant distributed clipboard.
A public library is available in `library/clipboard.h` for usage in apps.

## Building

```
make
```

## Usage

#### Server

```
Example Usage: ./xclippy
Example Usage: ./xclippy -c [ip] [port] -d [clipboard dir]
```

#### Test Apps

```
Example: ./xminifuzzer
```

## Documentation

```
doxygen Doxyfile
xdg-open docs/html/index.html
```

or

```
make docs
```

## External libraries used

[log.c](https://github.com/rxi/log.c/)
[lists](https://github.com/udp/list)
