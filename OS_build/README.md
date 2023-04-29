Here the code to generate the hardware with AI Engine plus generate the petalinux image:

```
git clone --recursive https://github.com/Xilinx/vck190-base-trd.git
cd vck190-base-trd
source <vitis-2021.2-path>/settings.sh
make overlay PFM=vck190_es1_mipiRxQuad_hdmiTx OVERLAY=xvdpu
source <petalinux-2021.2-path>/settings.sh
make sdcard PFM=vck190_es1_mipiRxQuad_hdmiTx OVERLAY=xvdpu YES=1
```
