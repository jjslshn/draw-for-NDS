#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <cstdarg>

#define ASCII_BEGIN	0x21
#define ASCII_END	0x80
#define HZ_PAGE_BEGIN	0x81
#define HZ_PAGE_END	0xfe
#define HZ_STARTED_CODE	0x40
#define HZ_ENDED_CODE	0xfe
#define HEAD_SIZE	0xa

//**********元常量************/
#define ASCII_FONT_WIDTH             6           //单字节码字模宽
#define ASCII_FONT_HEIGHT            12          //单字节码字模高
#define ASCII_FONT_BYTES             9           //一个单字节码字模占用的空间
#define HZ_FONT_WIDTH                11          //双字节码字模宽
#define HZ_FONT_HEIGHT               11          //双字节码字模高
#define HZ_FONT_BYTES                16          //一个双字节码字模占用的空间

//**********ascii单字节码相关************/
#define ASCII_COUNTS                  (ASCII_END - ASCII_BEGIN + 1)     //字库中单字节码个数
#define ASCII_SIZE                    (ASCII_COUNTS * ASCII_FONT_BYTES) //字库中单字节码占用总空间
#define ASCII_BASE                    HEAD_SIZE                       //计算单字节码字模在字库中位置时用到的偏移常量

//**********GBK双字节码相关**************/
#define HZ_BASE                       (ASCII_BASE + ASCII_SIZE)            //计算双字节码字模在字库中的位置时用到的偏移常量
#define HZ_PAGE_COUNTS                (HZ_PAGE_END - HZ_PAGE_BEGIN + 1)    //字库一共支持多少个GBK页面的汉字字体
#define HZ_COUNTS_PERPAGE             (HZ_ENDED_CODE - HZ_STARTED_CODE +1) //字库中的一页有多少个汉字被支持
#define HZ_COUNTS                     (HZ_PAGE_COUNTS * HZ_COUNTS_PERPAGE) //字库总共支持多少汉字
#define HZ_PAGE_SIZE                  (HZ_FONT_BYTES * HZ_COUNTS_PERPAGE)  //一页汉字占用的总空间
#define HZ_SIZE                       (HZ_FONT_BYTES * HZ_COUNTS)          //所有汉字占用的总空间

#define MASK_INIT   0x01
#define MASK_REINIT 0x80


u16 pen_color = RGB15(31,0,0);  // 画笔颜色
extern const char font_info[];
int pxa,pya,pxg,pyg;//位置函数


bool IS_ASCII_CODE(const char *byte_ptr){//判断一个位置的字符是否是单字节码
	if(( *(byte_ptr) >= ASCII_BEGIN ) && ( *(byte_ptr) <= ASCII_END )){  
		return true;
	}else{	
		return false;
	}
}


u16 ASCII_FONT_LOCATE(const char *byte_ptr){ //找到单字节码在字库中的字模的首位
	return (ASCII_BASE + ASCII_FONT_BYTES * ( *(byte_ptr) - ASCII_BEGIN )); 
}


bool IS_HZ_CODE(const char *byte_ptr){//判定给定位置上的字符是否是GBK码
	if(( *(byte_ptr) >= HZ_PAGE_BEGIN ) && ( *(byte_ptr) <= HZ_PAGE_END ) && ( *( (byte_ptr)+1 ) >= HZ_STARTED_CODE ) && ( *( (byte_ptr)+1 ) <= HZ_ENDED_CODE)){
		return true;	
	}else{	
		return false;
	}
}


u16 HZ_FONT_LOCATE(const char *byte_ptr){//找到汉字在字库中的首位置
	return (HZ_BASE + HZ_FONT_BYTES * ( *( (byte_ptr)+1 ) - HZ_STARTED_CODE  + ( *(byte_ptr) - HZ_PAGE_BEGIN ) * HZ_COUNTS_PERPAGE ));
}


void drawAscii(int x, int y, const char *code){////point lt, byte *code){
	const char *pixel;
 	u16 mask, i, j;  // 循环体中的计数
	
	int rbx = x + ASCII_FONT_WIDTH; //rb
	int rby = y + ASCII_FONT_HEIGHT;
	u16 *current;  // 显存位置
	if(x <= SCREEN_WIDTH && y <= SCREEN_HEIGHT && rbx <= SCREEN_WIDTH && rby <= SCREEN_HEIGHT){ // 可以描出字符
		current = VRAM_A + y*SCREEN_WIDTH + x;             // 找到左上角对应的顶点内存
		pixel = font_info + ASCII_FONT_LOCATE(code);     // 找到单字符对应的字模内存
		mask = 1;//MASK_INIT;

		for(i=1; i <= ASCII_FONT_HEIGHT; ++i){
			for(j=1; j <= ASCII_FONT_WIDTH; ++j){
				if(*pixel & mask){                           // 此点需要画
					*current = pen_color;
				}//可能范围不对
				++current;
				if(mask == MASK_REINIT){
					mask = MASK_INIT;
					++pixel;
				} //用完了一个BYTE的字模数据要往把指针下移
				else mask <<= 1;
			}
			current += SCREEN_WIDTH - ASCII_FONT_WIDTH;
		}
		pxa=rbx;
		pya=y;// 下一个字符的左上角位置///////////////////////////////////////////
	}
	else{
		pxa=x;
		pya=y;
	}
}


void drawGbk(int x, int y, const char *code){
	const char *pixel;
	u16 i, j, mask;

	int rbx = x + HZ_FONT_WIDTH; 
	int rby = y + HZ_FONT_HEIGHT;
	u16 *current;  // 显存位置
	if(x <= SCREEN_WIDTH && y <= SCREEN_HEIGHT && rbx <= SCREEN_WIDTH && rby <= SCREEN_HEIGHT){
		current = VRAM_A + y*SCREEN_WIDTH + x;
		pixel = font_info + HZ_FONT_LOCATE(code);
		mask = MASK_INIT;
		for(i=1; i <= HZ_FONT_HEIGHT; ++i){
			for(j=1;j <= HZ_FONT_WIDTH; ++j){
				if(*pixel & mask){
					*current = pen_color;
				}//可能范围不对
				++current;
				if(mask == MASK_REINIT) {
					mask = MASK_INIT;
					++pixel;
				}
		    	else mask <<= 1;
			}
			current += SCREEN_WIDTH - HZ_FONT_WIDTH;
		}
		pxg=rbx+1;
		pyg=y;
	}
	else{
		pxg=x;
		pyg=y;
	}
}


int println(int x, int y, const char *str){//point lt,
	const char *org_str = str;       // 保存起始位置，返回值用到
	int next_ltx,next_lty; //next_lt            // 保存下一个字符的左上角位置
	for(;*str!='\n' && *str != '\0';++str){
		if( IS_HZ_CODE(str) ){   // 汉字就调用drawGbk
			drawGbk(x, y, str);
			next_ltx=pxg;
			next_lty=pyg;

			if(next_ltx == x) break; //此行已无法往后写，不写了该返回了
				x = next_ltx;
				y = next_lty;
				++str;
		}else if( IS_ASCII_CODE(str) ){
			drawAscii(x, y, str);
			next_ltx=pxa;
			next_lty=pya;

			if(next_ltx == x) break;
			x = next_ltx;
			y = next_lty;
		}
	}
	if(*str == '\n') ++str;
	return str - org_str;
}


PrintConsole topScreen;//文字需要
PrintConsole bottomScreen;
//主程序/////////////////////////////////////////////////
int main() {
	
	
	/*//打印字模查找部分//////////////工作正常
	videoSetMode(MODE_0_2D);
	videoSetModeSub(MODE_0_2D);
	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);
	consoleInit(&topScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
	consoleInit(&bottomScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);
	consoleSelect(&bottomScreen);
	swiWaitForVBlank();
	consoleClear();
	
	const char *base;
	const char *pixel;
	const char *pixb;
	base = font_info;
	//pixb = (ASCII_BASE + ASCII_FONT_BYTES * ( *("a") - ASCII_BEGIN ));//位置
	pixel = font_info + (ASCII_BASE + ASCII_FONT_BYTES * ( *("a") - ASCII_BEGIN ));
	printf("word:0x%x\n", *("a"));
	printf("base:0x%x\n", base);
	printf("weizhi:0x%x\n",(ASCII_BASE + ASCII_FONT_BYTES * ( *("a") - ASCII_BEGIN )));
	printf("base+weizhi:0x%x\n", pixel);

	//printf("0x%x\n",0x36A+0x10*(*("a"+1)-0x40+(*("a")-0x81)*0xBF));
	*///查找结束
	
	
	videoSetMode(MODE_FB0);
	videoSetModeSub(MODE_5_2D);
	vramSetBankA(VRAM_A_LCD);
	
	println(113, 93, "12345asdbwWQQY+你猜一猜");


	while (1) {
		swiWaitForVBlank();
		scanKeys();
		if (!(keysHeld() & KEY_A)) break;
	}
	return 0;
}
