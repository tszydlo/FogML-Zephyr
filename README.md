# FogML-Zephyr
The application recognizes gestures an IoT device performs based on the built-in accelerometer. It uses the classification algorithms and programming tools provided by FogML:
* FogML-SDK [https://github.com/tszydlo/fogml_sdk]
* FogML tools [https://github.com/tszydlo/FogML]

The application is implemented for the Zephyr-OS operating system. The tests were carried out on a Thingy:91 device containing nrf9160 and manufactured by Nordic Semiconductor.

## Classification
The accelerometer data processing pipeline is presented in the figure.

![Classification pipeline](./doc/classification.png)

In the example, gestures are classified using the random forest algorithm. The generated classifier source code is in the `src/fogml_generated/random_forest_model.c` file.

For the device to recognize other gestures, it is necessary to collect training data, assign labels, and train the model. The entire process is described in Jupyter Notebook, along with training data in the `tools` directory. Notepad can be run in eg Google Colab or Anaconda.

## Building
The application is implemented for the Zephyr OS operating system. Since the application is for Thingy:91 device, we use NRF Connect SDK 2.0.0 and VSCode plugin, which simplifies the process of building, flashing and debugging applications for Zephyr. The application can also be built directly using the `west` tool and the base Zephyr.

The application uses the basic drivers available in the operating system, and porting to another device should be relatively easy.

## Bibliography
Please mention us and cite our papers if you use this work and find it useful.

```
@inproceedings{FogMLSzydlo2018,
  author    = {Tomasz Szydlo and
               Joanna Sendorek and
               Robert Brzoza{-}Woch},
  editor    = {Yong Shi and
               Haohuan Fu and
               Yingjie Tian and
               Valeria V. Krzhizhanovskaya and
               Michael Harold Lees and
               Jack J. Dongarra and
               Peter M. A. Sloot},
  title     = {Enabling Machine Learning on Resource Constrained Devices by Source
               Code Generation of the Learned Models},
  booktitle = {Computational Science - {ICCS} 2018 - 18th International Conference,
               Wuxi, China, June 11-13, 2018, Proceedings, Part {II}},
  series    = {Lecture Notes in Computer Science},
  volume    = {10861},
  pages     = {682--694},
  publisher = {Springer},
  year      = {2018},
}
```
