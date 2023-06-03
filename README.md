# Versal-C-Code

### Memory Bomb
- `bomb1.c` => single core memory bomb
- `bomb2.c` => two core memory bomb

### Usage
`test_inference` folder contains 100 images to test the response times:

```sh
python3 main_subgraphs_age.py --images_dir test_inference/ --model AgeGen/Age/Age.xmodel --membomb ~/test/membomb/{bomb1,bomb2}
```
>**Info** there will be a `csv` folder with csv files of response times. 
The program will create a csv file named **<MODEL_NAME>_<MEM_BOMB_TYPE>.csv**. 
If you run without memory bomb it will create a file named **<MODEL_NAME>.csv**

Command-line arguments:
- `-d IMAGES_DIR`, `--images_dir IMAGES_DIR`: Path to folder of images. Default is images.
- `-m MODEL`, `--model MODEL`: Path of xmodel. Default is CNN.xmodel
- `-b MEMBOMB`, `--membomb MEMBOMB`: Path to memory bomb C program. Default to False

### GCC
Compile the C program with pthread
```sh
gcc -pthread <FILE.C> -o <OUTPUT>
```
