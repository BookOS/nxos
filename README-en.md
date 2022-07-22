
*“Books are the ladder of human progress.”*

<h1 align="center" style="margin: 30px 0 30px; font-weight: bold;">NXOS</h1>

<h4 align="center">Next generation XBook OS kernel</h4>

<p align="center">
<a href="https://gitee.com/BookOS/nxos/stargazers"><img src="https://gitee.com/BookOS/nxos/badge/star.svg"></a>
<a href="https://gitee.com/BookOS/nxos/members"><img src="https://gitee.com/BookOS/nxos/badge/fork.svg"></a>
<a href="https://github.com/BookOS/nxos/stargazers"><img src="https://img.shields.io/github/stars/BookOS/nxos?style=flat-square&logo=GitHub"></a>
<a href="https://github.com/BookOS/nxos/network/members"><img src="https://img.shields.io/github/forks/BookOS/nxos?style=flat-square&logo=GitHub"></a>
<a href="https://github.com/BookOS/nxos/blob/master/LICENSE"><img src="https://img.shields.io/github/license/BookOS/nxos.svg?style=flat-square"></a>
</p>

## Brief introduction

`NXOS`(that is`Next XBook Operating System`), It is a cross platform, concise, high-performance, high stability operating system kernel supporting multi-core. It will be applied to desktop operating system, server operating system, mobile terminal operating system and other fields.

We take simplicity, efficiency and stability as the core, and use a relatively simple and efficient way to realize some functions, remove some complex and miscellaneous functions, and turn complexity into simplicity.

Our goal is to make different tailoring for different application scenarios to achieve performance optimization.

For example, for the desktop operating system, we allow to appropriately increase the priority and running time of interactive threads to improve the interaction effect.

In the server operating system, we will start the disk cache in memory, so that the program can be loaded directly from memory when it is loaded again,

In order to reduce the loading time of the network service program and improve the performance of the server.

In the mobile operating system, we will give more consideration to optimizing the use of equipment resources, reducing power consumption and improving standby time.

## Platform support

| ARCH    | PLATFORM   |STATUS      |
| ------- | ---------- | ---------- |
| x86     | i386       | DONE       |
| riscv64 | qemu       | DONE       |
| riscv64 | k210       | DONE       |
| riscv64 | d1         | DONE       |
| riscv64 | hifive unmached| DONE       |
| x86-64  | amd64      | TODO       |
| arm64   | qemu       | TODO       |
| arm32   | qemu       | TODO       |
|loongArch| qemu       | TODO       |

## Document Center

🏠 [Document Center](https://gitee.com/BookOS/nxos-documentation)  

## Contact us

🌍 [Website](https://www.book-os.org)  
📫 [E-mail](mailto:book-os@163.com)  

## Thank

❤Thank every NXOS developer for their contributions and you who use NXOS!❤

                        -- NXOS kernel team
