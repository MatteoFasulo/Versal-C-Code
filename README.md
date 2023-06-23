# Versal-C-Code

### Table of Contents
- [Introduction](#introduction)
- [Models](#models)
- [Usage](#usage)
- [Command-line arguments](#command-line-arguments)
- [C programs](#c-programs)
- [Python scripts](#python-scripts)
- [GCC](#gcc)
- [Known issues](#Known-issues)
- [References](#references)

### Introduction

BSc Thesis: Evaluation of timing performance of Deep Neural Networks workloads accelerated on Versal AI Engine in presence of contention on shared resources.

This repository contains the code used to test the inference time of the models on the Versal AI Engine.

The code is divided into 3 folders:
- [AgeGen](code/Python/main_subgraphs_age.py) => Age detection model
- [ResNet50](code/Python/resnet50.py) => ResNet50 model
- [SqueezeNet](code/Python/squeezenet.py) => SqueezeNet model

The code is divided into 2 parts:
- [C programs](code/C/) => C programs to force memory access and CPU usage
- [Python scripts](code/Python/) => Python scripts to run the models on the Versal AI Engine

Vitis AI Profiler aka vaitrace was used to profile the models. The DPU profiling results are available in the [csv](code/csv/) folder. The profiling results are divided according to the model they refer to. 

In order to profile the models, the following command was used:
```sh
vaitrace -c <CONFIG.json> -p --va ./<PYTHON_SCRIPT.py> 
``` 
where:
- `<PYTHON_SCRIPT.py>` => Python script to run the model
- ` -c  <CONFIG.json>` => configuration file
- `-p` => Trace python application
- `--va` => Generate trace data for Vitis Analyzer

<CONFIG.json> => configuration file:
```json
{
    "trace": {
        "enable_trace_list": ["vart", "vitis-ai-library", "opencv", "custom"]
    },
    "trace_custom": ["<FUNCTION_NAME>"]
}
```
where:
- `<FUNCTION_NAME>` => python function to trace

### Models
- `AgeGen/Age/Age.xmodel` => Age detection model
- `ResNet50/resnet50.xmodel` => ResNet50 model
    - [Model Zoo](https://github.com/Xilinx/Vitis-AI/blob/c55b7565bde608dd65dda94abea154ad7db4d594/model_zoo/model-list/pt_resnet50_imagenet_224_224_8.2G_3.0/model.yaml)
    - [Download](https://www.xilinx.com/bin/public/openDownload?filename=resnet50-vck190-r2.0.0.tar.gz)
- `SqueezeNet/squeezenet.xmodel` => SqueezeNet model
    - [Model Zoo](https://github.com/Xilinx/Vitis-AI/tree/c55b7565bde608dd65dda94abea154ad7db4d594/examples/vai_runtime/squeezenet_pytorch)
    - [Download](https://www.xilinx.com/bin/public/openDownload?filename=squeezenet_pt-vck190-r2.0.0.tar.gz)

### Usage
[test_inference](code/test_inference/) folder contains 100 images to test the response times:
- [test_inference](code/test_inference/) => 100 images

```sh
python3 main_subgraphs_age.py --images_dir test_inference/ --model AgeGen/Age/Age.xmodel --membomb ~/test/membomb/{bomb1,bomb1_nice,bomb2,bomb2_nice}
python3 ResNet50/resnet50.py --images_dir test_inference/ --model ResNet50/resnet50.xmodel --membomb ~/test/membomb/{bomb1,bomb1_nice,bomb2,bomb2_nice}
python3 SqueezeNet/squeezenet.py --images_dir test_inference/ --model SqueezeNet/squeezenet.xmodel --membomb ~/test/membomb/{bomb1,bomb1_nice,bomb2,bomb2_nice}
```

### Command-line arguments

Command-line arguments:
- `-d IMAGES_DIR`, `--images_dir IMAGES_DIR`: Path to folder of images. Default is images.
- `-m MODEL`, `--model MODEL`: Path of xmodel. Default is CNN.xmodel
- `-b MEMBOMB`, `--membomb MEMBOMB`: Path to memory bomb C program. Default to False.
- `-t THREAD`, `--thread THREAD`: Number of threads. Default is 1. Only for ResNet50 and SqueezeNet.

### C programs
- [bomb1.c](code/C/bomb1.c) => single core memory bomb without priority
- [bomb1_nice.c](code/C/bomb1_nice.c) => single core memory bomb with priority
- [bomb2.c](code/C/bomb2.c) => two core memory bomb without priority
- [bomb2_nice.c](code/C/bomb2_nice.c) => two core memory bomb with priority

### Python scripts
- [Age detection model](code/Python/main_subgraphs_age.py) => Age detection model
- [ResNet50](code/Python/resnet50.py) => ResNet50 model
- [SqueezeNet](code/Python/squeezenet.py) => SqueezeNet model

### GCC
Compile the C program with pthread
```sh
gcc -pthread <FILE.C> -o <OUTPUT>
```

### Known issues
If you get this error:
```sh
CHECK fingerprint fail ! model_fingerprint 0x602001036088231 dpu_fingerprint 0x602001036088211
```
Set the environment variable `XLNX_ENABLE_FINGERPRINT_CHECK` to `0`:
```sh
export XLNX_ENABLE_FINGERPRINT_CHECK=0
```

If you get this error:
```sh
[dpu_control_xrt_xv_dpu.cpp:193] dpu timeout! core_idx = 0 ...
```

You will need to create a bash script and execute it as soon as Versal is powered on:
```sh
#!/bin/bash
for i in {0..39}; do
    for j in {1..8}; do
        a=0x20000000000
        b=0x31000
        devmem $[a+b+(i<<23)+(j<<18)] 32 0
    done
done
```
> **Important**: The script must be executed as root and before running any DPU operation.


### References
- [[dpu_control_xrt_xv_dpu.cpp:193] dpu timeout! core_idx = 0](https://github.com/Xilinx/Vitis-AI/issues/576#issuecomment-957238529)
- [DPU-Fingerprint-Error](https://github.com/Xilinx/Vitis-AI/issues/975#issuecomment-1223452542)
