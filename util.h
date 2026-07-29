#ifndef _UTIL_H_
#define _UTIL_H_

#include	"AppSet.h"

#if SKT_APP == 1
#include	"WIPIheader.h"
#endif

#if KTF_APP == 1
#include	<WIPIHeader.h>
	#if TOUCH_MODE == 1
	#define	MV_POINTER_PRESS_EVENT		MV_POINT_PRESS_EVENT
	#endif
#endif

#if LGT_APP == 1
#include "exp_wipic.h"
	#if TOUCH_MODE == 1
	#include "exp_wipic_lgt.h"
	#define	MV_POINTER_PRESS_EVENT		LGTV_POINTER_DOWN
	#define	MV_POINTER_MOVE_EVENT		LGTV_POINTER_MOVE
	#define	MV_POINTER_REPEAT_EVENT		LGTV_POINTER_REPEAT
	#define	MV_POINTER_RELEASE_EVENT	LGTV_POINTER_UP
	#endif
#endif

#if SKT_APP != 1
typedef struct MC_PointerEvent {
	M_Int32	x;
	M_Int32	y;
	M_Int32	key;
} MC_PointerEvent;
#endif

typedef struct _ptr
{
	M_Byte*		m_pBuff;							///<	메모리 포인터
	M_Uint32	m_BuffID;							///< 	메모리 포인터 ID
	
}PTR;

enum{
		WIPI_KEY_UP = -1,
		WIPI_KEY_DOWN = -2,
		WIPI_KEY_LEFT = -3,
		WIPI_KEY_RIGHT = -4,

		WIPI_KEY_UP_SCR = 100,
		WIPI_KEY_DOWN_SCR = 101,
		
		WIPI_KEY_OK = -5,
		WIPI_KEY_CLR = -16,
		
		WIPI_KEY_SOFT1 = -6,
		WIPI_KEY_SOFT2 = -7,
		WIPI_KEY_STAR = 42,
		WIPI_KEY_SARP = 35,
		WIPI_KEY_0 = 48,
		WIPI_KEY_1,
		WIPI_KEY_2,
		WIPI_KEY_3,
		WIPI_KEY_4,
		WIPI_KEY_5,
		WIPI_KEY_6,
		WIPI_KEY_7,
		WIPI_KEY_8,
		WIPI_KEY_9	
};

extern char		g_TempStr1[100];
extern char		g_TempStr2[512];	//각종 에러메시지용 (NetModul, DSPModul에서 주로 사용된다.)

extern const int fsinTbl[36];
extern const int fcosTbl[36];

extern MC_PointerEvent		g_pPointerEvt;


M_Int16 ByteToShort(M_Byte * buff);
int ByteToInt(M_Byte * bytes, int offset);

M_Byte Collision_Chk(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh);

void NotOverlapRand(int* RandSlot, int cnt, int RandMax);
void Bubble_sort(int* buf, int size, int flag);

int File_Write(char *file_name, M_Byte *bin, unsigned short bin_len);
int File_Read(char *file_name, M_Byte *bin, unsigned short max_len);

unsigned int WRand(void);
void WSrand(unsigned int seed);

int umin( int a, int b );
int umax( int a, int b );

int umin3( int a, int b, int c );
int umax3( int a, int b, int c );

M_Int8 NumPosMax( M_Uint32 v );

M_Boolean PointCheck(int check_x, int check_y, int check_w, int check_h);

M_Boolean	exAlloc( PTR* p, M_Uint32 len );
void		exFree( PTR* p );

void		dspSaveFile( M_Byte* FileName, M_Byte* FileContent,  M_Int32 FileSize );
void		dspLoadFile( M_Byte* FileName, M_Byte* FileContent, M_Int32 FileSize );
M_Uint32	dspGetFileSize( M_Byte* FileName );


/*int a = atoi(str);
  a(알파벳) => i(숫자)
   
  반대로..
  String a = itoa(i);
  i(숫자) => a(알파벳)
*/

#endif