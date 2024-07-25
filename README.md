# cosmic

It's a modern c++ library which targets to a high performance web server.

## Dev Env

- Fedora 40
- gcc 14.1
- cmake

```shell
sudo dnf install boost-devel libubsan libasan
```

## Project Directories

- src - source code
- lib - library output
- bin - binary output
- tests - testing code
- build - build intermediate
- deps - third-party libraries

## Log System

1. Log4J

```text
   Logger (Define Logger)
   |
   |-------Formatter
   |
   Appender (Log output)
```

## Config System

I use yaml to config our system by leveraging [yaml-cpp](https://github.com/jbeder/yaml-cpp).

## Thread

