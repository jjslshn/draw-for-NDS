# draw-for-NDS
这是一套来自gba中日词典的文字显示软件代码，通过 字模 画出中文文字。我移植到nds上，把主要代码编写了，但是自己能力有限。

nds的c编译环境下，无法识别gbk字码，只能u8识别，如果使用gbk字库，需要u8转uncode，再转gbk

9.15日最新的更新是，使用点阵生成uncode字体文件，从新修改了一下代码部分，终于可以准确显示中文字体了。还存在其他的问题：print函数不正确，格式切换之后的缓存有异常。不过已经解决了一些问题。估计也没人玩这个，所以，还是自己慢慢搞了。
