# jos学习笔记

由于`操作系统导论`该课本主要介绍的操作系统的核心概念，缺少了很多实现代码，更不像CSAPP一样在课本中对lab代码提供详细的说明，本书和 `jos` 以及`xv6`之间并没有紧密的连接，所以写下自习笔记。





## Step 1 代码框架

```
workdir
|------ GNUmakefile
|
|------ boot
|         |---- Makefrag
|         |---- boot.S
|         |---- main.c
|------ fs
|
|------ inc
|
|------ kern
|         |---- Makefrag
|         |---- kernel.ld
|         |---- entry.S
|         |---- entrypgdir,c
|         |---- init.c
|         |---- monitor.c
|         |---- monitor.h
|         |---- kdebug.c
|         |---- kdebug.h
|         |---- printf.c
|         |---- console.c
|         |---- console.h
|
|------ user
|
|------ conf
|         |---- env.mk
|         |---- lab.mk

```



