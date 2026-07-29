/*********************************************************************
                           - INFO -

  @ Name				: EASTVIL WIPI C DRAW ENGINE 3.1
  @ Version				: 3.1
  @ Date				: 2009. 4. 13
  @ Make				: Kim Jong Wook / Shin Hee Jung

  Copyright(C) 2008-2009 ..E.A.S.T.V.I.L..
*********************************************************************/

#ifndef _DRAW_H_
#define _DRAW_H_

#include	"AppSet.h"

#if SKT_APP == 1
#include	"WIPIheader.h"
#endif

#if KTF_APP == 1
#include	<WIPIHeader.h>
#endif

#if LGT_APP == 1
#include "exp_wipic.h"
#endif


typedef struct _DClip
{
	M_Int32	sx;	//이미지의 시작점 X좌표
	M_Int32	sy;	//이미지의 시작점 Y좌표
	M_Int32	dx;	//스크린에 찍을 X좌표
	M_Int32	dy;	//스크린에 찍을 Y좌표
	M_Int32	sw;	//출력할 가로 크기
	M_Int32	sh;	//출력할 세로 크기
}DClip;


typedef struct {
	
	M_Int32		w_seek;	
	M_Int32		w_seek_tmp;
	M_Int32		w_seek_start;	
	M_Int32		max_dx;
	M_Int32		max_line_count;
	M_Int32		w_dy; 
	M_Int32		vRGB;
	
}STREX;

///////////////////////////////////////////////////////////////////////////////////////	Basic Section
#define DRAW_GRADIENT			0	//그라데이션
#define DRAW_EX_GRADIENT		0	//확장 그라데이션

MC_GrpContext* GetGC( void );

void SetTrans( MC_GrpImage img );
void SetDrawEng(MC_GrpContext *gc, M_Int32 LCD_WIDTH, M_Int32 LCD_HEIGHT, MC_GrpFrameBuffer *mainFrameBuffer, MC_GrpFrameBuffer *offScreenBuffer);

int GetCenterX(int v);
int GetCenterY(int v);

void LoadImage(MC_GrpImage* image, char* path, M_Int32 idx);
int imgres_idget(char* path);

void DrawImage( MC_GrpImage img, M_Int32 x, M_Int32 y);
void DrawImageEx( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32 w, M_Int32 h, M_Int32 dx, M_Int32 dy );
void DrawString( M_Int32 x, M_Int32 y, const char* str, void* color );
void ReleseImage(MC_GrpImage* image);

void DFillRect( M_Int32 x, M_Int32 y, M_Int32 w, M_Int32 h, void* color );
void DDrawRect( M_Int32 x, M_Int32 y, M_Int32 w, M_Int32 h, void* color );
void DFillArc( M_Int32 x, M_Int32 y, M_Int32 w, M_Int32 h, M_Int32 s, M_Int32 e, void* color );
void DDrawArc( M_Int32 x, M_Int32 y, M_Int32 w, M_Int32 h, M_Int32 s, M_Int32 e, void* color );
void DDrawLine( M_Int32 x1, M_Int32 y1, M_Int32 x2, M_Int32 y2, void* color );

M_Int8 DDCliper( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32 w, M_Int32 h, DClip* rt);

M_Int32 GetFontID(void);

void SaveLCD(MC_GrpFrameBuffer* buff);
void RestoreLCD(MC_GrpFrameBuffer* buff);

void DrawNumber(MC_GrpImage img, M_Uint32 number, int x, int y, int w, int h, int space, char flag);
M_Int32 dspLineCnt( const M_Char* str, M_Int32 max);
void dspDrawString( const M_Char* str, M_Int32 x, M_Int32 y, M_Int32 max, M_Int16 hy, M_Int8 center, M_Int8 r, M_Int8 g, M_Int8 b, M_Int8 line, M_Int8 page);
void DrawStringC(int x, int w, int y, char* str, M_Uint8 r, M_Uint8 g, M_Uint8 b);


void GDrawGradientLine(M_Int32 x, M_Int32 y, M_Int32 whith, M_Int32 height, M_Int32 cColor, M_Int32 eColor, M_Int8 flag);
void GDrawExGradientLine(M_Int32 x, M_Int32 y, M_Int32 x2, M_Int32 y2, M_Int32 length, M_Int32 height, M_Int32 cColor, M_Int32 eColor, M_Int8 flag);


void SetFontInfo(void);
void Set_StrEx(M_Int32 seek_start, M_Int32 wdx, M_Int32 ldy, M_Int32 max_line, M_Int32 vRGB);
void ResetSeek_StrEx(void);
M_Boolean Key_StrEx(void);
void Draw_StrEx(char* str, int x, int y);

///////////////////////////////////////////////////////////////////////////////////////	Matrix Section
#define DRAW_ROTATE				0	//이미지 회전(좌측 상단 기준) : DDrawBuffReSize()
#define DRAW_CENTER_ROTATE		0	//이미지 회전(이미지 중간 기준) : DDrawImageCenterRotate()
#define DRAW_BUFFRESIZE			0	//화면 확대/축소 : DDrawBuffReSizeInit(), DDrawBuffReSizeRelese(), DDrawBuffReSize()
#define DRAW_FLIP_EX			1	//DDrawImageFlip()의 확장

void SetAlpha( M_Int32 alpha );
void DDrawImage( MC_GrpImage img, M_Int32 x, M_Int32 y);
void DDrawImageReSize( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32 nw, M_Int32 nh );
void DDrawImageFlip( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int8 mode );	//차후 삭제 예정 DDrawImageFlipEx()함수로 사용.

void DDrawImageRotate( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32 angle );
void DDrawImageCenterRotate( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32 angle );

void DDrawBuffReSizeInit( void );
void DDrawBuffReSizeRelese( void );
void DDrawBuffReSize( M_Int32 x, M_Int32 y, M_Int32 nw, M_Int32 nh );

enum {
	DD_TRAN_NON,
	DD_TRAN_ROT90,
	DD_TRAN_ROT180,
	DD_TRAN_ROT270,
	DD_TRAN_MIR,
	DD_TRAN_MIR_ROT90,
	DD_TRAN_MIR_ROT180,
	DD_TRAN_MIR_ROT270
};
void DDrawImageFlipEx( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32 w, M_Int32 h, M_Int32 cx, M_Int32 cy, M_Int8 mode );




///////////////////////////////////////////////////////////////////////////////////////	Distort Section
#define DRAW_FILL_COLOR			0	//컬러키를 제외하고 지정한 색상으로 채운다. : DDrawImageFillColor()
#define DRAW_COLOR_TO			0	//팔레트변환 : DDrawImageColorTo()

#define DRAW_MOSAIC				0	//모자이크효과 : DDrawImageMosaic()
#define DRAW_GRAY				0	//그레이스케일변환 : DDrawImageGray()
#define DRAW_BRIGHTNESS			0	//명도변환 : DDrawImageBrightness()
#define DRAW_RANDOM_DITHERING	0	//랜덤디더링 : DDrawImageRandomDithering()

#define DRAW_LANTERN			0	//랜턴 효과
#define DRAW_INVERT				0	//이미지 색반전
#define DRAW_BLUR				0	//블러 효과
#define DRAW_KERNEL				0	//커널필터 효과

#define DRAW_FILL_INVERT		0	//지정된 화면의 이미지 색반전
#define DRAW_FILL_BLUR			0	//전체화면 블러 효과
#define DRAW_FILL_RECTEX		0	//지정된 화면에 알파값이 포함된 사각형을 그린다.

void DDrawImageFillColor( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32 color );
void DDrawImageColorTo( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32* pDestColor, M_Int32* pToColor , M_Int32 length );

void DDrawImageMosaic( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32 Mos);
void DDrawImageGray( MC_GrpImage img, M_Int32 x, M_Int32 y);
void DDrawImageBrightness( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int16 v);
void DDrawImageRandomDithering( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32 flag);

void DrawLantern(MC_GrpImage Img, int x, int y, int w, int h, M_Int8 R, M_Int8 G, M_Int8 B);
void DrawInvert(MC_GrpImage img, int x, int y);
void DrawBlur(MC_GrpImage img, int x, int y, int val);
void DrawKernel(MC_GrpImage img, int x, int y, int* Kernel, int flag);

void DFillInvertRect(M_Int32 x, M_Int32 y, M_Int32 width, M_Int32 height);
void DFillInvertRect16(MC_GrpFrameBuffer g, M_Int32 x, M_Int32 y, M_Int32 width, M_Int32 height);
void DFillInvertRect32(MC_GrpFrameBuffer g, M_Int32 x, M_Int32 y, M_Int32 width, M_Int32 height);

void DFillBlurRect(void);
void DFillBlurRect16(MC_GrpFrameBuffer g, M_Int32 x, M_Int32 y, M_Int32 width, M_Int32 height);
void DFillBlurRect32(MC_GrpFrameBuffer g, M_Int32 x, M_Int32 y, M_Int32 width, M_Int32 height);

void DFillRectEx(M_Int32 dstX, M_Int32 dstY, M_Int32 width, M_Int32 height, M_Int32 color, M_Uint16 alpha);
void DFillRectEx16(MC_GrpFrameBuffer g, M_Int32 dstX, M_Int32 dstY, M_Int32 width, M_Int32 height, M_Int32 color, M_Uint16 alpha);
void DFillRectEx32(MC_GrpFrameBuffer g, M_Int32 dstX, M_Int32 dstY, M_Int32 width, M_Int32 height, M_Int32 color, M_Uint16 alpha);




///////////////////////////////////////////////////////////////////////////////////////	Blend Section
#define HALF_ALPHA				127

#define FLAG_ADD_BLAND			0	//이 값이 1일 경우 아래 블랜딩 모드의 알파값을 조절할 수 있다.(하지만 부하가 크기 때문에 비추)
#define DRAW_COLORDODGE			0	//ColorDodge
#define DRAW_LIGHTEN			0	//Lighten
#define DRAW_SCREEN				0	//Screen
#define DRAW_DIFFERENCE			0	//Different
#define DRAW_DARKEN				0	//Darken
#define DRAW_MULTIPLY			0	//Multiply
#define DRAW_ADDBLEND			0	//ADD
#define DRAW_SUBTRACT			0	//Subtract
#define DRAW_NEGATION			0	//Negation
#define DRAW_EXCLUSION			0	//Exclusion
#define DRAW_OVERLAY			0	//Overlay
#define DRAW_SOFTLIGHT			0	//SoftLight
#define DRAW_HARDLIGHT			0	//HardLight
#define DRAW_COLORBURN			0	//ColorBurn
#define DRAW_LINEARDODGE		0	//LinearDodge
#define DRAW_LINEARBURN			0	//LinearBurn
#define DRAW_LINEARLIGHT		0	//LinearLight
#define DRAW_VIVIDLIGHT			0	//VividLight
#define DRAW_PINLIGHT			0	//PinLight
#define DRAW_HARDMIX			0	//HardMix
#define DRAW_REFLECT			0	//Reflect
#define DRAW_GLOW				0	//Glow
#define DRAW_PHOENIX			0	//Phoenix

#define DRAW_RASTER				1	//래스터 연산 : DDrawImageRaster()
#define DRAW_DKEFF				0	//굴절효과 : DDrawDKEff()


void DDrawImageColorDodge(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImageLighten(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImageScreen(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImageDifference(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImageDarken(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImageMultiply(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImageAddBlend(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImageSubtract(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImageNegation(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImageExclusion(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImageOverlay(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImageSoftLight(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImageHardLight(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImageColorBurn(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImageLinearDodge(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImageLinearBurn(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImageLinearLight(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImageVividLight(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImagePinLight(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImageHardMix(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImageReflect(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImageGlow(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);
void DDrawImagePhoenix(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha);


enum {
	ROP_SRCINVERT,		//[Src^Des]배경과 설정값을 XOR 연산하여 출력한다.
	ROP_SRCAND,			//[Src&Des]배경과 설정값을 AND 연산하여 출력한다.
	ROP_SCRPAINT,		//[Src|Des]배경과 설정값을 OR 연산하여 출력한다.
	ROP_DSTINVERT,		//[~Des]배경을 Inverse한다.	
	ROP_NOTSRCCOPY,		//[~Src]설정값을 Inverse하여 출력 한다.
	ROP_SRCERASE,		//[Src&~Des]배경값을 Inverse한 다음 설정값과 AND 연산을 한다.	
	ROP_MERGEPAINT,		//[~Src|Des]설정값을 Inverse한 다음 배경값과 OR 연산을 한다.
	ROP_NOTSRCERASE,	//[~(Src|Des)]배경과 설정할 값을 OR 연산을 하고 난 후에 그 값을 Inverse한다.
	ROP_BLACKNESS,		//[0]검은색 출력
	ROP_WHITENESS		//[255]흰색 출력
};

void DDrawImageRaster( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int8 flag);
void DDrawDKEff( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int16 refraction );

///////////////////////////////////////////////////////////////////////////////////////	Test Section
#define DRAW_Z_RESIZE			0	//이미지에 기울기를 준다.

void DDrawImageZReSize( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32 nw, M_Int32 nh, M_Int32 z );






#endif