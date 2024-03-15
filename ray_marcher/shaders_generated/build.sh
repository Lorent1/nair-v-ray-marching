#!/bin/sh
glslangValidator -V kernel2D_RayMarchAAX1.comp -o kernel2D_RayMarchAAX1.comp.spv -DGLSL -I.. -I/home/petr/dev/nair-v-ray-marching/external/json/include -I/home/petr/kernel_slicer/TINYSTL -I/home/petr/dev/nair-v-ray-marching/external/LiteMath 
glslangValidator -V kernel2D_RayMarchAAX4.comp -o kernel2D_RayMarchAAX4.comp.spv -DGLSL -I.. -I/home/petr/dev/nair-v-ray-marching/external/json/include -I/home/petr/kernel_slicer/TINYSTL -I/home/petr/dev/nair-v-ray-marching/external/LiteMath 
