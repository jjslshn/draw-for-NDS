# draw-for-NDS
这是一套来自gba中日词典的文字显示软件代码，通过 字模 画出中文文字。
我移植到nds上，把主要代码编写了，但是自己能力有限，源代码也是不能编译成功，自己移植后发现可能是变量问题，文字变量是const char*, 代码里处理的是const unsigned char*,我这个刚看c，这个就自己没能力处理。
如果输入强转化，这个转化后因为和字模对不上，虽然能显示字体，但找不对文字也没意义。

画中文字体到程序界面的代码，存在缺陷，因为不会改，所以请求帮助，看那位大神有时间，能帮忙处理下，非常感谢

println部分，变量格式不能输入 文字，