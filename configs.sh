#!/bin/bash

echo "Seleccione el tipo de prueba a realizar:"
echo "1. Fifo"
echo "2. HRRN"
echo "3. Deadlock"
echo "4. Memoria FIRST"
echo "5. Memoria BEST"
echo "6. Memoria WORST"
echo "7. FileSystem"
echo "8. Errores"

read -p "Ingrese el numero de la prueba: " prueba

case $prueba in
    1)
        echo "Prueba Fifo"
        cp "./pruebas/fifo/kernel.config" "./kernel/cfg/kernel.config"
        cp "./pruebas/fifo/memoria.config" "./memoria/cfg/memoria.config"
        cp "./pruebas/fifo/cpu.config" "./cpu/cfg/cpu.config"
        ;;
    2)
        echo "Prueba HRRN"
        cp "./pruebas/hrrn/kernel.config" "./kernel/cfg/kernel.config"
        cp "./pruebas/hrrn/memoria.config" "./memoria/cfg/memoria.config"
        cp "./pruebas/hrrn/cpu.config" "./cpu/cfg/cpu.config"
        ;;    
    3)
        echo "Prueba Deadlock"
        cp "./pruebas/deadlock/kernel.config" "./kernel/cfg/kernel.config"
        cp "./pruebas/deadlock/memoria.config" "./memoria/cfg/memoria.config"
        cp "./pruebas/deadlock/cpu.config" "./cpu/cfg/cpu.config"
        ;;
    4)
        echo "Prueba Memoria FIRST"
        cp "./pruebas/memoria/kernel.config" "./kernel/cfg/kernel.config"
        cp "./pruebas/memoria/memoria1.config" "./memoria/cfg/memoria.config"
        cp "./pruebas/memoria/cpu.config" "./cpu/cfg/cpu.config"
        ;;
    5)
        echo "Prueba Memoria BEST"
        cp "./pruebas/memoria/kernel.config" "./kernel/cfg/kernel.config"
        cp "./pruebas/memoria/memoria2.config" "./memoria/cfg/memoria.config"
        cp "./pruebas/memoria/cpu.config" "./cpu/cfg/cpu.config"
        ;;
    6)
        echo "Prueba Memoria WORST"
        cp "./pruebas/memoria/kernel.config" "./kernel/cfg/kernel.config"
        cp "./pruebas/memoria/memoria3.config" "./memoria/cfg/memoria.config"
        cp "./pruebas/memoria/cpu.config" "./cpu/cfg/cpu.config"
        ;;
    7)
        echo "Prueba File System"
        cp "./pruebas/fileSystem/kernel.config" "./kernel/cfg/kernel.config"
        cp "./pruebas/fileSystem/memoria.config" "./memoria/cfg/memoria.config"
        cp "./pruebas/fileSystem/cpu.config" "./cpu/cfg/cpu.config"
        ;;
    8)
        echo "Prueba Errores"
        cp "./pruebas/error/kernel.config" "./kernel/cfg/kernel.config"
        cp "./pruebas/error/memoria.config" "./memoria/cfg/memoria.config"
        cp "./pruebas/error/cpu.config" "./cpu/cfg/cpu.config"
        ;;
    *)
        echo "Opcion incorrecta"
        ;;
esac

exit 0