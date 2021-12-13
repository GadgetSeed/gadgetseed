#!/bin/bash

patch -p1 -d ~/STM32Cube/Repository/STM32CubeH7 < patches/STM32CubeH7_QSPI_driver.patch
