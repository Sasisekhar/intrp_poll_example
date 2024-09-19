| Tested Targets | ESP32 | ESP32-S3 |
| -------------- | ----- | -------- |

# _Interrupt-Poll Example_

This example helps to compare the performance of recieveing input via interrupts vs polls. The model can be simulated using the CADMIUM DEVS simulator, and can be executed on the ESP32S3

## How to use example

This example is meant to run on Linux.

For dependancies, run (pay attention, you will be asked for your password):
```sh
./install_dependencies.sh
```
The only external dependency is pahomqtt for c++

For building the project, run:
```sh
cmake .
make
```
For running:
```sh
./GPT
```

## Example folder contents

The source files of this project can be found in the **main** directory. The entry point file is the [main.cpp](main/main.cpp) file.

The submodels can be found in the **include** directory

```
├── main
│   ├── include
│   └── main.cpp
└── README.md                  This is the file you are currently reading
```
