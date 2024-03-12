#!/bin/bash

cd ./memoria
make
cd ..

cd ./file_system
make
cd ..

cd ./cpu
make
cd ..

cd ./kernel
make
cd ..

cd ./consola
make
cd ..