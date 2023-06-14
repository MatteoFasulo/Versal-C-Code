# Versal-C-Code

### Memory Bomb
- `bomb1.c` => single core memory bomb without priority
- `bomb1_nice.c` => single core memory bomb with priority
- `bomb2.c` => two core memory bomb without priority
- `bomb2_nice.c` => two core memory bomb with priority

### Models
- `AgeGen/Age/Age.xmodel` => Age detection model
- `ResNet50/resnet50.xmodel` => ResNet50 model

### Usage
`test_inference` folder contains 100 images to test the response times:
- `test_inference/` => 100 images
- `unico/` => 1 image

```sh
python3 main_subgraphs_age.py --images_dir test_inference/ --model AgeGen/Age/Age.xmodel --membomb ~/test/membomb/{bomb1,bomb1_nice,bomb2,bomb2_nice}
python3 ResNet50/resnet50.py --images_dir test_inference/ --model ResNet50/resnet50.xmodel --thread 1 --membomb ~/test/membomb/{bomb1,bomb1_nice,bomb2,bomb2_nice}
```
>**Info** there will be a `csv` folder with csv files of response times. 
The program will create a csv file named **<MODEL_NAME>_<MEM_BOMB_TYPE>.csv**. 
If you run without memory bomb it will create a file named **<MODEL_NAME>.csv**

Command-line arguments:
- `-d IMAGES_DIR`, `--images_dir IMAGES_DIR`: Path to folder of images. Default is images.
- `-m MODEL`, `--model MODEL`: Path of xmodel. Default is CNN.xmodel
- `-b MEMBOMB`, `--membomb MEMBOMB`: Path to memory bomb C program. Default to False
- `-t THREAD`, `--thread THREAD`: Number of threads. Default is 1. Only for ResNet50

### GCC
Compile the C program with pthread
```sh
gcc -pthread <FILE.C> -o <OUTPUT>
```

### Xilinx Vitis-AI
Compile the xmodel with Vitis-AI
```sh
vai_c_tensorflow2 -m <MODEL.H5> -a <ARCH.JSON> -o <OUTPUT>
```

### Model and DPU fingerprint check failed
If you get this error:
```sh
CHECK fingerprint fail ! model_fingerprint 0x602001036088231 dpu_fingerprint 0x602001036088211
```
Set the environment variable `XLNX_ENABLE_FINGERPRINT_CHECK` to `0`:
```sh
export XLNX_ENABLE_FINGERPRINT_CHECK=0
```

### References
- [DPU-Fingerprint-Error](https://github.com/Xilinx/Vitis-AI/issues/975#issuecomment-1223452542)
