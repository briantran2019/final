## Final Project

This is a culumination of the past homeworks working all together. The point of these past homeworks was to take the repositories and transform them into individual libraries to make it easier to integrate into other projects.

- SSD1306 library will handle displaying messages onto the screen.
- BMP280 library will capture pressure and temperature.
- MOSQUITTO library will handle tasking via mqtt protocol.

I've provided two source files, a `subscriber.c` and a `publisher.c`.

### `subscriber.c`
This is similiar to the `mosquitto_sub` binary in that it will wait for a message on a specific topic. The `message_callback` function is where you need to make some modifications. You'll need to handle specific tasking given provided objects that are `task`, `string_msg`, `int_msg`, and for extra credit `morse`.

- `string_msg`
  - This is simply a message that is written to the OLED.

- `int_msg`
  - This is simply a message that is written to the OLED.

- `task`
  - This task can contain the messages `get_temperature`, `get_pressure`, and `get_temperature_pressure`.
  - Use the library you created for the `bmp280` and print the results onto your OLED screen.
  - Extra credit: publish the message to a `taskingcomplete` topic with the results, use the objects `temp` and `pressure`.

- Extra credit: `morse`
  - Use the blink led code to blink the onboard LED.
  - Print the morse code on the OLED.

### `publisher.c`
This is an example publisher of the code that will send out messages. You can use it with your subscriber for testings and ensuring that your code is working with some sample messages.

The `subscriber` and `publisher` will need to use a specific topic for grading purposes, each team (even it working alone) will need a unique topic name after you request it from me.

## Compiling the `subscriber` and `publisher`.

You'll need the following folders from the mosquitto homework.

- install_cv1800b_cjson
- install_cv1800b_openssl
- install_cv1800b_mqtt

Here's an updated `milkv_duo.make` toolchain file

```cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv)

set(TOOLCHAIN_PREFIX $ENV{TOOLCHAIN_PREFIX})
set(SYSROOT $ENV{SYSROOT})

set(CMAKE_SYSROOT ${SYSROOT})
set(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}g++")

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")

include_directories("/app/install_cv1800b_mqtt/include")
include_directories("/app/install_cv1800b_cjson/include")
link_directories("/app/install_cv1800b_cjson/lib")
link_directories("/app/install_cv1800b_mqtt/lib")
link_directories("/app/install_cv1800b_mqtt/lib")
link_directories("/app/install_cv1800b_openssl/lib")

set(CMAKE_FIND_ROOT_PATH "/app/install_cv1800b_openssl" "/app/install_cv1800b_cjson" "/app/install_cv1800b_mqtt")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lwiringx -lcjson -lmosquitto -lssl -lcrypto")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lwiringx")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -lwiringx")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

and a `CMakeLists.txt`
```cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv)

set(TOOLCHAIN_PREFIX $ENV{TOOLCHAIN_PREFIX})
set(SYSROOT $ENV{SYSROOT})

set(CMAKE_SYSROOT ${SYSROOT})
set(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}g++")

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")

include_directories("/app/install_cv1800b_mqtt/include")
include_directories("/app/install_cv1800b_cjson/include")

set(CMAKE_FIND_ROOT_PATH "/app/install_cv1800b_openssl" "/app/install_cv1800b_cjson" "/app/install_cv1800b_mqtt")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lwiringx")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lwiringx")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -lwiringx")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

### Configure CMake
```bash
docker run --rm -v $PWD:/app -v $PWD/install_cv1800b_openssl:/app/install_cv1800b_openssl -v $PWD/install_cv1800b_cjson:/app/install_cv1800b_cjson -v $PWD/install_cv1800b_mqtt:/app/install_cv1800b_mqtt ejortega/duo-sdk bash -c "mkdir build && cd build && cmake -DCMAKE_TOOLCHAIN_FILE=/app/milkv_duo.cmake .."
```

### Compile
```bash
docker run --rm -v $PWD:/app -v $PWD/install_cv1800b_openssl:/app/install_cv1800b_openssl -v $PWD/install_cv1800b_cjson:/app/install_cv1800b_cjson -v $PWD/install_cv1800b_mqtt:/app/install_cv1800b_mqtt ejortega/duo-sdk bash -c "cd build && make"
```

**NOTE: This only compiles the provided code. You will need to make some modifications to include your libraries.**

## Results
There's multiple was to approach this. My recommendation is to copy the library folders to your working directory and modify the `CMakeLists.txt` with `include_directories` option to point to the each library's header, compile the library, and link it to the `subscriber` binary using `target_link_library` option. Another option is to just throw all the code together and compile everything at once.

**The results will be presented in class during our scheduled final on May 6th from 3:00 - 4:50.**