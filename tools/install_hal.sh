#!/bin/bash

echo "The installation of the STM32Cube HAL driver will start."

halhome=$HOME
halbase="/STM32Cube/Repository"
halpath=$halhome$halbase

#echo $halhome
#echo $halbase
echo "Install path :" $halpath

mkdir -p $halpath
cd $halpath

#pwd
git clone https://github.com/STMicroelectronics/STM32CubeF7.git
git clone https://github.com/STMicroelectronics/STM32CubeF4.git
git clone https://github.com/STMicroelectronics/STM32CubeL1.git
