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
Build the whole code.
```
$ make
```
Run the hypervisor and OSes.
Sagevisor runs in HS-mode and and the two OSes run in VS and VU modes.
```
$ sh qemustart.sh
```

```
                                  +-----------------------------------------------+
                                  | dining philosophers program (runs in VU-mode) |
                                  +-----------------------------------------------+
+------------------------------+  +-----------------------------------------------+
| Nowhere OS (runs in VS-mode) |  |           Sophia OS (runs in VS-mode)         |
+------------------------------+  +-----------------------------------------------+
+---------------------------------------------------------------------------------+
|               Sagevisor hypervisor (runs in HS-mode)                            |
+---------------------------------------------------------------------------------+
+---------------------------------------------------------------------------------+
|                     OpenSBI (runs in M-mode)                                    |
+---------------------------------------------------------------------------------+
+---------------------------------------------------------------------------------+
|                     riscv64 (qemu)                                              |
+---------------------------------------------------------------------------------+
```
