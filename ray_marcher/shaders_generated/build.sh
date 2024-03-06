#!/bin/sh
glslangValidator -V kernel2D_RayMarch.comp -o kernel2D_RayMarch.comp.spv -DGLSL -I.. -I/home/petr/kernel_slicer/TINYSTL -I/home/petr/dev/nair-v-ray-marching/external/LiteMath 
