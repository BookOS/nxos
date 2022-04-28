*“书是人类进步的阶梯。”*

<h1 align="center" style="margin: 30px 0 30px; font-weight: bold;">NXOS</h1>
<h4 align="center">下一代XBook操作系统内核</h4>
<p align="center">
	<a href="https://gitee.com/BookOS/nxos/stargazers"><img src="https://gitee.com/BookOS/nxos/badge/star.svg"></a>
	<a href="https://gitee.com/BookOS/nxos/members"><img src="https://gitee.com/BookOS/nxos/badge/fork.svg"></a>
    <a href="https://github.com/BookOS/nxos/stargazers"><img src="https://img.shields.io/github/stars/BookOS/nxos?style=flat-square&logo=GitHub"></a>
	<a href="https://github.com/BookOS/nxos/network/members"><img src="https://img.shields.io/github/forks/BookOS/nxos?style=flat-square&logo=GitHub"></a>
    <a href="https://github.com/BookOS/nxos/blob/master/LICENSE"><img src="https://img.shields.io/github/license/BookOS/nxos.svg?style=flat-square"></a>
</p>

## 简介

`NXOS`是`Next XBook Operating System`的意思，是一个跨平台的简洁、高性能、高稳定性的支持多核的操作系统内核，它将应用于桌面操作系统领域，服务器操作系统领域以及移动终端操作系统领域。

我们以简洁、高效、稳定为核心，用比较简洁且高效的方式去实现一些功能，去掉一些复杂，冗杂的功能，化繁为简。

我们的目标是针对不同的应用场景，可以做不同的裁剪，来实现性能最优化。
例如对于桌面操作系统，我们允许适当提高交互线程的优先级，运行时长等，来提示交互效果。
在服务器操作系统中，我们将做开启磁盘在内存中的缓存，使得再次加载程序时可以直接从内存中加载，
来减少网络服务程序的加载时间，提升服务器的性能。
在移动端操作系统中，我们将更多考虑到设备资源的使用优化，较少耗电，提高待机时长等。

## 平台支持

| ARCH    | PLATFORM   |STATUS      |
| ------- | ---------- | ---------- |
| x86     | i386       | DONE       |
| riscv64 | qemu       | DONE       |
| riscv64 | k210       | DONE       |
| riscv64 | d1         | DONE       |
| x86_64  | amd64      | TODO       |
| arm64   | qemu       | TODO       |
| arm32   | qemu       | TODO       |
| longarch| qemu       | TODO       |

## 文档中心

🏠 [文档中心](https://gitee.com/BookOS/nxos-documentation)  

## 联系我们  
🌍 [官网](https://www.book-os.org)  
📫 [邮箱](mailto:book-os@163.com)  

## 致谢

❤感谢每一位NXOS开发者的贡献，以及使用NXOS的你！❤

-- NXOS内核团队

