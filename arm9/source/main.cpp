#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <cstdarg>

//#include "pixelprint.c"

//国字编程部分////////////////////////////////////////////////////////////////////////////////////////////////
void shn(int x, int y,uint16* buffer,uint16 color){
	buffer += y*SCREEN_WIDTH + x;
	*buffer = color;
}

u16 fdate[16]={0x0000,0x7ffc,0x4004,0x5ff4,0x4104,0x4104,0x4104,0x4fe4,0x4144,0x4124,0x4124,0x5ff4,0x4004,0x4004,0x7ffc,0x4004};
u16 test;

void drawf(){
	for(int y=0;y<16;y++){
		test = fdate[y];
		for (int x=0;x<16;x++){
			if((test&0x8000) !=0){
				shn(x+10,y+10,VRAM_A,RGB15(31,0,0));
			}else{
				shn(x+10,y+10,VRAM_A,RGB15(0,0,0));
			}
		test <<=1;
		}
	}
}

//F1，F2，化Cn ///////////////////////////////////////////////////////////////////////////////////////////
const u8 buttons_pic[6][10] = 
  {
    {8, 0x4f, 0x62, 0x42, 0x4E, 0x42, 0x42, 0x42, 0xE2, 0x00},//F1
    {9, 0xcf, 0x44, 0x0a, 0x74, 0x28, 0x48, 0x88, 0x08, 0xf1},//F2
    {9, 0xCF, 0x44, 0x0a, 0x74, 0x24, 0x50, 0xa4, 0x48, 0x61},//F3
    {9, 0x06, 0x12, 0x04, 0x08, 0x10, 0x2B, 0x6c, 0x4a, 0x93},//Cn
    {9, 0x0e, 0x08, 0xd0, 0x23, 0x49, 0x92, 0x5c, 0x09, 0x11},//Jp
    {9, 0x26, 0x53, 0xa6, 0x4a, 0x93, 0x26, 0x55, 0x4a, 0x93}//ok
  };


void button_draw(int x, int y,const u8* bit){
	const u8 *pixel;
	u8 delta;
	u16 mask,i,j;

	u16 *current;  // 显存位置VRAM_A
	current = VRAM_A + y*SCREEN_WIDTH + x; 
	pixel = bit+1;
	mask = 1;
	delta = SCREEN_WIDTH - *bit;

	for(i=1; i <= 8; ++i){
		for(j=1; j <= *bit; ++j){
			if(*pixel & mask){
				*current = RGB15(31,0,0);
			}
			++current;
			if(mask == 0x80) {
				mask = 1;
				++pixel;
			} 
			else mask <<= 1;
		}
		current += delta;
	}
}


//字符模数画字部分//////////////////////////////////////////////////////////////////////////////////
u16 pen_color = RGB15(31,0,0);  // 画笔颜色
extern const u8 font_info[];
//#define MASK_INIT   0x01
//#define MASK_REINIT 0x80


#define ASCII_FONT_WIDTH             6           //单字节码字模宽
#define ASCII_FONT_HEIGHT            12          //单字节码字模高
#define ASCII_FONT_BYTES             9           //一个单字节码字模占用的空间

#define HZ_FONT_WIDTH                11          //双字节码字模宽
#define HZ_FONT_HEIGHT               11          //双字节码字模高
#define HZ_FONT_BYTES                16          //一个双字节码字模占用的空间

#define IS_ASCII_CODE(byte_ptr)       ( *(byte_ptr) >= 0x21 ) && ( *(byte_ptr) <= 0x80 )  //判断一个位置的字符是否是单字节码
#define ASCII_FONT_LOCATE(byte_ptr)   0xa + 9 * ( *(byte_ptr) - 0x21 )  //找到单字节码在字库中的字模的首位

#define IS_HZ_CODE(byte_ptr)         ( *(byte_ptr) >= 0x81 ) && ( *(byte_ptr) <= 0xfe ) && ( *( (byte_ptr)+1 ) >= 0x40 ) && ( *( (byte_ptr)+1 ) <= 0xfe) //判定给定位置上的字符是否是GBK码
#define HZ_FONT_LOCATE(byte_ptr)     0x36A + 16 * ( *( (byte_ptr)+1 ) - 0x40  + ( *(byte_ptr) - 0x81 ) * 0xBF ) //找到汉字在字库中的首位置

int pxa,pya;
void drawAscii(int x, int y, byte *code){////point lt, byte *code){
	const unsigned char *pixel;
 	byte mask, i, j;  // 循环体中的计数
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
				if(mask == 0x80){//MASK_REINIT
					mask = 1;//MASK_INIT;
					++pixel;
				} //用完了一个BYTE的字模数据要往把指针下移
				else mask <<= 1;
			}
			current += SCREEN_WIDTH - ASCII_FONT_WIDTH;
		}
		pxa=rbx;
		pya=y;// 下一个字符的左上角位置///////////////////////////////////////////
	}
	pxa=x;pya=y;
}

int pxg,pyg;
void drawGbk(int x, int y, byte *code){
	const unsigned char *pixel;
	byte i, j, mask;

	int rbx = x + HZ_FONT_WIDTH; 
	int rby = y + HZ_FONT_HEIGHT;
	u16 *current;  // 显存位置
	if(x <= SCREEN_WIDTH && y <= SCREEN_HEIGHT && rbx <= SCREEN_WIDTH && rby <= SCREEN_HEIGHT){
		current = VRAM_A + y*SCREEN_WIDTH + x;
		pixel = font_info + HZ_FONT_LOCATE(code);
		mask = 1;//MASK_INIT;
		for(i=1; i <= HZ_FONT_HEIGHT; ++i){
			for(j=1;j <= HZ_FONT_WIDTH; ++j){
				if(*pixel & mask){
					*current = pen_color;
				}//可能范围不对
				++current;
				if(mask == 0x80) {//MASK_REINIT
					mask = 1;//MASK_INIT;
					++pixel;
				}
		    	else mask <<= 1;
			}
			current += SCREEN_WIDTH - HZ_FONT_WIDTH;
		}
		pxg=rbx+1;pyg=y;
  	}
 	pxg=x;pyg=y;
}


int println(int x, int y, byte *str){

	byte *org_str = str;       // 保存起始位置，返回值用到
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

/*
void printop(int x, int y, const char* p){
	
	byte cb[10] = {0};
	memcpy((char*)cb,p,4);
	println(x, y, cb);
}
*/


//主程序/////////////////////////////////////////////////
int main() {
	
	videoSetMode(MODE_FB0);
	videoSetModeSub(MODE_5_2D);
	vramSetBankA(VRAM_A_LCD);
	
	println(113, 93, "gfd:");

	button_draw( 23, 23, buttons_pic[4] );//工作正常

	while (1) {
		swiWaitForVBlank();
		scanKeys();
		if (!(keysHeld() & KEY_A)) break;
	}
	return 0;
}
