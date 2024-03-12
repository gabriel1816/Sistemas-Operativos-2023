#!/bin/bash

validate_ip() {
  
  local ip_pattern='^([0-9]{1,3}\.){3}[0-9]{1,3}$'

  if [[ $1 =~ $ip_pattern ]]; then
    return 0  
  else
    return 1  
  fi
}



read -p "Ingresa la IP de Kernel: " ipKernel
read -p "Ingresa la IP de Memoria: " ipMemoria
read -p "Ingresa la IP de FileSystem: " ipFS
read -p "Ingresa la IP de CPU: " ipCPU

if [[ $ipKernel == "" || $ipMemoria == "" || $ipFS == "" || $ipCPU == "" ]]; then
  echo "Error: No ingresaste nada :0"
  exit 0
fi

if validate_ip "$ipKernel" && validate_ip "$ipMemoria" && validate_ip "$ipFS" && validate_ip "$ipCPU" ; then
  echo "Todas las IPs ingresadas son validas para actualizar los configs"
else
  echo "Error: Pusiste cualquier cosa :/"
  exit 0
fi

# Regexs
sKer="IP_KERNEL=[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}"
sMem="IP_MEMORIA=[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}"
sFs="IP_FILESYSTEM=[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}"
sCpu="IP_CPU=[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}"

# Kernel
kc0="./kernel/cfg/kernel.config"
kc1="./pruebas/fifo/kernel.config"
kc2="./pruebas/hrrn/kernel.config"
kc3="./pruebas/deadlock/kernel.config"
kc4="./pruebas/fileSystem/kernel.config"
kc5="./pruebas/memoria/kernel.config"
kc6="./pruebas/error/kernel.config"

sed -E -i "s/$sKer/IP_KERNEL=$ipKernel/" $kc0
sed -E -i "s/$sKer/IP_KERNEL=$ipKernel/" $kc1
sed -E -i "s/$sKer/IP_KERNEL=$ipKernel/" $kc2
sed -E -i "s/$sKer/IP_KERNEL=$ipKernel/" $kc3
sed -E -i "s/$sKer/IP_KERNEL=$ipKernel/" $kc4
sed -E -i "s/$sKer/IP_KERNEL=$ipKernel/" $kc5
sed -E -i "s/$sKer/IP_KERNEL=$ipKernel/" $kc6

sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $kc0
sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $kc1
sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $kc2
sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $kc3
sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $kc4
sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $kc5
sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $kc6

sed -E -i "s/$sCpu/IP_CPU=$ipCPU/" $kc0
sed -E -i "s/$sCpu/IP_CPU=$ipCPU/" $kc1
sed -E -i "s/$sCpu/IP_CPU=$ipCPU/" $kc2
sed -E -i "s/$sCpu/IP_CPU=$ipCPU/" $kc3
sed -E -i "s/$sCpu/IP_CPU=$ipCPU/" $kc4
sed -E -i "s/$sCpu/IP_CPU=$ipCPU/" $kc5
sed -E -i "s/$sCpu/IP_CPU=$ipCPU/" $kc6

sed -E -i "s/$sFs/IP_FILESYSTEM=$ipFS/" $kc0
sed -E -i "s/$sFs/IP_FILESYSTEM=$ipFS/" $kc1
sed -E -i "s/$sFs/IP_FILESYSTEM=$ipFS/" $kc2
sed -E -i "s/$sFs/IP_FILESYSTEM=$ipFS/" $kc3
sed -E -i "s/$sFs/IP_FILESYSTEM=$ipFS/" $kc4
sed -E -i "s/$sFs/IP_FILESYSTEM=$ipFS/" $kc5
sed -E -i "s/$sFs/IP_FILESYSTEM=$ipFS/" $kc6

# Consola
cc0="./consola/cfg/consola.config"

sed -E -i "s/$sKer/IP_KERNEL=$ipKernel/" $cc0

# FS
fsc0="./file_system/cfg/file_system.config"

sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $fsc0
sed -E -i "s/$sFs/IP_FILESYSTEM=$ipFS/" $fsc0

# CPU
cpc0="./cpu/cfg/cpu.config"
cpc1="./pruebas/fifo/cpu.config"
cpc2="./pruebas/hrrn/cpu.config"
cpc3="./pruebas/deadlock/cpu.config"
cpc4="./pruebas/error/cpu.config"
cpc5="./pruebas/fileSystem/cpu.config"
cpc6="./pruebas/memoria/cpu.config"

sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $cpc0
sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $cpc1
sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $cpc2
sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $cpc3
sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $cpc4
sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $cpc5
sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $cpc6

sed -E -i "s/$sCpu/IP_CPU=$ipCPU/" $cpc0
sed -E -i "s/$sCpu/IP_CPU=$ipCPU/" $cpc1
sed -E -i "s/$sCpu/IP_CPU=$ipCPU/" $cpc2
sed -E -i "s/$sCpu/IP_CPU=$ipCPU/" $cpc3
sed -E -i "s/$sCpu/IP_CPU=$ipCPU/" $cpc4
sed -E -i "s/$sCpu/IP_CPU=$ipCPU/" $cpc5
sed -E -i "s/$sCpu/IP_CPU=$ipCPU/" $cpc6


# Memoria
mc0="./memoria/cfg/memoria.config"
mc1="./pruebas/fifo/memoria.config"
mc2="./pruebas/hrrn/memoria.config"
mc3="./pruebas/deadlock/memoria.config"
mc4="./pruebas/fileSystem/memoria.config"
mc5="./pruebas/memoria/memoria1.config"
mc6="./pruebas/memoria/memoria2.config"
mc7="./pruebas/memoria/memoria3.config"
mc8="./pruebas/error/memoria.config"

sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $mc0
sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $mc1
sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $mc2
sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $mc3
sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $mc4
sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $mc5
sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $mc6
sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $mc7
sed -E -i "s/$sMem/IP_MEMORIA=$ipMemoria/" $mc8