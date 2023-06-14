# VCK190 ES1 MIPIRxQuad HDMI Tx with AI Engine

This folder contains the hardware design for the VCK190 ES1 MIPIRxQuad HDMI Tx design. The design is based on the VCK190 Base TRD design. The design is generated using the Vitis 2021.2 tool. 

Here the code to generate the hardware with AI Engine plus generate the petalinux image:

```sh
git clone --branch 2021.2 --recursive https://github.com/Xilinx/vck190-base-trd.git
cd vck190-base-trd
source <vitis-2021.2-path>/settings.sh
make overlay PFM=vck190_es1_mipiRxQuad_hdmiTx OVERLAY=xvdpu
source <petalinux-2021.2-path>/settings.sh
make sdcard PFM=vck190_es1_mipiRxQuad_hdmiTx OVERLAY=xvdpu YES=1
```

The `xvdpu` overlay is the one that contains the AI Engine design. The `YES=1` is to generate the petalinux image. The `PFM` is the platform name. The `OVERLAY` is the overlay name.

## RootFS

The rootfs is based on the [petalinux-v2021.2-final-vck190-es1-mipiRxQuad-hdmiTx-v1.0.0.bsp](https://www.xilinx.com/bin/public/openDownload?filename=petalinux-v2021.2-final-vck190-es1-mipiRxQuad-hdmiTx-v1.0.0.bsp) BSP with added packages.

## Base TRD Dashboard
The Base TRD dashboard is implemented using the python bokeh framework and visualizes the following useful platform statistics:

- CPU utilization
- Memory usage
- Temperature using the versal-sysmon device through the hwmon interface
- Power and voltage using the ina226 power monitors through the hwmon interface
- Memory throughput using the AXI performance monitors (APM) in the PL

The dashboard is intended to be run in parallel with any of the accelerated applications and is useful to monitor, evaluate and debug system performance while the target application is running live. A network connection is required to run the bokeh server.

To start the bokeh server run:

```sh
/etc/init.d/trd-dashboard-init restart
```

To connect to the bokeh server and to start the dashboard, copy the URL printed on the serial console and paste it into the Chrome browser address bar, for example:

- http://192.168.1.133:5006/trd-dashboard

## JupyterLab Server

To start Jupyter Notebook run:

```sh
/etc/init.d/jupyterlab-server stop
/etc/init.d/jupyterlab-server start
```

To connect to the JupyterLab server, copy the URL printed on the serial console and paste it into the Chrome browser address bar, for example:

- http://192.168.1.77:8888/?token=06cfb958c61eb0581bb759f40e3a4c3a6252cef3b7075449

## Documentation

The documentation for the VCK190 Base TRD design can be found here:

- https://xilinx.github.io/vck190-base-trd/2021.2/html/index.html