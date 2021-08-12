#!/bin/bash

echo "Update STM32Cube HAL driver will start."

halhome=$HOME
halbase="/STM32Cube/Repository"
halpath=$halhome$halbase

#echo $halhome
#echo $halbase
echo "Install path :" $halpath

mkdir -p $halpath
cd $halpath

#pwd
git -C STM32CubeF7 pull
git -C STM32CubeF4 pull
git -C STM32CubeL1 pull
git -C STM32CubeH7 pull
