#include	"util.h"

#pragma warning(disable : 4013)
#pragma warning(disable : 4018)
#pragma warning(disable : 4022)
#pragma warning(disable : 4028)
#pragma warning(disable : 4024)
#pragma warning(disable : 4047)
#pragma warning(disable : 4101)
#pragma warning(disable : 4133)
#pragma warning(disable : 4146)
#pragma warning(disable : 4244)
#pragma warning(disable : 4305)
#pragma warning(disable : 4312)
#pragma warning(disable : 4761)

const int fsinTbl[36]={
	 0,        17,         34,         49,         64,         76,         86,         93,         98,         100,        
	98,         93,         86,         76,         64,         50,         34,         17,         0,          -17,        
	-34,        -49,        -64,        -76,        -86,        -93,        -98,        -100,       -98,        -93,        
	-86,        -76,        -64,        -50,        -34,        -17        };

const int fcosTbl[36]={       
	99,         98,         93,         86,         76,         64,         50,         34,         17,         0,          
	-17,        -34,        -49,        -64,        -76,        -86,        -93,        -98,        -100,       -98,        
	-93,        -86,        -76,        -64,        -50,        -34,        -17,        0,          17,         34,         
	49,         64,         76,         86,         93,         98         };

char		g_TempStr1[100];		//문자열을 저장할 임시 변수
char		g_TempStr2[512];

MC_PointerEvent		g_pPointerEvt;		//터치 패라메터
/*---------------------------------------------------------------------------*
Description :	1바이트 데이터 2개를 합쳐서 2바이트 데이터로 반환해준다.
				(M_Byte + M_Byte = M_Int16)
Arguments :		clip			- 사운드 클립
				status			- 콜백 상태값
Returns :		1바이트 두개가 합쳐진 2바이트 데이터(M_Int16)
*---------------------------------------------------------------------------*/
M_Int16 ByteToShort(M_Byte * buff)
{
	return (buff[0] & 0x00ff) | ((buff[1] << 8) & 0xff00);
}

/*---------------------------------------------------------------------------*
Description :	1바이트 데이터 4개를 합쳐서 4바이트 데이터로 반환해준다.
(M_Byte + M_Byte = M_Int16)
Arguments :		
Returns :		1바이트 두개가 합쳐진 2바이트 데이터(M_Int16)
*---------------------------------------------------------------------------*/
int ByteToInt(M_Byte * bytes, int offset)
{
	int temp = 0;
	int data = 0;
	
	temp = bytes[0+offset];
	data = ((temp & 0xFF) << 24);  																					//--총길리 32 바이트중 맨뒷부분에서 8바이트만 검색
	
	temp = bytes[1+offset];
	data += ((temp & 0xFF) << 16);
	
	temp = bytes[2+offset];
	data += ((temp & 0xFF) << 8);				   
	
	temp = bytes[3+offset];										   
	data += (temp & 0xFF);
	
	return data;
}

/*---------------------------------------------------------------------------*
Description :	충돌체크
Arguments :		sx			- 충돌할 오브젝트의 x좌표
				sy			- 충돌할 오브젝트의 y좌표
				sw			- 충돌할 오브젝트의 가로 크기
				sh			- 충돌할 오브젝트의 세로 크기
				dx			- 충돌될 오브젝트의 x좌표
				dy			- 충돌될 오브젝트의 y좌표
				dw			- 충돌될 오브젝트의 가로 크기
				dh			- 충돌될 오브젝트의 세로 크기
Returns :		충돌 체크 여부 (TRUE = 충돌)
*---------------------------------------------------------------------------*/
M_Byte Collision_Chk(int sx, int sy, int sw, int sh,
					 int dx, int dy, int dw, int dh)
{
	if( sx+sw <= dx ) 
		return FALSE;
	
	if( sx >= dx+dw )
		return FALSE;
	
	if( sy+sh <= dy ) 
		return FALSE;
	
	if( sy >= dy+dh ) 
		return FALSE;
	
	// 충돌
	return TRUE;
}

/*---------------------------------------------------------------------------*
Description:	겹치지 않는 랜덤값

Arguments:		*RandSlot	: 겹치지 않는 랜덤값을 결과를 저장할 배열
				cnt			: 배열의 개수
				RandMax		: 랜덤값의 한계치
			
Returns:      void
*---------------------------------------------------------------------------*/
void NotOverlapRand(int* RandSlot, int cnt, int RandMax)
{
	int rnd_slot_val, i, j;
	
	for(i=0;i < cnt;  )
	{
		rnd_slot_val = WRand()%RandMax;
		
		for(j=0; j< i; j++)
		{
			if (RandSlot[j] == rnd_slot_val) break;
		}
		
		if (i==j) RandSlot[i++] = rnd_slot_val;
	}
}

/*---------------------------------------------------------------------------*
Description:  버블소트
  
Arguments:		buf : 소트할 배열
				size : 배열의 개수
				flag : FALSE - 작은수가 앞으로, TRUE - 큰수가 앞으로
				
Returns:      void
*---------------------------------------------------------------------------*/
void Bubble_sort(int* buf, int size, int flag)
{
	int i = 0;
	int j = 0;
	int temp = 0;
	
	for(i = size-1; i > 0; i--)
	{
		for(j = 0; j < i; j++)
		{
			if (flag == FALSE)
			{
				if(buf[j] > buf[j+1])
				{
					temp = buf[j];
					buf[j] = buf[j+1];
					buf[j+1] = temp;
				}
			}
			else
			{
				if(buf[j] < buf[j+1])
				{
					temp = buf[j];
					buf[j] = buf[j+1];
					buf[j+1] = temp;
				}
			}
			
		}
	}
}

/*---------------------------------------------------------------------------*
Description :	파일 쓰기
Arguments :		file_name	- 파일명
bin			- 저장할 데이터
bin_len		- 저장할 데이터의 크기
Returns :		저장할 데이터의 크기

  ECT :			저장위치는 "Bin\wipi\app\클래스아이디\파일명"에 저장된다.
  예) C:\SDK\SKT\Clet\Bin\wipi\app\CID00SID00
*---------------------------------------------------------------------------*/
int File_Write(char *file_name, M_Byte *bin, unsigned short bin_len)
{
	M_Int32 fileID;
	MC_FileInfo fi;
	
	fileID = MC_fsOpen(file_name, MC_FILE_OPEN_WRTRUNC, MC_DIR_PRIVATE_ACCESS);
	if(fileID < 0) 
		return fileID;
	
	bin_len = (unsigned short)MC_fsWrite(fileID, bin, bin_len);
	
	MC_fsClose(fileID);
	
	return bin_len;
}

/*---------------------------------------------------------------------------*
Description :	파일 읽기
Arguments :		file_name	- 파일명
bin			- 읽올 데이터
bin_len		- 읽을 데이터의 크기
Returns :		읽은 데이터의 크기
*---------------------------------------------------------------------------*/
int File_Read(char *file_name, M_Byte *bin, unsigned short max_len)
{
	M_Int32 fileID;
	int len;
	
	fileID = MC_fsOpen(file_name, MC_FILE_OPEN_RDONLY, MC_DIR_PRIVATE_ACCESS);
	if(fileID < 0)
		return fileID;
	
	len = MC_fsRead(fileID, (M_Byte*)bin, max_len );
	
	
	MC_fsClose(fileID);
	
	return len;
}


/*---------------------------------------------------------------------------*
Description :	랜덤시드
Arguments :		seed - 시드값
Returns :		void
*---------------------------------------------------------------------------*/
unsigned int rd;
void WSrand(unsigned int seed) { rd = seed;}

/*---------------------------------------------------------------------------*
Description :	랜덤
Arguments :		void
Returns :		난수 리턴
*---------------------------------------------------------------------------*/
unsigned int WRand(void)
{
	rd = rd * 1103515245L + 12345;
	return((unsigned)((rd / 65536L) % 65533));
}

/*---------------------------------------------------------------------------*
Description :	비교해서 큰수를 리턴한다.
Arguments :		void
Returns :		난수 리턴
*---------------------------------------------------------------------------*/
int umin( int a, int b )
{
	if( a < b ){ return a; }
	else { return b; }
}

/*---------------------------------------------------------------------------*
Description :	비교해서 작은 수를 리턴한다.
Arguments :		void
Returns :		난수 리턴
*---------------------------------------------------------------------------*/
int umax( int a, int b )
{
	if( a > b ){ return a; }
	else { return b; }
}

/*---------------------------------------------------------------------------*
Description :	비교해서 큰수를 리턴한다. (3항)
Arguments :		void
Returns :		난수 리턴
*---------------------------------------------------------------------------*/
int umin3( int a, int b, int c )
{
	if( a < b || a < c)			{ return a; }
	else if ( b < a || b < c)	{ return b; }
	else { return c; }
}

/*---------------------------------------------------------------------------*
Description :	비교해서 작은 수를 리턴한다. (3항)
Arguments :		void
Returns :		난수 리턴
*---------------------------------------------------------------------------*/
int umax3( int a, int b, int c )
{
	if( a > b || a > c)			{ return a; }
	else if ( b > a || b > c)	{ return b; }
	else { return c; }
}


/*---------------------------------------------------------------------------*
Description :	몇자리 숫자인지 반환한다.
Arguments :		void
Returns :		난수 리턴
*---------------------------------------------------------------------------*/
M_Int8 NumPosMax( M_Uint32 v )
{
	M_Int8 NumPos_Flag;

	if		(v/10000000000 != 0)	NumPos_Flag = 10;
	else if (v/1000000000 != 0)		NumPos_Flag = 9;
	else if (v/100000000 != 0)		NumPos_Flag = 8;
	else if (v/10000000 != 0)		NumPos_Flag = 7;
	else if (v/1000000 != 0)		NumPos_Flag = 6;
	else if (v/100000 != 0)			NumPos_Flag = 5;
	else if (v/10000 != 0)			NumPos_Flag = 4;
	else if (v/1000 != 0)			NumPos_Flag = 3;
	else if (v/100 != 0)			NumPos_Flag = 2;
	else if (v/10 != 0)				NumPos_Flag = 1;
	else							NumPos_Flag = 0;
	
	return NumPos_Flag;
}

/*---------------------------------------------------------------------------*
Description :	터치영역 체크
Arguments :		check_x	- 체크 영역 시작점 x좌표
				check_y	- 체크 영역 시작점 y좌표
				check_w	- 체크 영역 가로 크기
				check_h	- 체크 영역 세로 크기
Returns :		TRUE - 설정한 영역 체크
*---------------------------------------------------------------------------*/
M_Boolean PointCheck(int check_x, int check_y, int check_w, int check_h)
{

#if TOUCH_RECT_DRAW == 1
	MC_grpSetContext( GetGC(), MC_GRP_CONTEXT_ALPHA_IDX, 128 );
	DFillRect( check_x, check_y, check_w, check_h, (void*)MC_grpGetPixelFromRGB(255, 0, 255) );
	MC_grpInitContext( GetGC() );
#endif

	if((check_x <= g_pPointerEvt.x && check_x+check_w >= g_pPointerEvt.x) && (check_y <= g_pPointerEvt.y && check_y+check_h >= g_pPointerEvt.y) )	return TRUE;
	else																																			return FALSE;
}

/*---------------------------------------------------------------------------*
*	메모리 할당 후 id값을 얻어 해당 id에 포인터를 가져와 m_pBuff 에 할당하게 됩니다.
* @param PTR* p : 동적 생성 변수 관리 구조체 포인터.
* @param M_Uint32 len : 동적 생성 변수 크기.
* @return M_Boolean : 처리 성공 여부 TRUE, FALSE.
*---------------------------------------------------------------------------*/
M_Boolean exAlloc(PTR* p, M_Uint32 len)
{
	if( p != NULL )
	{
		if( p->m_BuffID ) return FALSE;
		
		p->m_BuffID = MC_knlAlloc( len );
		p->m_pBuff = (M_Byte*)MC_GETDPTR( p->m_BuffID );
		memset( p->m_pBuff, 0, len );
		
		return TRUE;
	}
	return FALSE;
}

/*---------------------------------------------------------------------------* 
*	동적 생성한 변수의 포인터를 해제 시킵니다.
* @param PTR* p : 동적 생성 변수 관리 구조체 포인터.
*---------------------------------------------------------------------------*/
void exFree( PTR* p )
{
	if( p != NULL )
	{
		if( p->m_BuffID == NULL || !p->m_BuffID ) return;
		
		MC_knlFree( p->m_BuffID );
		p->m_BuffID = 0;
	}
}

void dspSaveFile( M_Byte* FileName, M_Byte* FileContent, M_Int32 FileSize)
{
	M_Int32 fileID = 0;
	M_Int32 writePoint = 0;
	M_Int32 writeLength = 0;

	fileID = MC_fsOpen(FileName, MC_FILE_OPEN_WRTRUNC, MC_DIR_PRIVATE_ACCESS);

	while(1) {
		if(FileSize - writePoint >= 256)
			writeLength = 256;
		else
			writeLength = FileSize - writePoint;

		MC_fsWrite(fileID, (FileContent + writePoint), writeLength);
		writePoint += writeLength;

		if(writePoint >= FileSize)
			break;
	}

    MC_fsClose(fileID);
}

void dspLoadFile( M_Byte* FileName, M_Byte* FileContent, M_Int32 FileSize )
{
	M_Int32 fileID = 0;
	M_Int32 readPoint = 0;
	M_Int32 readLength = 0;
	
	fileID = MC_fsOpen(FileName, MC_FILE_OPEN_RDONLY, MC_DIR_PRIVATE_ACCESS);

	while(1) {
		if(FileSize - readPoint >= 256)
			readLength = 256;
		else
			readLength = FileSize - readPoint;

		MC_fsRead(fileID, (FileContent + readPoint), readLength);
		readPoint += readLength;

		if(readPoint == FileSize) break;
	}

    MC_fsClose(fileID);
}


M_Uint32 dspGetFileSize( M_Byte* FileName )
{
	MC_FileInfo fi;	
	memset( &fi, 0, sizeof(MC_FileInfo) );	
	MC_fsFileAttribute(FileName, &fi, MC_DIR_PRIVATE_ACCESS);
	return fi.size;
}