#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <cstdarg>


#define ASCII_BEGIN	0x20        //第一个字符个数编号位置
#define ASCII_END	0x80

//**********元常量************/
#define ASCII_FONT_WIDTH             6           //单字节码字模宽
#define ASCII_FONT_HEIGHT            12          //单字节码字模高
#define ASCII_FONT_BYTES             0x9           //一个单字节码字模占用的空间

#define HZ_FONT_WIDTH                16          //双字节码字模宽
#define HZ_FONT_HEIGHT               11          //双字节码字模高
#define HZ_FONT_BYTES                0x16          //一个双字节码字模占用的空间

#define MASK_INIT   0x01
#define MASK_REINIT 0x80


u16 pen_color = RGB15(31,0,0);  // 画笔颜色
extern const  char font_asc[];
extern const  char font_cn[];
int px,py;//位置函数



u32 utf8to16(const  char *text){//utf8转uc码
	u16 out;
	for(uint i = 0; i < 2;){
		char16_t c;
		if(!(text[i] & 0x80)){
			c = text[i++];
		}else if((text[i] & 0xE0) == 0xC0){
			c  = (text[i++] & 0x1F) << 6;
			c |=  text[i++] & 0x3F;
		}else if((text[i] & 0xF0) == 0xE0){
			c  = (text[i++] & 0x0F) << 12;
			c |= (text[i++] & 0x3F) << 6;
			c |=  text[i++] & 0x3F;
		}else{
			i++; 
		}
		out += c;
	}
	return out;
}



bool IS_ASCII_CODE(const  char *byte_ptr){//判断一个位置的字符是否是单字节码
	if(utf8to16(byte_ptr) < 0x128 ){  
		return true;
	}else{	
		return false;
	}
}


u16 ASCII_FONT_LOCATE(const  char *byte_ptr){ //找到单字节码在字库中的字模的首位
	return (0xa + ASCII_FONT_BYTES * ( *(byte_ptr) - ASCII_BEGIN )); 
}


u32 CN_FONT_LOCATE(const  char *byte_ptr){ //找到单字节码在字库中的字模的首位
	u32 f = utf8to16(byte_ptr);
	return (f - 0x4e00)* HZ_FONT_BYTES; 
}




void drawAscii(int x, int y, const  char *code){////point lt, byte *code){
	const  char *pixel;
 	u16 mask, i, j;  // 循环体中的计数
	
	int rbx = x + ASCII_FONT_WIDTH; //rb
	int rby = y + ASCII_FONT_HEIGHT;
	u16 *current;  // 显存位置
	if(x <= SCREEN_WIDTH && y <= SCREEN_HEIGHT && rbx <= SCREEN_WIDTH && rby <= SCREEN_HEIGHT){ // 可以描出字符
		current = VRAM_A + y*SCREEN_WIDTH + x;             // 找到左上角对应的顶点内存
		pixel = font_asc + ASCII_FONT_LOCATE(code);     // 找到单字符对应的字模内存
		mask = 1;//MASK_INIT;

		for(i=0; i < ASCII_FONT_HEIGHT; ++i){
			for(j=0; j < ASCII_FONT_WIDTH; ++j){
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
		px=rbx;
		py=y;// 下一个字符的左上角位置///////////////////////////////////////////
	}
	else{
		px=x;
		py=y;
	}
}


void drawGbk(int x, int y, const  char *code){
	const  char *pixel;
	u16 i, j, mask;

	int rbx = x + 11;//HZ_FONT_WIDTH; 
	int rby = y + HZ_FONT_HEIGHT;
	u16 *current;  // 显存位置
	if(x <= SCREEN_WIDTH && y <= SCREEN_HEIGHT && rbx <= SCREEN_WIDTH && rby <= SCREEN_HEIGHT){
		current = VRAM_A + y*SCREEN_WIDTH + x;
		pixel = font_cn + CN_FONT_LOCATE(code);
		mask = MASK_INIT;
		for(i=0; i < HZ_FONT_HEIGHT; ++i){//高
			for(j=0;j < HZ_FONT_WIDTH; ++j){//宽
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
		px=rbx+1;
		py=y;
	}
	else{
		px=x;
		py=y;
	}
}

void drawGbk2(int x, int y, u32 code){
	const  char *pixel;
	u16 i, j, mask;

	int rbx = x + HZ_FONT_WIDTH; 
	int rby = y + HZ_FONT_HEIGHT;
	u16 *current;  // 显存位置
	if(x <= SCREEN_WIDTH && y <= SCREEN_HEIGHT && rbx <= SCREEN_WIDTH && rby <= SCREEN_HEIGHT){
		current = VRAM_A + y*SCREEN_WIDTH + x;
		pixel = font_cn + code;
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
	}
}

void println(int x, int y, const  char *str){//point lt,
	//const  char *org_str = str;       // 保存起始位置，返回值用到
	int next_ltx,next_lty; //next_lt            // 保存下一个字符的左上角位置
	for(;*str!=0 ;str++){
		if( !IS_ASCII_CODE(str) ){   // 汉字就调用drawGbk
			drawGbk(x, y, str);
			next_ltx=px;
			next_lty=py;

			if(next_ltx == x) break; //此行已无法往后写，不写了该返回了
				x = next_ltx;
				y = next_lty;
				++str;
		}else if( IS_ASCII_CODE(str) ){
			drawAscii(x, y, str);
			next_ltx=px;
			next_lty=py;

			if(next_ltx == x) break;
			x = next_ltx;
			y = next_lty;
		}
	}
	//if(*str == '\n') ++str;
	//return str - org_str;
}








PrintConsole topScreen;//文字需要
PrintConsole bottomScreen;
//主程序/////////////////////////////////////////////////
int main() {

	
	/*///打印字模查找部分//////////////工作正常
	videoSetMode(MODE_0_2D);
	videoSetModeSub(MODE_0_2D);
	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);
	consoleInit(&topScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
	consoleInit(&bottomScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);
	consoleSelect(&bottomScreen);
	swiWaitForVBlank();
	consoleClear();
	
	const char *pBuf = "丂";
	unsigned char *pTmp = (unsigned char *)pBuf;

	
	//printf("word:%c\n",pBuf);
	//printf("base:%x\n",pBuf);
	//printf("base:%x\n",utf8to16("丅"));
	//printf("base:%x\n",pTmp);
	//printf("base:%x\n",pTmp);
	//printf("base:%x\n",*("丂"));
	//printf("base:%c\n","丂");
	//printf("base:%x\n",utf8to16("你"));
	printf("base:%x\n",utf8to16("啊"));
	
	//char xx[] = "爱";
	//printf ("%x%x%x", (unsigned char) xx[0], (unsigned char) xx[1], (unsigned char) xx[2] ); 

	//printf("0x%x\n",0x36A+0x10*(*("a"+1)-0x40+(*("a")-0x81)*0xBF));
	*/
	
	videoSetMode(MODE_FB0);
	videoSetModeSub(MODE_5_2D);
	vramSetBankA(VRAM_A_LCD);
	
	

	drawGbk2(55,25,((0x6c88 - 0x4e00) * 0x16));
	drawGbk(55,45,"好");
	println(13, 93, "好de好");
	

	while (1) {
		swiWaitForVBlank();
		scanKeys();
		if (!(keysHeld() & KEY_A)) break;
	}
	return 0;
}
