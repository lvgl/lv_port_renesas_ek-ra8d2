# LVGL ported to Renesas EK-RA8D2

## Overview

The Renesas EK-RA8D2 is an evaluation board for a high-end 1GHz ARM Cortex-M85 microcontroller
with large external memory (128MB).
It has a D/AVE 2D graphics accelerator which LVGL supports and is enabled in this project.
The included display is 7" and has a resolution of 1024x600.

## Buy

You can purchase a Renesas EK-RA8D2 from https://www.renesas.com/en/design-resources/boards-kits/ek-ra8d2

## Benchmark

<!-- <a href="https://www.youtube.com/watch?v=XXXXXXXXXXXXXXXXXXXXXXX">
    <img src="https://github.com/user-attachments/assets/87c1f2e5-0260-4772-b711-13fdab467474" width="75%">
</a> -->

Here is the benchmark result for D/AVE 2D with double-buffered direct GLCDC.

| Name                      | Avg. CPU | Avg. FPS | Avg. time | render time | flush time |
| :------------------------ | -------: | -------: | --------: | ----------: | ---------: |
| Empty screen              | 17%      | 29       | 21        | 14          | 7          |
| Moving wallpaper          | 24%      | 25       | 37        | 33          | 4          |
| Single rectangle          | 4%       | 29       | 8         | 1           | 7          |
| Multiple rectangles       | 11%      | 30       | 14        | 7           | 7          |
| Multiple RGB images       | 23%      | 29       | 15        | 7           | 8          |
| Multiple ARGB images      | 23%      | 30       | 16        | 9           | 7          |
| Rotated ARGB images       | 10%      | 30       | 13        | 5           | 8          |
| Multiple labels           | 72%      | 21       | 40        | 32          | 8          |
| Screen sized text         | 74%      | 8        | 117       | 111         | 6          |
| Multiple arcs             | 34%      | 29       | 28        | 17          | 11         |
| Containers                | 49%      | 29       | 20        | 11          | 9          |
| Containers with overlay   | 50%      | 14       | 54        | 45          | 9          |
| Containers with opa       | 52%      | 28       | 22        | 14          | 8          |
| Containers with opa_layer | 87%      | 9        | 95        | 84          | 11         |
| Containers with scrolling | 69%      | 14       | 66        | 57          | 9          |
| Widgets demo              | 66%      | 14       | 52        | 44          | 8          |
| All scenes avg.           | 41%      | 23       | 37        | 30          | 7          |

## Specification

### CPU and Memory
- **MCU:** 1GHz ARM Cortex-M85 and 250MHz ARM Cortex-M33
- **RAM:** 1872KB internal, 128MB external SDRAM
- **Flash:** 1MB
- **GPU:** D/AVE 2D

### Display and Touch
- **Resolution:** 1024x600
- **Display Size:** 7"
- **Interface:** Parallel RGB
- **Color Depth:** 16-bit
- **Technology:** LCD
- **DPI:** 170 px/inch
- **Touch Pad:** Capacitive

### Connectivity
- 2x USB host or device
- 2x display connectors (connects to included display board)
- Ecosystem connectors (Arduino, mikroBUS, and Pmod)

## Getting started

### Hardware setup
- Mount the graphics expansion board to the mainboard.
- Connect a USB C cable to DEBUG1 (J10) and your PC.

### Software setup
- [Install e2 studio 2025-10.0 or newer (important) for your OS](https://www.renesas.com/en/software-tool/e2studio-information-rz-family).
  On Linux, Ubuntu 24.04 or newer is required.
  - When prompted, choose "Custom Install".
  - Ensure "RA" is included in your selection of "Device Families" to install.
  - Ensure "Renesas FSP Smart Configurator Core" and "Renesas FSP Smart Configurator ARM"
    are included in your selection of "Customize Features".
  - Ensure "LLVM Embedded Toolchain for Arm 18.1.3" is selected.
- Install FSP Packs. v6.2.0 is required.
  - On Windows, simply download FSP_Packs_v6.2.0.exe
    [from here](https://github.com/renesas/fsp/releases/tag/v6.2.0) under "Assets".
    Run the installer and follow the prompts.
  - On Linux, download FSP_Packs_v6.2.0.zip
    [from here](https://github.com/renesas/fsp/releases/tag/v6.2.0) under "Assets".
    Locate the e2 studio install location. If it is `~/.local/share/renesas/e2_studio`
    and the ZIP download is `~/Downloads/FSP_Packs_v6.2.0.zip`, unzip the file
    with the following commands:
    ```shell
    cd ~/.local/share/renesas/e2_studio
    unzip ~/Downloads/FSP_Packs_v6.2.0.zip
    ```
    The directory structure in the ZIP overlaps with the
    e2_studio install location. This is expected. The `unzip` process will
    update the directory structure with the new FSP files from the ZIP and preserve any existing FSP packs.


### Run the project
- Clone the repository
  ```shell
  git clone https://github.com/lvgl/lv_port_renesas_ek-ra8d2.git
  ```
- Open e2 studio and go to **File > Open Projects from File System...**. Click "Directory"
  and navigate to the cloned project and then click "Finish".
- Click the hammer to build the project. If it is greyed-out, first single-click the project
  in the left sidebar and the hammer should become clickable.
- To upload and run the project, click the bug (debug) icon. The debugger will break (stop execution)
  at the beginning. Click the "Resume" button to continue execution.
- To upload the optimized Release version of the build, open the dropdown next to the bug
  icon and click "Debug Configurations...". Change the "C/C++ Application:" to start with
  "Release/" instead of "Debug/". It's also recommended to set the "Build Configuration:"
  to "Release". Remember to change these settings back if you want a good experience debugging
  the app.

### Debugging
- In the previous section, the project was run using the debugger.
  Simply continue using the interactive debugger in e2 studio to debug your program.
  Set breakpoints, continue, step, etc. as with any other Eclipse-based IDE.

## Notes

e2 studio has a configurator to adjust configuration settings. If you want to change an
LVGL config, first check if it's an option present in the FSP Configuratior. To get there,
open the "FSP Configuration" perspective by clicking the tab in the very top-right corner
of the window. Next, open "configuration.xml" by double-clicking it in the "Project Explorer".
When you are done, make sure to click "Generate Project Content".
For all other configs, `src/lv_conf_user.h` is the file to edit. `ra_cfg\fsp_cfg\lvgl\lvgl\lv_conf.h` includes this file
and has some defaults which will be set if they are not explicitly set in `src/lv_conf_user.h`.
To see all the possible LVGL configs, look at `./ra/lvgl/lvgl/lv_conf_template.h`.

LVGL comes from the FSP so the source code will appear upon building. It can be edited
in-place, but changes will be lost if a "clean" is performed.

LVGL logging and `printf` goes to a virtual serial port with baud 115200 over the USB debug connection.
You can open any serial terminal on your PC to view it.

At the time of writing, `LV_MEM_SIZE` (set in the configurator) is set to 1 MB (0x100000) so that the largest layer can be allocated
for the benchmark's "Containers with opa_layer" scene. You can easily reduce the LVGL memory size to
much less, e.g. 128 kB, for typical usage. Enable `LV_USE_MEM_MONITOR` to see the memory usage in the bottom-left
corner of the display.

## Contribution and Support

If you find any issues with the development board feel free to open an Issue in this repository. For LVGL related issues (features, bugs, etc) please use the main [lvgl repository](https://github.com/lvgl/lvgl).

If you found a bug and found a solution too please send a Pull request. If you are new to Pull requests refer to [Our Guide](https://docs.lvgl.io/master/CONTRIBUTING.html#pull-request) to learn the basics.

