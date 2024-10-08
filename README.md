# Educational hypervisor and os for risc-v

There are 3 components here:
- Sophia OS is a tiny multi-core OS to run a program of the dining philosophers problem.
- Nowhere OS is a tiny single core OS.
- Sagevisor is a tiny hypervisor to run Sophia OS and Nowhere OS.

# Getting started
Install tools for riscv64 on Ubuntu 24.04LTS
```
$ sh setup.sh
```
Build the hypervisor and two OSes.
```
$ make
```
Run the hypervisor and two OSes.
```
$ sh qemustart.sh
```
