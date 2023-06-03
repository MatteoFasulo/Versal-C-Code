# Versal-C-Code

### Memory Bomb
- `bomb1.c` => single core memory bomb
- `bomb2.c` => two core memory bomb

### Usage
```sh
python3 main_subgraphs_age.py --images_dir unico/ --model AgeGen/Age/Age.xmodel --membomb ~/test/membomb/<C_PROGRAM>
```
Command-line arguments:
- `-d IMAGES_DIR`, `--images_dir IMAGES_DIR`: Path to folder of images. Default is images.
- `-m MODEL`, `--model MODEL`: Path of xmodel. Default is CNN.xmodel
- `-b MEMBOMB`, `--membomb MEMBOMB`: Path to memory bomb C program. Default to False

### GCC
Compile the C program with pthread
```sh
gcc -pthread <FILE.C> -o <OUTPUT>
```
