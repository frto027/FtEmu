# MyEmu
尝试自己编写一个FC模拟器

标准C语言，用了QT当IDE，接口方面`OpenGL`作图形显示，[`GLFW`](http://www.glfw.org/)处理窗口和用户输入，尽量跨平台，需要`pthread`多线程支持。

这个工程最近没有更新，实现细节还有些问题，ppu、输入输出相关的东西没有写完，里面的CPU已经可以正常跑通mario.nes程序了。

[NESDoc.pdf](http://nesdev.com/NESDoc.pdf)   
[6502Refrence](http://obelisk.me.uk/6502/reference.html)    
[6502 registers](http://obelisk.me.uk/6502/registers.html)  
[NesDev](http://wiki.nesdev.com)
