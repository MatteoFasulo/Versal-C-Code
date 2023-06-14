"""
Copyright 2019 Xilinx Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

from ctypes import *
from typing import List
import cv2
import numpy as np
import xir
import vart
import os
import math
from threading import Thread
import time
import sys
import csv
import subprocess
import argparse


def response_time_csv(data, filename: str = 'response_times.csv', header: list = ['img', 'time']):
    """
    It creates 'csv' folder with .csv file inside.
    Args:
        - filename: str => name of csv file
        - header: list => list of strings containing column names
        - data: list of lists => each list with img_name and time
    """
    if not os.path.isdir('csv'):
        os.mkdir('csv')

    if header is None:
        header = ['img', 'time']

    with open(f"csv{os.sep}{filename}", 'w', encoding='UTF8') as f:
        writer = csv.writer(f)
        writer.writerow(header)
        writer.writerows(data) # writerows expects a list of lists!

    return

class CustomThread(Thread):
    def __init__(self, group=None, target=None, name=None,
                 args=(), kwargs={}):
        Thread.__init__(self, group, target, name, args, kwargs)
    def run(self):
        if self._target != None:
            self._return = self._target(*self._args, **self._kwargs)
    def join(self, *args):
        Thread.join(self, *args)
        return self._return

"""
Calculate softmax
data: data to be calculated
size: data size
return: softamx result
"""


def CPUCalcSoftmax(data, size, scale):
    sum = 0.0
    result = [0 for i in range(size)]
    for i in range(size):
        result[i] = math.exp(data[i] * scale)
        sum += result[i]
    for i in range(size):
        result[i] /= sum
    return result


def get_script_directory():
    path = os.getcwd()
    return path


"""
Get topk results according to its probability
datain: data result of softmax
filePath: filePath in witch that records the infotmation of kinds
"""


def TopK(datain, size, filePath):

    cnt = [i for i in range(size)]
    pair = zip(datain, cnt)
    pair = sorted(pair, reverse=True)
    softmax_new, cnt_new = zip(*pair)
    fp = open(filePath, "r")
    data1 = fp.readlines()
    fp.close()
    for i in range(5):
        idx = 0
        for line in data1:
            if idx == cnt_new[i]:
                print("Top[%d] %d %s" % (i, idx, (line.strip)("\n")))
            idx = idx + 1


"""
pre-process for resnet50 (caffe)
"""
_B_MEAN = 103.53
_G_MEAN = 116.28
_R_MEAN = 123.675
MEANS = [_B_MEAN, _G_MEAN, _R_MEAN]
SCALES = [0.017429, 0.017507, 0.01712475]


def preprocess_one_image_fn(image_path, fix_scale, width=224, height=224):
    means = MEANS
    scales = SCALES
    image = cv2.imread(image_path)
    image = cv2.resize(image, (width, height))
    B, G, R = cv2.split(image)
    B = (B - means[0]) * scales[0] * fix_scale
    G = (G - means[1]) * scales[1] * fix_scale
    R = (R - means[2]) * scales[2] * fix_scale
    image = cv2.merge([B, G, R])
    image = image.astype(np.int8)
    return image


"""
run squeezenet with batch
runner: dpu runner
img: imagelist to be run
cnt: threadnum
"""


def runSqueezeNet(runner: "Runner", img, images_list):
    """get tensor"""
    inputTensors = runner.get_input_tensors()
    outputTensors = runner.get_output_tensors()

    input_ndim = tuple(inputTensors[0].dims)
    pre_output_size = int(outputTensors[0].get_data_size() / input_ndim[0])

    batchSize = input_ndim[0]

    output_ndim = tuple(outputTensors[0].dims)
    output_fixpos = outputTensors[0].get_attr("fix_point")
    output_scale = 1 / (2**output_fixpos)

    n_of_images = len(img)
    response_times = []
    count = 0
    image_index = 0
    while count < n_of_images:
        if (count+batchSize<=n_of_images):
            runSize = batchSize
        else:
            runSize = n_of_images-count
        """prepare batch input/output """
        inputData = [np.empty(input_ndim, dtype=np.int8, order="C")]
        outputData = [np.empty(output_ndim, dtype=np.int8, order="C")]
        """init input image to input buffer """
        for j in range(runSize):
            imageRun = inputData[0]
            imageRun[j, ...] = img[(count + j) % n_of_images].reshape(input_ndim[1:])
        """run with batch """
        start_time = time.time()
        job_id = runner.execute_async(inputData, outputData)
        runner.wait(job_id)
        end_time = time.time()
        inference_time = end_time - start_time
        response_times.append([images_list[image_index], inference_time])
        """softmax&TopK calculate with batch """
        """Benchmark DPU FPS performance over Vitis AI APIs execute_async() and wait() """
        """Uncomment the following code snippet to include softmax calculation for model’s end-to-end FPS evaluation """
        #for j in range(runSize):
        #    softmax = CPUCalcSoftmax(outputData[0][j], pre_output_size, output_scale)
        #    TopK(softmax, pre_output_size, "./words.txt")

        count = count + runSize

        image_index += 1

    return response_times

"""
 obtain dpu subgrah
"""


def get_child_subgraph_dpu(graph: "Graph") -> List["Subgraph"]:
    assert graph is not None, "'graph' should not be None."
    root_subgraph = graph.get_root_subgraph()
    assert (root_subgraph
            is not None), "Failed to get root subgraph of input Graph object."
    if root_subgraph.is_leaf:
        return []
    child_subgraphs = root_subgraph.toposort_child_subgraph()
    assert child_subgraphs is not None and len(child_subgraphs) > 0
    return [
        cs for cs in child_subgraphs
        if cs.has_attr("device") and cs.get_attr("device").upper() == "DPU"
    ]


def main():
    global threadnum

    ap = argparse.ArgumentParser()
    ap.add_argument('-d', '--images_dir', type=str, default='./images/', help='Path to folder of images. Default is images')
    ap.add_argument('-m', '--model',      type=str, default='./ResNet50/resnet50.xmodel', help='Path of xmodel. Default is CNN.xmodel')
    ap.add_argument('-b', '--membomb', type=str, default=False, required=False, help='Path to memory bomb C program. Default to False(?)')
    ap.add_argument('-t', '--thread', type=int, default=1, required=False, help='Thread number. Default to 1')

    args = ap.parse_args()
    print("\n")
    print ('Command line options:')
    print (' --images_dir : ', args.images_dir)
    print (' --model      : ', args.model)

    if args.thread:
        print (' --thread : ', args.thread)
        threadnum = int(args.thread)

    if args.membomb:
        print (' --membomb : ', args.membomb)
        proc = subprocess.Popen([args.membomb], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print (f'\nExecuting membomb...\nPID: {proc.pid}')

    print("\n")


    listimage = os.listdir(args.images_dir)
    threadAll = []
    i = 0
    global runTotall
    runTotall = len(listimage)
    g = xir.Graph.deserialize(args.model)
    subgraphs = get_child_subgraph_dpu(g)
    assert len(subgraphs) == 1  # only one DPU kernel
    all_dpu_runners = []
    for i in range(int(threadnum)):
        all_dpu_runners.append(vart.Runner.create_runner(subgraphs[0], "run"))

    input_fixpos = all_dpu_runners[0].get_input_tensors()[0].get_attr("fix_point")
    input_scale = 2**input_fixpos
    """image list to be run """
    img = []
    for i in range(runTotall):
        path = os.path.join(args.images_dir, listimage[i])
        img.append(preprocess_one_image_fn(path, input_scale))
    """
      The cnt variable is used to control the number of times a single-thread DPU runs.
      Users can modify the value according to actual needs. It is not recommended to use
      too small number when there are few input images, for example:
      1. If users can only provide very few images, e.g. only 1 image, they should set
         a relatively large number such as 360 to measure the average performance;
      2. If users provide a huge dataset, e.g. 50000 images in the directory, they can
         use the variable to control the test time, and no need to run the whole dataset.
    """
    """run with batch """
    time_start = time.time()
    for i in range(int(threadnum)):
        t1 = CustomThread(target=runSqueezeNet, args=(all_dpu_runners[i], img, listimage))
        threadAll.append(t1)
    for x in threadAll:
        x.start()
    for x in threadAll:
        response_times = x.join()

    del all_dpu_runners

    csv_filename = f"{args.model.split('/')[-1]}"
    csv_header = None

    if args.membomb:
        pid = proc.pid
        command = f"kill -0 {pid} 2>/dev/null && echo 'True' || echo 'False'"
        output = subprocess.check_output(command, shell=True, text=True)
        if output.strip() == 'True':
            print("\nProcess with PID", pid, "is running.\n")
        else:
            print("\nProcess with PID", pid, "is not running.\n")

        csv_filename += f"_{args.membomb.split('/')[-1]}"
        csv_header = ["img", f"time_{args.membomb.split('/')[-1]}"]

    csv_filename += ".csv"

    response_time_csv(filename=csv_filename, header=csv_header, data=response_times)

    time_end = time.time()
    timetotal = time_end - time_start
    total_frames = len(listimage)
    fps = float(total_frames / timetotal)
    print(
        "FPS=%.2f, total frames = %.2f , time=%.6f seconds"
        % (fps, total_frames, timetotal)
    )


if __name__ == "__main__":
    main()
