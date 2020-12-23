# RSD Report S Device

## How to use 

Follow detailed instructions provided specifically for this example. 

Select the instructions depending on Espressif chip installed on your development board:

- [ESP32 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html)
- [ESP32-S2 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/get-started/index.html)

Make sure have esp32 idf installed 
run at RSD directory
```
idf.py build && idf.py flash
```

## RSD folder contents

The project **RSD** contains eight source file in C language. The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt` files that provide set of directives and instructions describing the project's source files and targets (executable, library, or both). 

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── example_test.py            Python script used for automated example testing
├── main
│   ├── CMakeLists.txt
│   ├── main.c                   main() entry
│   ├── wifi_connect.c           Wifi connect task
│   ├── sound_task.c             Sound task gathering sound amp
│   ├── time_task.c              Time task to print time periodically
│   ├── nas_task.c               NAS submit record request
│   ├── parser_func.c            Parser functions
│   ├── reboot_task.c            Reboot timer 120mins
│   ├── freewifi_req.c           Free WIFI auth request
│   └── include
│          ├── main.h
│          ├── wifi_connect.h
│          ├── sound_task.h
│          ├── time_task.h
│          ├── nas_task.h
│          ├── parser_func.h
│          ├── reboot_task.h
│          ├── nas_queue.h
│          └── freewifi_req.h
├── Makefile                   Makefile used by legacy GNU Make
└── README.md                  This is the file you are currently reading
```

## Troubleshooting

* Program upload failure

    * Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
    * The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.
