# RabbitMQ C++ AMQP client library

## Introduction

This is a C++ language header only AMQP client library. It currently is a wrapper around the [rabbitmq-c](https://github.com/alanxz/rabbitmq-c) library.

## Quick Start

Since this is a header only library there is no building and there aren't many dependancies if building and running tests/examples isn't neccecairy. Everything needed (besides the [rabbitmq-c](https://github.com/alanxz/rabbitmq-c) library) is in the include directory. It is expected that the developer would provide include and library paths to link and include [rabbitmq-c](https://github.com/alanxz/rabbitmq-c) in his own project.

### Dependancies:
- C++11 and C++14 require [mpark::variant](https://github.com/mpark/variant.git) (which is a submodule of this repository)
- [rabbitmq-c](https://github.com/alanxz/rabbitmq-c) binaries for runtime and includes at compile time

## Building and running tests, examples and generating documentation

After recursively cloning you need to use CMake >= 3.13 to build the tests and generate docs.

    git clone --recurse-submodules https://github.com/djsavic1988/rabbitmq-cxx
    cd rabbitmq-cxx
    mkdir build && cd build
    cmake -DBUILD_DOC=ON -DBUILD_EXAMPLES=ON -DBUILD_UNIT_TESTS=ON ..
    ./librabbitmq-cxx-tests  # runs tests

### Required for generating documentation
- [Doxygen] (https://www.doxygen.nl/index.html)

## Threading

The restrictions for the [rabbitmq-c](https://github.com/alanxz/rabbitmq-c) apply to this library
