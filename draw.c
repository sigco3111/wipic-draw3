#include	"draw.h"
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

#define ChannelBlend_Normal(B,L)      ((M_Uint8)(B))
#define ChannelBlend_Lighten(B,L)     ((M_Uint8)((L > B) ? L:B))
#define ChannelBlend_Darken(B,L)      ((M_Uint8)((L > B) ? B:L))
#define ChannelBlend_Multiply(B,L)    ((M_Uint8)((B * L) >> 8))
#define ChannelBlend_Average(B,L)     ((M_Uint8)((B + L) >> 1))
#define ChannelBlend_Add(B,L)         ((M_Uint8)(umin(255, (B + L))))
#define ChannelBlend_Subtract(B,L)    ((M_Uint8)((B + L < 255) ? 0:(B + L - 255)))
#define ChannelBlend_Difference(B,L)  ((M_Uint8)(abs(B - L)))
#define ChannelBlend_Negation(B,L)    ((M_Uint8)(255 - abs(255 - B - L)))
#define ChannelBlend_Screen(B,L)      ((M_Uint8)(255 - (((255 - B) * (255 - L)) >> 8)))
#define ChannelBlend_Exclusion(B,L)   ((M_Uint8)(B + L - (((B * L)<<1) >> 8)))
#define ChannelBlend_Overlay(B,L)     ((M_Uint8)((L < 128) ? (((B * L)<<1) >> 8):(255 - ((((255 - B) * (255 - L))<<1) >> 8))))
#define ChannelBlend_SoftLight(B,L)   ((M_Uint8)((L < 128) ? ((((B >> 1)<<1) + 64)) * (L >> 8): \
										(255 - (((((255 - ((B >> 1) + 64))<<1) * (255 - L)) >> 8)))))
#define ChannelBlend_HardLight(B,L)   (ChannelBlend_Overlay(L,B))
#define ChannelBlend_ColorDodge(B,L)  ((M_Uint8)((B == 255) ? B:umin(255, ((L << 8 ) / (255 - B)))))
#define ChannelBlend_ColorBurn(B,L)   ((M_Uint8)((B == 0) ? B:umax(0, (255 - ((255 - L) << 8 ) / B))))
#define ChannelBlend_LinearDodge(B,L) (ChannelBlend_Add(B,L))
#define ChannelBlend_LinearBurn(B,L)  (ChannelBlend_Subtract(B,L))
#define ChannelBlend_LinearLight(B,L) ((M_Uint8)(B < 128)? ChannelBlend_LinearBurn((B<<1),L):ChannelBlend_LinearDodge(((B - 128)<<1),L))
#define ChannelBlend_VividLight(B,L)  ((M_Uint8)(B < 128)? ChannelBlend_ColorBurn((B<<1),L):ChannelBlend_ColorDodge(((B - 128)<<1),L))
#define ChannelBlend_PinLight(B,L)    ((M_Uint8)(B < 128)? ChannelBlend_Darken((B<<1),L):ChannelBlend_Lighten(((B - 128)<<1),L))
#define ChannelBlend_HardMix(B,L)     ((M_Uint8)((ChannelBlend_VividLight(B,L) < 128) ? 0:255))
#define ChannelBlend_Reflect(B,L)     ((M_Uint8)((L == 255) ? L:umin(255, (B * B / (255 - L)))))
#define ChannelBlend_Glow(B,L)        (ChannelBlend_Reflect(L,B))
#define ChannelBlend_Phoenix(B,L)     ((M_Uint8)(umin(B,L) - umax(B,L) + 255))
#define ChannelBlend_Alpha(B,L,O)     ((M_Uint8)(O * B + (1 - O) * L))
#define ChannelBlend_AlphaF(B,L,F,O)  (ChannelBlend_Alpha(F(B,L),B,O))

//=============================================================================================

#define ColorBlend_Buffer(T,A,B,M)      (T)[0] = ChannelBlend_##M((A)[0], (B)[0]), \
										(T)[1] = ChannelBlend_##M((A)[1], (B)[1]), \
										(T)[2] = ChannelBlend_##M((A)[2], (B)[2])
#define ColorBlend_Normal(T,A,B)        (ColorBlend_Buffer(T,A,B,Normal))
#define ColorBlend_Lighten(T,A,B)       (ColorBlend_Buffer(T,A,B,Lighten))
#define ColorBlend_Darken(T,A,B)        (ColorBlend_Buffer(T,A,B,Darken))
#define ColorBlend_Multiply(T,A,B)      (ColorBlend_Buffer(T,A,B,Multiply))
#define ColorBlend_Average(T,A,B)       (ColorBlend_Buffer(T,A,B,Average))
#define ColorBlend_Add(T,A,B)           (ColorBlend_Buffer(T,A,B,Add))
#define ColorBlend_Subtract(T,A,B)      (ColorBlend_Buffer(T,A,B,Subtract))
#define ColorBlend_Difference(T,A,B)    (ColorBlend_Buffer(T,A,B,Difference))
#define ColorBlend_Negation(T,A,B)      (ColorBlend_Buffer(T,A,B,Negation))
#define ColorBlend_Screen(T,A,B)        (ColorBlend_Buffer(T,A,B,Screen))
#define ColorBlend_Exclusion(T,A,B)     (ColorBlend_Buffer(T,A,B,Exclusion))
#define ColorBlend_Overlay(T,A,B)       (ColorBlend_Buffer(T,A,B,Overlay))
#define ColorBlend_SoftLight(T,A,B)     (ColorBlend_Buffer(T,A,B,SoftLight))
#define ColorBlend_HardLight(T,A,B)     (ColorBlend_Buffer(T,A,B,HardLight))
#define ColorBlend_ColorDodge(T,A,B)    (ColorBlend_Buffer(T,A,B,ColorDodge))
#define ColorBlend_ColorBurn(T,A,B)     (ColorBlend_Buffer(T,A,B,ColorBurn))
#define ColorBlend_LinearDodge(T,A,B)   (ColorBlend_Buffer(T,A,B,LinearDodge))
#define ColorBlend_LinearBurn(T,A,B)    (ColorBlend_Buffer(T,A,B,LinearBurn))
#define ColorBlend_LinearLight(T,A,B)   (ColorBlend_Buffer(T,A,B,LinearLight))
#define ColorBlend_VividLight(T,A,B)    (ColorBlend_Buffer(T,A,B,VividLight))
#define ColorBlend_PinLight(T,A,B)      (ColorBlend_Buffer(T,A,B,PinLight))
#define ColorBlend_HardMix(T,A,B)       (ColorBlend_Buffer(T,A,B,HardMix))
#define ColorBlend_Reflect(T,A,B)       (ColorBlend_Buffer(T,A,B,Reflect))
#define ColorBlend_Glow(T,A,B)          (ColorBlend_Buffer(T,A,B,Glow))
#define ColorBlend_Phoenix(T,A,B)       (ColorBlend_Buffer(T,A,B,Phoenix))

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

M_Int32 HCENTER = 1;	/// 0000 0000 0000 0001
M_Int32 VCENTER = 2;	/// 0000 0000 0000 0010

M_Int32 LEFT = 4;	/// 0000 0000 0000 0100
M_Int32 RIGHT = 8;	/// 0000 0000 0000 1000

M_Int32 TOP = 16;	/// 0000 0000 0001 0000
M_Int32 BOTTOM = 32;	/// 0000 0000 0010 0000

M_Int32 COLOR_WHITE = 0xFFFFFF;
M_Int32 COLOR_TRANSPARENT = 0xFF00FF;

M_Uint32 COLOR_TRANS;


M_Int32 sR, sG, sB; ///< 소스
M_Int32 tR, tG, tB; ///< 대상 프레임버퍼
M_Int32 rR, rG, rB; ///< 알파블랜딩 결과

M_Int32 RGB1[3], RGB2[3], RGB3[3];

static int i, j, k, l;

MC_GrpContext* m_gc;
//M_Int16 m_sw, m_sh;

M_Int32  m_resID[1000];
M_Int32  m_size[1000];

//===============================================
static MC_GrpFrameBuffer* m_mainFrameBuffer;
static MC_GrpFrameBuffer* m_offScreenBuffer;	

static M_Uint16* m_pBackFB_16;
//static M_Uint16* m_pBackFB_32;
static M_Uint32* m_pBackFB_32;

static M_Int32 m_BPP;
static M_Int32 m_BPL;

static M_Int32 m_LCD_WIDTH, m_LCD_HEIGHT, m_LCD_WIDTH_M1, m_LCD_HEIGHT_M1, m_LCD_WIDTH_C, m_LCD_HEIGHT_C;

M_Int32 m_alpha;

STREX	lStr;
M_Int32 m_fntID;	//폰트의 설정이 저장되는 변수


#if LGT_APP != 1
#define MC_GRP_GET_FRAME_BUFFER_POINTER(a) ((M_Int32*)MC_GETDPTR(((struct _MC_GrpFrameBuffer*)MC_GETDPTR(a))->frameBufID)) 
#endif

/*---------------------------------------------------------------------------*
Description :	GC를 리턴한다.
Arguments :		void
Returns :		m_gc
*---------------------------------------------------------------------------*/
MC_GrpContext* GetGC( void )
{
	return m_gc;
}

/*---------------------------------------------------------------------------*
Description :	투명 컬러키값을 설정한다.
Arguments :		v			- 컬러키 값
Returns :		void
*---------------------------------------------------------------------------*/
void SetTrans( MC_GrpImage img )
{
	//알파블렌딩 및 픽셀 연산 할 이미지의 컬러키는 255,0,255로 설정한다. (PNG의 투명값을 흰색으로 인식해버린다.)
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	if(m_BPP == 32)
	{
		pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
		COLOR_TRANS = (M_Uint32)(*(pFB_32+0+0)); 
	}
	else
	{
		pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
		COLOR_TRANS = *(pFB_16+0+0); 
	}
}

/*---------------------------------------------------------------------------*
Description :	드로우엔진에서 필요한 값을 세팅한다. (드로우엔진 사용전에 꼭 한번은 호출해야한다.)
Arguments :		gc					- 게임에서 사용한 MC_GrpContext의 포인터
				LCD_WIDTH			- 스크린 가로크기
				LCD_HEIGHT			- 스크린 세로크기
				mainFrameBuffer		- 메인스크린 프레임 버퍼 포인터
				offScreenBuffer		- 오프스크린 프레임 버퍼 포인터
Returns :		void
*---------------------------------------------------------------------------*/
void SetDrawEng(MC_GrpContext *gc, M_Int32 LCD_WIDTH, M_Int32 LCD_HEIGHT, MC_GrpFrameBuffer* mainFrameBuffer, MC_GrpFrameBuffer* offScreenBuffer)
{
	MC_GrpFrameBuffer dst;
	MC_GrpFrameBuffer dstBuffer;

	MC_GrpDisplayInfo m_DisInfo;
	MC_grpGetDisplayInfo(0, &m_DisInfo);
	
	m_gc = gc;
	m_LCD_WIDTH = LCD_WIDTH;
	m_LCD_HEIGHT = LCD_HEIGHT;
	
	m_LCD_WIDTH_C = m_LCD_WIDTH >> 1;
	m_LCD_HEIGHT_C = m_LCD_HEIGHT >> 1;
	
	m_LCD_WIDTH_M1 = m_LCD_WIDTH - 1;
	m_LCD_HEIGHT_M1 = m_LCD_HEIGHT - 1;
	
	m_mainFrameBuffer = mainFrameBuffer;
	m_offScreenBuffer = offScreenBuffer;
			
	m_BPP = MC_GRP_GET_FRAME_BUFFER_BPP( *m_mainFrameBuffer );
	m_BPL = MC_GRP_GET_FRAME_BUFFER_BPL( *m_mainFrameBuffer );

	if(m_BPP == 32)		m_pBackFB_32 = ((M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( *m_offScreenBuffer ));
	else				m_pBackFB_16 = ((M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( *m_offScreenBuffer ));
	
	MC_grpSetContext(m_gc, MC_GRP_CONTEXT_FONT_IDX, (void*)MC_grpGetFont(MC_GRP_FT_FACE_SYSTEM, MC_GRP_FT_SIZE_SMALL, MC_GRP_FT_STYLE_PLAIN));
}

/*---------------------------------------------------------------------------*
Description :	이미지를 중앙에 출력하기 위한 x좌표값
Arguments :		v	- 이미지의 가로 크기
Returns :		int - x좌표
*---------------------------------------------------------------------------*/
int GetCenterX(int v)
{
	//return ((m_LCD_WIDTH/2) - (265/2));
	return (m_LCD_WIDTH-v)>>1;
}

/*---------------------------------------------------------------------------*
Description :	이미지를 중앙에 출력하기 위한 y좌표값
Arguments :		v	- 이미지의 세로 크기
Returns :		int - y좌표
*---------------------------------------------------------------------------*/
int GetCenterY(int v)
{
	//return ((m_LCD_HEIGHT/2) - (265/2));
	return (m_LCD_HEIGHT-v)>>1;
}

/*---------------------------------------------------------------------------*
Description :	이미지를 로드한다.
Arguments :		image		- 불러온 이미지를 담아둘 이미지데이터의 포인터
				parm		- 불러올 이미지의 경로
Returns :		void
*---------------------------------------------------------------------------*/
void LoadImage(MC_GrpImage* image, char* path, M_Int32 idx)
{
	M_Uint32 mBufID;
	M_Int32  ret;
	
	//이미지 파일의 리소스 ID를 얻어온다.
#if SKT_APP == 1
	if(m_resID[idx] == 0)	m_resID[idx] = MC_knlGetResourceID(path, &m_size[idx]);
#endif
#if KTF_APP == 1
	MC_knlSprintk(g_TempStr2, "%s.png", path);
	if(m_resID[idx] == 0)	m_resID[idx] = MC_knlGetResourceID(g_TempStr2, &m_size[idx]);
#endif
#if LGT_APP == 1
	MC_knlSprintk(g_TempStr2, "res/%s.png", path);
	if(m_resID[idx] == 0)	m_resID[idx] = MC_knlGetResourceID(g_TempStr2, &m_size[idx]);
#endif
	
	//이미지 파일의 크기만큼 메모리를 할당한다.
	mBufID = MC_knlCalloc(m_size[idx]);
	MC_knlGetResource(m_resID[idx], (void*)mBufID, m_size[idx]);

	//이미지를 생성한다.
	ret = MC_grpCreateImage(image, mBufID, 0, m_size[idx]);
}

/*---------------------------------------------------------------------------*
Description :	파일명에서 인덱스를 얻어와 반환한다.
Arguments :		path		- 파일 경로
Returns :		void
*---------------------------------------------------------------------------*/
int imgres_idget(char* path)
{
	int i=0, ind_count=0;
	int flag=0, re_val=0;
	int c_count=0;
	char filenumberc[4];
		

	for(i = 0; i <50; i++)
	{
		if(path[i] == ']')
		{
			flag = 2;
			break;
		}
		
		if(flag == 1)
		{
			filenumberc[ind_count] = path[i];
			ind_count++;
		}
		
		if(path[i] == '[') 
		{
			flag = 1;
		}
	}
	
	if(flag == 2)
	{
		re_val = atoi(filenumberc);
		
		return re_val;
	}
	else
	{
		//		MC_knlPrintk("get res id fail! \n");	
		return 9999;
	}
}

/*---------------------------------------------------------------------------*
Description :	클리퍼를 반환한다.
Arguments :		img			- 클리퍼를 만들 이미지
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				w			- 이미지의 가로 크기
				h			- 이미지의 세로 크기
				rt			- 클리퍼
Returns :		0이 아닐 경우는 화면밖으로 완전히 나갔을때
*---------------------------------------------------------------------------*/
M_Int8 DDCliper( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32 w, M_Int32 h, DClip* rt)
{
	M_Int16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Int16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	rt->sh = ih;
	rt->sw = iw;
	rt->sx = 0;
	rt->sy = 0;
	rt->dx = x;
	rt->dy = y;
	
	if( x < 0 )
	{
		rt->sx = -x;			
		rt->dx = 0;
		
		if (rt->sw > w)	rt->sw = w + x;
		else			rt->sw = iw + x;
		
		if( rt->sw > m_LCD_WIDTH ) ///< 이미지소스가 LCD폭보다 더 크다면
		{
			rt->sw = m_LCD_WIDTH; ///< 이미지소스 참조는 실제 화면에 그려지는 LCD_WIDTH 만큼만 참조한다
		}
	}
	///< 이미지가 LCD 오른쪽에 걸릴경우
	else if( x + iw > m_LCD_WIDTH ) 
	{
		rt->sw = m_LCD_WIDTH - x;
	}
	
	
	///< 이미지가 위쪽에 걸릴경우
	if( y < 0 )
	{
		rt->sy = -y;
		rt->dy = 0;
		
		if (rt->sh > h)	rt->sh = h + y;
		else			rt->sh = ih + y;
		
		if( rt->sh > m_LCD_HEIGHT )
		{
			rt->sh = m_LCD_HEIGHT; ///< 이미지소스 높이가 화면크기보다 크면 실제 화면에 그릴수 있는 LCD_HEIGHT만큼만 참조한다.
		}
	}
	///< 이미지가 아래쪽에 걸릴경우
	else if( y + ih > m_LCD_HEIGHT )
	{			
		rt->sh = m_LCD_HEIGHT - y;			
	}

	//이미지가 화면상에서 완전히 나갔을 때 출력 루틴 종료
	if ( y < -ih)			return 1;
	if ( y > m_LCD_HEIGHT)	return 1;
	if ( x < -iw)			return 1;
	if ( x > m_LCD_WIDTH)	return 1;

	return 0;
}

/*---------------------------------------------------------------------------*
Description :	이미지를 출력한다.
Arguments :		src			- 출력할 이미지데이터
				dx			- 스크린상의 출력할 위치 (x좌표)
				dy			- 스크린상의 출력할 위치 (y좌표)
				w			- 이미지의 가로 크기
				h			- 이미지의 세로 크기
Returns :		void
*---------------------------------------------------------------------------*/
void DrawImage( MC_GrpImage img, M_Int32 x, M_Int32 y)
{
	M_Int32 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT);
	M_Int32 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);

	M_Int32 sw = iw, sh = ih, sx = 0, sy = 0;

	//KTF 전용 클리퍼
	//화면밖으로 나갔을 때

	///< 이미지가 LCD 왼쪽 경계에 걸릴경우
	if( x < 0 )
	{
		sx = -x;
				
		sw = iw + x;
		if( sw > m_LCD_WIDTH ) ///< 이미지소스가 LCD폭보다 더 크다면
		{
			sw = m_LCD_WIDTH; ///< 이미지소스 참조는 실제 화면에 그려지는 LCD_WIDTH 만큼만 참조한다
		}
		x = 0;
	}
	///< 이미지가 LCD 오른쪽에 걸릴경우
	else if( x + iw > m_LCD_WIDTH ) 
	{
		sw = m_LCD_WIDTH - x;
	}
	
	
	///< 이미지가 위쪽에 걸릴경우
	if( y < 0 )
	{
		sy = -y;
		
		sh = ih + y;
		if( sh > m_LCD_HEIGHT )
		{
			sh = m_LCD_HEIGHT; ///< 이미지소스 높이가 화면크기보다 크면 실제 화면에 그릴수 있는 LCD_HEIGHT만큼만 참조한다.
		}
		y = 0;
	}
	///< 이미지가 아래쪽에 걸릴경우
	else if( y + ih > m_LCD_HEIGHT )
	{			
		sh = m_LCD_HEIGHT - y;			
	}

	MC_grpDrawImage(*m_offScreenBuffer, x, y, sw, sh, img, sx, sy, m_gc);
}

/*---------------------------------------------------------------------------*
Description :	이미지에서 필요한 부분만 잘라서 출력할때 사용
Arguments :		src			- 출력할 이미지데이터
				dx			- 스크린상의 출력할 위치 (x좌표)
				dy			- 스크린상의 출력할 위치 (y좌표)
				w			- 이미지의 가로 크기
				h			- 이미지의 세로 크기
				sx			- 이미지메모리상의 위치 (x좌표 출력시작점)
				sy			- 이미지메모리상의 위치 (y좌표 출력시작점)
Returns :		void
*---------------------------------------------------------------------------*/
void DrawImageEx( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32 w, M_Int32 h, M_Int32 dx, M_Int32 dy )
{	
	M_Int32 sw = w, sh = h, sx = dx, sy = dy;
	
	//KTF 전용 클리퍼
	//화면밖으로 나갔을 때
	
	///< 이미지가 LCD 왼쪽 경계에 걸릴경우
	if( x < 0 )
	{
		sx = -x+dx;
		
		sw = w + x;
		if( sw > m_LCD_WIDTH ) ///< 이미지소스가 LCD폭보다 더 크다면
		{
			sw = m_LCD_WIDTH; ///< 이미지소스 참조는 실제 화면에 그려지는 LCD_WIDTH 만큼만 참조한다
		}
		x = 0;
	}
	///< 이미지가 LCD 오른쪽에 걸릴경우
	else if( x + w > m_LCD_WIDTH ) 
	{
		sw = m_LCD_WIDTH - x;
	}
	
	
	///< 이미지가 위쪽에 걸릴경우
	if( y < 0 )
	{
		sy = -y+dy;
		
		sh = h + y;
		if( sh > m_LCD_HEIGHT )
		{
			sh = m_LCD_HEIGHT; ///< 이미지소스 높이가 화면크기보다 크면 실제 화면에 그릴수 있는 LCD_HEIGHT만큼만 참조한다.
		}
		y = 0;
	}
	///< 이미지가 아래쪽에 걸릴경우
	else if( y + h > m_LCD_HEIGHT )
	{			
		sh = m_LCD_HEIGHT - y;			
	}

	MC_grpDrawImage(*m_offScreenBuffer, x, y, sw, sh, img, sx, sy, m_gc);
}


/*---------------------------------------------------------------------------*
Description :	텍스트 출력
Arguments :		x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				str			- 출력할 문자열
				color		- 문자의 색 (ex : (void*)MC_grpGetPixelFromRGB(0,0,255))
Returns :		void
*---------------------------------------------------------------------------*/
void DrawString( M_Int32 x, M_Int32 y, const char* str, void* color )
{
	MC_grpSetContext( m_gc, MC_GRP_CONTEXT_FG_PIXEL_IDX, color ); // set blue color

#if LGT_APP == 1
	MC_grpDrawString( *m_offScreenBuffer, x, y-10, str, -1, m_gc );
#else
	MC_grpDrawString( *m_offScreenBuffer, x, y, str, -1, m_gc );
#endif
}


/*---------------------------------------------------------------------------*
Description :	DrawFillRect
Arguments :		x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				w			- 가로 크기
				h			- 세로 크기
				color		- 문자의 색 (ex : (void*)MC_grpGetPixelFromRGB(0,0,255))
Returns :		void
*---------------------------------------------------------------------------*/
void DFillRect( M_Int32 x, M_Int32 y, M_Int32 w, M_Int32 h, void* color )
{
	MC_grpSetContext( m_gc, MC_GRP_CONTEXT_FG_PIXEL_IDX, color ); // set blue color
	MC_grpFillRect(*m_offScreenBuffer, x, y, w, h, m_gc );
}

/*---------------------------------------------------------------------------*
Description :	DDrawRect
Arguments :		x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				w			- 가로 크기
				h			- 세로 크기
				color		- 문자의 색 (ex : (void*)MC_grpGetPixelFromRGB(0,0,255))
Returns :		void
*---------------------------------------------------------------------------*/
void DDrawRect( M_Int32 x, M_Int32 y, M_Int32 w, M_Int32 h, void* color )
{
	MC_grpSetContext( m_gc, MC_GRP_CONTEXT_FG_PIXEL_IDX, color ); // set blue color
	MC_grpDrawRect(*m_offScreenBuffer, x, y, w, h, m_gc );
}

/*---------------------------------------------------------------------------*
Description :	원그리기
Arguments :		x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				w			- 가로 크기
				h			- 세로 크기
				color		- 문자의 색 (ex : (void*)MC_grpGetPixelFromRGB(0,0,255))
Returns :		void
*---------------------------------------------------------------------------*/
void DFillArc( M_Int32 x, M_Int32 y, M_Int32 w, M_Int32 h, M_Int32 s, M_Int32 e, void* color )
{
	MC_grpSetContext( m_gc, MC_GRP_CONTEXT_FG_PIXEL_IDX, color ); // set blue color
	MC_grpFillArc(*m_offScreenBuffer, x, y, w, h, s, e, m_gc) ;
}

/*---------------------------------------------------------------------------*
Description :	원그리기
Arguments :		x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				w			- 가로 크기
				h			- 세로 크기
				color		- 문자의 색 (ex : (void*)MC_grpGetPixelFromRGB(0,0,255))
Returns :		void
*---------------------------------------------------------------------------*/
void DDrawArc( M_Int32 x, M_Int32 y, M_Int32 w, M_Int32 h, M_Int32 s, M_Int32 e, void* color )
{
	MC_grpSetContext( m_gc, MC_GRP_CONTEXT_FG_PIXEL_IDX, color ); // set blue color
	MC_grpDrawArc(*m_offScreenBuffer, x, y, w, h, s, e, m_gc);
}

/*---------------------------------------------------------------------------*
Description :	선그리기
Arguments :		x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				w			- 가로 크기
				h			- 세로 크기
				color		- 문자의 색 (ex : (void*)MC_grpGetPixelFromRGB(0,0,255))
Returns :		void
*---------------------------------------------------------------------------*/
void DDrawLine( M_Int32 x1, M_Int32 y1, M_Int32 x2, M_Int32 y2, void* color )
{
	MC_grpSetContext( m_gc, MC_GRP_CONTEXT_FG_PIXEL_IDX, color ); // set blue color
	MC_grpDrawLine(*m_offScreenBuffer, x1, y1, x2, y2, m_gc);
}

/*---------------------------------------------------------------------------*
Description :	로드된 이미지 데이터를 메모리에서 지운다.
Arguments :		image		- 릴리즈시킬 이미지
Returns :		void
*---------------------------------------------------------------------------*/
void ReleseImage(MC_GrpImage* image)
{
	if( *image != NULL)
	{
		MC_grpDestroyImage( *image );
		*image = NULL;
	}

	MC_knlGetFreeMemory();

	if(m_BPP == 32)		m_pBackFB_32 = ((M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( *m_offScreenBuffer ));
	else				m_pBackFB_16 = ((M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( *m_offScreenBuffer ));
}

/*---------------------------------------------------------------------------*
Description :	알파값을 설정한다. (DDrawImage 함수에서만 적용)
Arguments :		alpha	- 알파값 0 ~ 255
Returns :		void
*---------------------------------------------------------------------------*/
void SetAlpha( M_Int32 alpha )
{	
	if( alpha > 255 ) { alpha = 255; }
	else if( alpha < 0 ) { alpha = 0; }
	m_alpha = alpha;
}

/*---------------------------------------------------------------------------*
Description :	프레임버퍼에 직접 이미지를 출력한다.
Arguments :		src			- 출력할 이미지데이터
				dx			- 스크린상의 출력할 위치 (x좌표)
				dy			- 스크린상의 출력할 위치 (y좌표)
Returns :		void
*---------------------------------------------------------------------------*/
void DDrawImage( MC_GrpImage img, M_Int32 x, M_Int32 y)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;

	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);

	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	M_Int32 sx = 0, sy = 0, dx = x, dy = y;

	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;

	M_Int32 offsetY, ry;
	M_Int32 row, col;


	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );

	//이미지가 화면상에서 완전히 나갔을 때 출력 루틴 종료
	if ( y < -ih)			return;
	if ( y > m_LCD_HEIGHT)	return;
	if ( x < -iw)			return;
	if ( x > m_LCD_WIDTH)	return;

	///< 이미지가 LCD 왼쪽 경계에 걸릴경우
	if( x < 0 )
	{
		sx = -x;			
		dx = 0;

		sw = iw + x;
		if( sw > m_LCD_WIDTH ) ///< 이미지소스가 LCD폭보다 더 크다면
		{
			sw = m_LCD_WIDTH; ///< 이미지소스 참조는 실제 화면에 그려지는 LCD_WIDTH 만큼만 참조한다
		}
	}
	///< 이미지가 LCD 오른쪽에 걸릴경우
	else if( x + iw > m_LCD_WIDTH ) 
	{
		sw = m_LCD_WIDTH - x;
	}


	///< 이미지가 위쪽에 걸릴경우
	if( y < 0 )
	{
		sy = -y;
		dy = 0;

		sh = ih + y;
		if( sh > m_LCD_HEIGHT )
		{
			sh = m_LCD_HEIGHT; ///< 이미지소스 높이가 화면크기보다 크면 실제 화면에 그릴수 있는 LCD_HEIGHT만큼만 참조한다.
		}
	}
	///< 이미지가 아래쪽에 걸릴경우
	else if( y + ih > m_LCD_HEIGHT )
	{			
		sh = m_LCD_HEIGHT - y;			
	}

	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + dx + m_LCD_WIDTH*dy; ///< 프레임버퍼에 그릴 시작위치

		if (m_alpha == 0)
		{
		}
		else if (m_alpha == HALF_ALPHA)
		{
			if( m_alpha < 1 ) return;
			
			for( row = sy; row < sy+sh; row++ )
			{
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = sx; col<sx+sw; col++ )
				{				
					pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
					
					MC_grpGetRGBFromPixel( pixel_32, &RGB1[0], &RGB1[1], &RGB1[2] );
					MC_grpGetRGBFromPixel( *( offset_32 + (col-sx) + offsetY ), &RGB2[0], &RGB2[1], &RGB2[2] );				

					ColorBlend_Average(RGB3, RGB1, RGB2);
					
					*( offset_32 + (col-sx) + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
				}
			}
		}
		else if( m_alpha < 255 )
		{
			if( m_alpha < 1 ) return;
						
			for( row = sy; row < sy+sh; row++ )
			{
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = sx; col<sx+sw; col++ )
				{				
					pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
					
					MC_grpGetRGBFromPixel( pixel_32, &sR, &sG, &sB );
					MC_grpGetRGBFromPixel( *( offset_32 + (col-sx) + offsetY ), &tR, &tG, &tB );				
					
					rR = ( sR*m_alpha + tR*(255-m_alpha) )>>8;
					rG = ( sG*m_alpha + tG*(255-m_alpha) )>>8;
					rB = ( sB*m_alpha + tB*(255-m_alpha) )>>8;
					
					*( offset_32 + (col-sx) + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
				}
			}
		}
		else
		{
			for( row = sy; row < sy+sh; row++ )
			{
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = sx; col < sx+sw; col++ )
				{				
					pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					*( offset_32 +  (col-sx) + offsetY ) = pixel_32;
				}
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + dx + m_LCD_WIDTH*dy; ///< 프레임버퍼에 그릴 시작위치

		if (m_alpha == 0)
		{
		}
		else if (m_alpha == HALF_ALPHA)
		{
			if( m_alpha < 1 ) return;
			
			for( row = sy; row < sy+sh; row++ )
			{
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = sx; col<sx+sw; col++ )
				{				
					pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
					
					MC_grpGetRGBFromPixel( pixel_16, &RGB1[0], &RGB1[1], &RGB1[2] );
					MC_grpGetRGBFromPixel( *( offset_16 + (col-sx) + offsetY ), &RGB2[0], &RGB2[1], &RGB2[2] );				
					
					ColorBlend_Average(RGB3, RGB1, RGB2);
					
					*( offset_16 + (col-sx) + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
				}
			}
		}
		else if( m_alpha < 255 )
		{
			if( m_alpha < 1 ) return;
						
			for( row = sy; row < sy+sh; row++ )
			{
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = sx; col<sx+sw; col++ )
				{				
					pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
					
					MC_grpGetRGBFromPixel( pixel_16, &sR, &sG, &sB );
					MC_grpGetRGBFromPixel( *( offset_16 + (col-sx) + offsetY ), &tR, &tG, &tB );				
					
					rR = ( sR*m_alpha + tR*(255-m_alpha) )>>8;
					rG = ( sG*m_alpha + tG*(255-m_alpha) )>>8;
					rB = ( sB*m_alpha + tB*(255-m_alpha) )>>8;
					
					*( offset_16 +  (col-sx) + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
				}
			}
		}
		else
		{
			for( row = sy; row < sy+sh; row++ )
			{
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = sx; col < sx+sw; col++ )
				{				
					pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					*( offset_16 +  (col-sx) + offsetY ) = pixel_16;
				}
			}
		}
	}
}


/*---------------------------------------------------------------------------*
Description :	이미지 확대/축소
Arguments :		img			- 출력할 이미지데이터
x			- 스크린상의 출력할 위치 (x좌표)
y			- 스크린상의 출력할 위치 (y좌표)
nw			- 확대/축소 될 가로사이즈
nh			- 확대/축소 될 세로사이즈
Returns :		void
*---------------------------------------------------------------------------*/
void DDrawImageReSize( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32 nw, M_Int32 nh )
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;

	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
    
	M_Int32 srcW, srcH, rx, ry;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY;
	M_Int32 row, col, px, py;
	
	
	srcW = iw;
	srcH = ih;
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );

	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 주어진 xy 좌표에 의한 프레임버퍼에 그릴 시작위치
		
		if (m_alpha == 0)
		{
		}
		else if (m_alpha == HALF_ALPHA)
		{
			if( m_alpha < 1 ) return;
			
			for( row=0; row<nh; row++ )
			{		
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;
				
				ry = (srcH*row/nh)*iw; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col=0; col<nw; col++ )
				{		
					px = x+col;			
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
					
					rx = srcW*col/nw; ///< 참조할 이미지소스의 x좌표
					pixel_32 = *( pFB_32  + rx + ry ); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					
					MC_grpGetRGBFromPixel( pixel_32, &RGB1[0], &RGB1[1], &RGB1[2] );
					MC_grpGetRGBFromPixel( *( offset_32 + col + offsetY ), &RGB2[0], &RGB2[1], &RGB2[2] );				
					
					ColorBlend_Average(RGB3, RGB1, RGB2);
					
					*( offset_32+col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
				}
			}
		}
		else if( m_alpha < 255 )
		{
			if( m_alpha < 1 ) return;
			
			for( row=0; row<nh; row++ )
			{		
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;
				
				ry = (srcH*row/nh)*iw; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col=0; col<nw; col++ )
				{		
					px = x+col;			
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
					
					rx = srcW*col/nw; ///< 참조할 이미지소스의 x좌표
					pixel_32 = *( pFB_32  + rx + ry ); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					
					
					MC_grpGetRGBFromPixel( pixel_32, &sR, &sG, &sB );
					MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &tR, &tG, &tB );				
					
					rR = ( sR*m_alpha + tR*(255-m_alpha) )>>8;
					rG = ( sG*m_alpha + tG*(255-m_alpha) )>>8;
					rB = ( sB*m_alpha + tB*(255-m_alpha) )>>8;
					
					*( offset_32+col + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
				}
			}
		}
		else 
		{
			for( row=0; row<nh; row++ )
			{		
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;
				
				ry = (srcH*row/nh)*iw; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col=0; col<nw; col++ )
				{		
					px = x+col;			
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
					
					rx = srcW*col/nw; ///< 참조할 이미지소스의 x좌표
					pixel_32 = *( pFB_32  + rx + ry ); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					
					*( offset_32+col + offsetY ) = pixel_32;
				}
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 주어진 xy 좌표에 의한 프레임버퍼에 그릴 시작위치

		if (m_alpha == 0)
		{
		}
		else if (m_alpha == HALF_ALPHA)
		{
			if( m_alpha < 1 ) return;
			
			for( row=0; row<nh; row++ )
			{		
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;
				
				ry = (srcH*row/nh)*iw; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col=0; col<nw; col++ )
				{		
					px = x+col;			
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
					
					rx = srcW*col/nw; ///< 참조할 이미지소스의 x좌표
					pixel_16 = *( pFB_16  + rx + ry ); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					
					MC_grpGetRGBFromPixel( pixel_16, &RGB1[0], &RGB1[1], &RGB1[2] );
					MC_grpGetRGBFromPixel( *( offset_16 + col + offsetY ), &RGB2[0], &RGB2[1], &RGB2[2] );				
					
					ColorBlend_Average(RGB3, RGB1, RGB2);
					
					*( offset_16 + col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
				}
			}
		}
		else if( m_alpha < 255 )
		{
			if( m_alpha < 1 ) return;
			
			for( row=0; row<nh; row++ )
			{		
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;
				
				ry = (srcH*row/nh)*iw; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col=0; col<nw; col++ )
				{		
					px = x+col;			
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
					
					rx = srcW*col/nw; ///< 참조할 이미지소스의 x좌표
					pixel_16 = *( pFB_16  + rx + ry ); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					
					
					MC_grpGetRGBFromPixel( pixel_16, &sR, &sG, &sB );
					MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &tR, &tG, &tB );				
					
					rR = ( sR*m_alpha + tR*(255-m_alpha) )>>8;
					rG = ( sG*m_alpha + tG*(255-m_alpha) )>>8;
					rB = ( sB*m_alpha + tB*(255-m_alpha) )>>8;
					
					*( offset_16+col + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
				}
			}
		}
		else 
		{
			for( row=0; row<nh; row++ )
			{		
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;
				
				ry = (srcH*row/nh)*iw; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col=0; col<nw; col++ )
				{		
					px = x+col;			
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
					
					rx = srcW*col/nw; ///< 참조할 이미지소스의 x좌표
					pixel_16 = *( pFB_16  + rx + ry ); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					
					*( offset_16 + col + offsetY ) = pixel_16;
				}
			}
		}
	}
}


/*---------------------------------------------------------------------------*
Description :	이미지 반전
Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				mode		- 0 : 일반 출력, 1:좌우 반전
				              FLIP_LR = 좌우반전
				              FLIP_UD = 상하반전
				              FLIP_LRUD = 좌우반전+상합반전
Returns :		void
*---------------------------------------------------------------------------*/
void DDrawImageFlip( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int8 mode )
{
	if (mode == 0)
	{
		DDrawImage( img, x, y);
	}
	else
	{
		M_Uint16* DCRGB_16;
		M_Uint16* SCRGB_16;
		M_Uint32* DCRGB_32;
		M_Uint32* SCRGB_32;

		M_Int32 PointSt;	//프레임버퍼 포인터의 시작점 보정
		M_Int32 indexD;
		M_Uint32 idxLT;
		M_Int32 clipL = 0, clipR = 0, clipT = 0, clipB = 0;
		int		i, j;
		M_Int16 w, h;
		MC_GrpFrameBuffer srcbuf;

		M_Int32 sx = 0, sy = 0, dx = x, dy = y;
		
		w = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
		h = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT);


		PointSt = (x>>1)+(m_LCD_WIDTH*(y>>1));
		

		//클립핑

		if(m_BPP == 32)
		{
			if (PointSt < 0)	SCRGB_32 = ((M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( *m_offScreenBuffer ));
			else				SCRGB_32 = ((M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( *m_offScreenBuffer ))+PointSt;

			if( (x % 2) == 1 ) SCRGB_32 += 1;
			if( (y % 2) == 1 ) SCRGB_32 += m_LCD_WIDTH;
			
			
			srcbuf = MC_grpGetImageFrameBuffer(img);
			DCRGB_32 = ((M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( srcbuf ));
		}
		else
		{
			if (PointSt < 0)	SCRGB_16 = ((M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( *m_offScreenBuffer ));
			else				SCRGB_16 = ((M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( *m_offScreenBuffer ))+PointSt;

			if( (x % 2) == 1 ) SCRGB_16 += 1;
			if( (y % 2) == 1 ) SCRGB_16 += m_LCD_WIDTH;
			
			
			srcbuf = MC_grpGetImageFrameBuffer(img);
			DCRGB_16 = ((M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( srcbuf ));
		}


		///////////////////////////////////////////////////////	클립퍼
		{
			//왼쪽 클립핑 (y == 0 일 경우 클립핑이 제대로 않된다.)
			if (x < 0)
			{
				clipL = abs(x);
				if (y <= 0)	PointSt = x-(clipT*m_LCD_WIDTH);	//에리어 익셉션 예외 처리
			}
			//오른쪽 클립핑
			if ((x+w) > m_LCD_WIDTH)
			{
				clipR = ((x+w) - m_LCD_WIDTH);
			}
			//위쪽 클립핑
			if (y < 0)
			{
				clipT = abs(y);
				PointSt = x-(clipT*m_LCD_WIDTH);	//에리어 익셉션 예외 처리
			}
			//아래쪽 클립핑
			if ((y+h) > m_LCD_HEIGHT)
			{
				clipB = (((y+h) - m_LCD_HEIGHT));
			}
		}
		///////////////////////////////////////////////////////

		for(i = clipT; i < h-clipB; i++)
		{
			for(j = clipL; j < w-clipR; j++)
			{
				indexD = (w-clipR-1)-j+(i*w);
				
				if(m_BPP == 32)
				{
					idxLT = PointSt+j+(i*m_LCD_WIDTH);

					if( m_alpha < 255 )
					{
						if( m_alpha < 1 ) return;
						
						if(DCRGB_32[indexD+clipR] == COLOR_TRANS )	//컬러키값이면 건너뛴다.
						{
							continue;
						}
						
						MC_grpGetRGBFromPixel( DCRGB_32[indexD+clipR], &sR, &sG, &sB );
						MC_grpGetRGBFromPixel( *(SCRGB_32+idxLT), &tR, &tG, &tB );				
						
						rR = ( sR*m_alpha + tR*(255-m_alpha) )>>8;
						rG = ( sG*m_alpha + tG*(255-m_alpha) )>>8;
						rB = ( sB*m_alpha + tB*(255-m_alpha) )>>8;
						
						*(SCRGB_32+idxLT) = MC_grpGetPixelFromRGB( rR, rG, rB );
					}
					else
					{
						if(DCRGB_32[indexD+clipR] == COLOR_TRANS )	//컬러키값이면 건너뛴다.
						{
							continue;
						}

						*(SCRGB_32+idxLT)      = DCRGB_32[indexD+clipR];
					}
				}
				else
				{
					idxLT = PointSt+j+(i*m_LCD_WIDTH);

					if( m_alpha < 255 )
					{
						if( m_alpha < 1 ) return;

						if(DCRGB_16[indexD+clipR] == COLOR_TRANS )	//컬러키값이면 건너뛴다.
						{
							continue;
						}
						
						MC_grpGetRGBFromPixel( DCRGB_16[indexD+clipR], &sR, &sG, &sB );
						MC_grpGetRGBFromPixel( *(SCRGB_16+idxLT), &tR, &tG, &tB );				
						
						rR = ( sR*m_alpha + tR*(255-m_alpha) )>>8;
						rG = ( sG*m_alpha + tG*(255-m_alpha) )>>8;
						rB = ( sB*m_alpha + tB*(255-m_alpha) )>>8;
						
						*(SCRGB_16+idxLT) = MC_grpGetPixelFromRGB( rR, rG, rB );
					}
					else
					{
						if(DCRGB_16[indexD+clipR] == COLOR_TRANS )	//컬러키값이면 건너뛴다.
						{
							continue;
						}
						*(SCRGB_16+idxLT)      = DCRGB_16[indexD+clipR];
					}
				}
			} 
		}
	}
}


/*---------------------------------------------------------------------------*
Description :	이미지 반전 확장팩 (MC_grpDrawImageRegion() 대체 함수)
Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				w			- 가로 크기
				h			- 세로 크기
				cx			- 출력할 이미지의 x좌표 시작점
				cy			- 출력할 이미지의 y좌표 시작점
				mode		-	DD_TRAN_NON			: 디폴트
								DD_TRAN_ROT90		: 90도 회전
								DD_TRAN_ROT180		: 180도 회전(상/하 반전)
								DD_TRAN_ROT270		: 270도 회전
								DD_TRAN_MIR			: 좌/우 반전
								DD_TRAN_MIR_ROT90	: 90도 회전 + 좌/우 반전
								DD_TRAN_MIR_ROT180	: 180도 회전 (상/하 + 좌/우 반전)
								DD_TRAN_MIR_ROT270	: 270도 회전 + 좌/우 반전
								0					: 원본 출력
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_FLIP_EX == 1
void DDrawImageFlipEx( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32 w, M_Int32 h, M_Int32 cx, M_Int32 cy, M_Int8 mode )
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;

	M_Int16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Int16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);

	DClip clip;

	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;

	M_Int32 offsetY, ry;
	M_Int32 row, col;

	int px, py;

	int idx = 0;


	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
			
	switch (mode)
	{
	case DD_TRAN_NON:
		if (DDCliper( img, x, y, w, h, &clip))	return;

		idx = (iw*(cy))+(cx);
		
		if (clip.sw > w)	clip.sw = w;
		if (clip.sh > h)	clip.sh = h;

		if(m_BPP == 32)
		{
			offset_32 = m_pBackFB_32 + clip.dx + m_LCD_WIDTH*clip.dy; ///< 프레임버퍼에 그릴 시작위치
			
			for( row = clip.sy; row < clip.sy+clip.sh; row++ )
			{
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*(row-clip.sy); ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = clip.sx; col < clip.sx+clip.sw; col++ )
				{				
					pixel_32 = *(pFB_32 + col + ry + idx); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
					*( offset_32 +  (col-clip.sx) + offsetY ) = pixel_32;
				}
			}
		}
		else
		{
			offset_16 = m_pBackFB_16 + clip.dx + m_LCD_WIDTH*clip.dy; ///< 프레임버퍼에 그릴 시작위치
			
			for( row = clip.sy; row < clip.sy+clip.sh; row++ )
			{
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*(row-clip.sy); ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = clip.sx; col < clip.sx+clip.sw; col++ )
				{				
					pixel_16 = *(pFB_16 + col + ry + idx); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					*( offset_16 +  (col-clip.sx) + offsetY ) = pixel_16;
				}
			}
		}
		break;
	case DD_TRAN_MIR:
		if (DDCliper( img, x, y, w, h, &clip))	return;

		idx = (iw*(cy-1))+(iw-cx-1);
		
		if (clip.sw > w)	clip.sw = w;
		if (clip.sh > h)	clip.sh = h;

		if(m_BPP == 32)
		{
			offset_32 = m_pBackFB_32 + clip.dx + m_LCD_WIDTH*clip.dy; ///< 프레임버퍼에 그릴 시작위치
			
			for( row = clip.sy; row < clip.sy+clip.sh; row++ )
			{
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*(row-clip.sy); ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = clip.sx; col < clip.sx+clip.sw; col++ )
				{				
					pixel_32 = *(pFB_32 + iw-col + ry + idx); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					*( offset_32 +  (col-clip.sx) + offsetY ) = pixel_32;
				}
			}
		}
		else
		{
			offset_16 = m_pBackFB_16 + clip.dx + m_LCD_WIDTH*clip.dy; ///< 프레임버퍼에 그릴 시작위치
			
			for( row = clip.sy; row < clip.sy+clip.sh; row++ )
			{
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*(row-clip.sy); ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = clip.sx; col < clip.sx+clip.sw; col++ )
				{				
					pixel_16 = *(pFB_16 + iw-col + ry + idx); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					*( offset_16 +  (col-clip.sx) + offsetY ) = pixel_16;
				}
			}
		}
		break;
	case DD_TRAN_MIR_ROT180:
		if (DDCliper( img, x, y, w, h, &clip))	return;

		idx = (iw*(cy-1))+(cx);
		
		if (clip.sw > w)	clip.sw = w;
		if (clip.sh > h)	clip.sh = h;

		if(m_BPP == 32)
		{
			offset_32 = m_pBackFB_32 + clip.dx + m_LCD_WIDTH*clip.dy; ///< 프레임버퍼에 그릴 시작위치
			
			for( row = clip.sy; row < clip.sy+clip.sh; row++ )
			{
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*(row-clip.sy); ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = clip.sx; col < clip.sx+clip.sw; col++ )
				{				
					pixel_32 = *(pFB_32 + col + ry + idx); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
					*( offset_32 +  (col-clip.sx) + offsetY ) = pixel_32;
				}
			}
		}
		else
		{
			offset_16 = m_pBackFB_16 + clip.dx + m_LCD_WIDTH*clip.dy; ///< 프레임버퍼에 그릴 시작위치
			
			for( row = clip.sy; row < clip.sy+clip.sh; row++ )
			{
				ry = iw*(ih-row); ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*(row-clip.sy); ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = clip.sx; col < clip.sx+clip.sw; col++ )
				{				
					pixel_16 = *(pFB_16 + col + ry + idx); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					*( offset_16 +  (col-clip.sx) + offsetY ) = pixel_16;
				}
			}
		}
		break;
	case DD_TRAN_ROT270:

		//이미지가 화면상에서 완전히 나갔을 때 출력 루틴 종료
		if ( y < -iw)			return;
		if ( y > m_LCD_HEIGHT)	return;
		if ( x < -ih)			return;
		if ( x > m_LCD_WIDTH)	return;

		idx = (iw*(cy-1))+(iw-1-cx);

		if(m_BPP == 32)
		{
			offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
			
			for( row = 0; row < iw; row++ )			//세로열
			{
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;
				
				for( col = 0; col < ih; col++ )		//가로열
				{
					px = x+col;
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
					
					pixel_32 = *(pFB_32 + iw*col + (iw-row) + idx); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					
					*( offset_32 +  (m_LCD_WIDTH*row) + col ) = pixel_32;
				}
			}
		}
		else
		{
			offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
			
			for( row = 0; row < iw; row++ )			//세로열
			{
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;
								
				for( col = 0; col < ih; col++ )		//가로열
				{
					px = x+col;
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;

					pixel_16 = *(pFB_16 + iw*col + (iw-row) + idx); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			

					*( offset_16 +  (m_LCD_WIDTH*row) + col ) = pixel_16;
				}
			}
		}
		break;
	case DD_TRAN_MIR_ROT90:
		
		//이미지가 화면상에서 완전히 나갔을 때 출력 루틴 종료
		if ( y < -iw)			return;
		if ( y > m_LCD_HEIGHT)	return;
		if ( x < -ih)			return;
		if ( x > m_LCD_WIDTH)	return;

		idx = (iw*(cy-2))+(iw-cx-1);
		
		if(m_BPP == 32)
		{
			offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
			
			for( row = 0; row < iw; row++ )			//세로열
			{
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;
				
				for( col = 0; col < ih; col++ )		//가로열
				{
					px = x+col;
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
					
					pixel_32 = *(pFB_32 + iw*(ih-col) + (iw-row) + idx); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					
					*( offset_32 +  (m_LCD_WIDTH*row) + col ) = pixel_32;
				}
			}
		}
		else
		{
			offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
			
			for( row = 0; row < iw; row++ )			//세로열
			{
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;
				
				for( col = 0; col < ih; col++ )		//가로열
				{
					px = x+col;
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
					
					pixel_16 = *(pFB_16 + iw*(ih-col) + (iw-row) + idx); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					
					*( offset_16 +  (m_LCD_WIDTH*row) + col ) = pixel_16;
				}
			}
		}
		break;
	case DD_TRAN_MIR_ROT270:
		
		//이미지가 화면상에서 완전히 나갔을 때 출력 루틴 종료
		if ( y < -iw)			return;
		if ( y > m_LCD_HEIGHT)	return;
		if ( x < -ih)			return;
		if ( x > m_LCD_WIDTH)	return;

		idx = (iw*(cy))+(cx);
		
		if(m_BPP == 32)
		{
			offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
			
			for( row = 0; row < iw; row++ )			//세로열
			{
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;
				
				for( col = 0; col < ih; col++ )		//가로열
				{
					px = x+col;
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
					
					pixel_32 = *(pFB_32 + iw*(col) + (row) + idx); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					
					*( offset_32 +  (m_LCD_WIDTH*row) + col ) = pixel_32;
				}
			}
		}
		else
		{
			offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
			
			for( row = 0; row < iw; row++ )			//세로열
			{
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;
				
				for( col = 0; col < ih; col++ )		//가로열
				{
					px = x+col;
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
					
					pixel_16 = *(pFB_16 + iw*(col) + (row) + idx); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					
					*( offset_16 +  (m_LCD_WIDTH*row) + col ) = pixel_16;
				}
			}
		}
		break;
	case DD_TRAN_ROT90:
		
		//이미지가 화면상에서 완전히 나갔을 때 출력 루틴 종료
		if ( y < -iw)			return;
		if ( y > m_LCD_HEIGHT)	return;
		if ( x < -ih)			return;
		if ( x > m_LCD_WIDTH)	return;

		idx = (iw*(cy-1))+(cx);
		
		if(m_BPP == 32)
		{
			offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
			
			for( row = 0; row < iw; row++ )			//세로열
			{
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;
				
				for( col = 0; col < ih; col++ )		//가로열
				{
					px = x+col;
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
					
					pixel_32 = *(pFB_32 + iw*(ih-col) + (row) + idx); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					
					*( offset_32 +  (m_LCD_WIDTH*row) + col ) = pixel_32;
				}
			}
		}
		else
		{
			offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
			
			for( row = 0; row < iw; row++ )			//세로열
			{
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;
				
				for( col = 0; col < ih; col++ )		//가로열
				{
					px = x+col;
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
					
					pixel_16 = *(pFB_16 + iw*(ih-col) + (row) + idx); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					
					*( offset_16 +  (m_LCD_WIDTH*row) + col ) = pixel_16;
				}
			}
		}
		break;
	case DD_TRAN_ROT180:
		if (DDCliper( img, x, y, w, h, &clip))	return;
		
		idx = (iw*(cy-2))+(iw-cx-1);

		if (clip.sw > w)	clip.sw = w;
		if (clip.sh > h)	clip.sh = h;
		
		if(m_BPP == 32)
		{
			offset_32 = m_pBackFB_32 + clip.dx + m_LCD_WIDTH*clip.dy; ///< 프레임버퍼에 그릴 시작위치
			
			for( row = clip.sy; row < clip.sy+clip.sh; row++ )
			{
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*(row-clip.sy); ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = clip.sx; col < clip.sx+clip.sw; col++ )
				{				
					pixel_32 = *(pFB_32 + (iw-col) + ry + idx); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
					*( offset_32 +  (col-clip.sx) + offsetY ) = pixel_32;
				}
			}
		}
		else
		{
			offset_16 = m_pBackFB_16 + clip.dx + m_LCD_WIDTH*clip.dy; ///< 프레임버퍼에 그릴 시작위치
			
			for( row = clip.sy; row < clip.sy+clip.sh; row++ )
			{
				ry = iw*(ih-row); ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*(row-clip.sy); ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = clip.sx; col < clip.sx+clip.sw; col++ )
				{				
					pixel_16 = *(pFB_16 + (iw-col) + ry + idx); ///< 이미지소스에서 칼라값 얻어옴
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					*( offset_16 +  (col-clip.sx) + offsetY ) = pixel_16;
				}
			}
		}
		break;
	}
}
#endif


/*---------------------------------------------------------------------------*
Description :	숫자 이미지 출력
Arguments :		img		- 출력할 이미지 (0~9까지 1줄로 되어 있는 숫자이미지)
				number	- 출력할 숫자
				x		- x좌표
				y		- y좌표
				type	- 자간
				flag	- 0:좌측 정렬, 1:우측정렬
Returns :		void
*---------------------------------------------------------------------------*/
void DrawNumber(MC_GrpImage img, M_Uint32 number, int x, int y, int w, int h, int space, char flag)
{
	int			ex;
	M_Uint32	ScoreTemp = number;
	M_Uint32	tmp10;
	
	char		ScorePos_Flag, r;
	
	
	if		(ScoreTemp/10000000000 != 0)	ScorePos_Flag = 10;
	else if (ScoreTemp/1000000000 != 0)		ScorePos_Flag = 9;
	else if (ScoreTemp/100000000 != 0)		ScorePos_Flag = 8;
	else if (ScoreTemp/10000000 != 0)		ScorePos_Flag = 7;
	else if (ScoreTemp/1000000 != 0)		ScorePos_Flag = 6;
	else if (ScoreTemp/100000 != 0)			ScorePos_Flag = 5;
	else if (ScoreTemp/10000 != 0)			ScorePos_Flag = 4;
	else if (ScoreTemp/1000 != 0)			ScorePos_Flag = 3;
	else if (ScoreTemp/100 != 0)			ScorePos_Flag = 2;
	else if (ScoreTemp/10 != 0)				ScorePos_Flag = 1;
	else									ScorePos_Flag = 0;
	
	
	tmp10 = 1;
	
	for (r = 0; r < ScorePos_Flag+1; r++)
	{
		if (flag)	ex = x + ((ScorePos_Flag - r) * space);
		else		ex = x - (r * space) - w;	// 7 = 이미지 가로 크기
		
		if (r == 0)
		{
			DrawImageEx( img, ex, y, w, h, (ScoreTemp%10)*w, 0 );
		}
		else
		{
			tmp10 *= 10;
			DrawImageEx( img, ex, y, w, h, ((ScoreTemp/tmp10)%10)*w, 0 );
		}
	}
}

/*---------------------------------------------------------------------------*
Description :	문자열을 가운데 정렬로 출력한다.
Arguments :		x		- x좌표 (정중앙으로 정렬하려면 0)
				w		- 특정 구간을 기중으로 가운데 정렬할때 (정중앙으로 정렬하려면 0)
				y		- y좌표
				str		- 문자열
				r,g,b	- 출력할 문자열의 색
Returns :		void
*---------------------------------------------------------------------------*/
void DrawStringC(int x, int w, int y, char* str, M_Uint8 r, M_Uint8 g, M_Uint8 b)
{
	//폰트 아이디 저장
	m_fntID = MC_grpGetFont(MC_GRP_FT_FACE_SYSTEM, MC_GRP_FT_SIZE_SMALL, MC_GRP_FT_STYLE_PLAIN );
	MC_grpSetContext(m_gc, MC_GRP_CONTEXT_FONT_IDX, (void*)MC_grpGetFont(MC_GRP_FT_FACE_SYSTEM, MC_GRP_FT_SIZE_SMALL, MC_GRP_FT_STYLE_PLAIN));

	MC_knlSprintk(g_TempStr1, "%s", str);
	DrawString( x+GetCenterX(w+MC_grpGetStringWidth(m_fntID, g_TempStr1, -1)), y, g_TempStr1, (void*)MC_grpGetPixelFromRGB(r, g, b));
}

/*---------------------------------------------------------------------------*
Description :	폰트아이디를 반환한다.
Arguments :		void
Returns :		폰트아이디
*---------------------------------------------------------------------------*/
M_Int32 GetFontID(void)
{
	return m_fntID;
}

/*---------------------------------------------------------------------------*
Description :	화면 캡쳐
Arguments :		buff : 캡쳐할 프레임버퍼
Returns :		void
*---------------------------------------------------------------------------*/
void SaveLCD(MC_GrpFrameBuffer* buff)
{
	#if LGT_APP == 1
	{
		M_Uint16* pMainFB;
		M_Uint16* pBackFB;
		
		pMainFB = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( *buff );
		pBackFB = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( *m_offScreenBuffer );
		
		memcpy( pMainFB, pBackFB, m_LCD_WIDTH * m_LCD_HEIGHT * 2 );
	}
	#else
	MC_grpCopyFrameBuffer( *buff, 0, 0, m_LCD_WIDTH, m_LCD_HEIGHT, *m_offScreenBuffer, 0, 0, m_gc);
	#endif
}

/*---------------------------------------------------------------------------*
Description :	캡쳐화면 출력
Arguments :		buff : 캡쳐한 프레임버퍼
Returns :		void
*---------------------------------------------------------------------------*/
void RestoreLCD(MC_GrpFrameBuffer* buff)
{
	#if LGT_APP == 1
	{
		M_Uint16* pMainFB;
		M_Uint16* pBackFB;
		
		pMainFB = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( *m_offScreenBuffer );
		pBackFB = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( *buff );
		
		memcpy( pMainFB, pBackFB, m_LCD_WIDTH * m_LCD_HEIGHT * 2 );
	}
	#else
	MC_grpCopyFrameBuffer( *m_offScreenBuffer, 0, 0, m_LCD_WIDTH, m_LCD_HEIGHT, *buff, 0, 0, m_gc);
	#endif
}

/*---------------------------------------------------------------------------*
Description :	텍스트가 몇줄이나 되는지 체크한다.
				(dspDrawString()와 세트로 사용하기 위한 용도)
Arguments :		str : 그릴 문자열.
				max : 한줄에 그릴 최대 문자 수.
Returns :		텍스트의 줄수를 반환한다.
*---------------------------------------------------------------------------*/
M_Int32 dspLineCnt( const M_Char* str, M_Int32 max)
{
	M_Int32		len			= 0;
	M_Int32		seek		= 0;
	M_Int32		offset		= 0;
	M_Int32		tmp			= 0;
	
	M_Int32		line		= 0;
	
	len = strlen( str );
	while( len > seek )
	{
		tmp = 0;
		while( seek < (max + offset) && len > seek )
		{
			if( *(str + seek) == '$' )	//개행문자
			{
				++seek;
				tmp = 1;
				break;
			}
			
			if( (*(str + seek) & 0xff) > 0x80 )
			{
				seek += 2;
				if( seek > (max + offset) ){ seek -= 2; break; }
			}
			else
				seek += 1;
		}
		offset = seek;

		line++;
	}

	return line;
}

/*---------------------------------------------------------------------------*
Description :	문자열을 최대 바이트(max)에 맞춰 개행 출력한다.
				'$' 문자가 있을 경우에도 역시 개행.
				(max값은 최대로 63byte 가능)
Arguments :		str : 그릴 문자열.
				x : 그릴 위치 x.
				y : 그릴 위치 y.
				max : 한줄에 그릴 최대 문자 수.
				hy : 자간. (15정도가 적당)
				center : 1 = 가운데 정렬
				r,g,b = 글자색

				line = 한페이지를 몇라인으로 설정할 것인지
				page = 몇 페이지를 출력할지 (0 ~ x페이지)
Returns :		void
*---------------------------------------------------------------------------*/
void dspDrawString( const M_Char* str, M_Int32 x, M_Int32 y, M_Int32 max, M_Int16 hy, M_Int8 center, M_Int8 r, M_Int8 g, M_Int8 b, M_Int8 line, M_Int8 page)
{ 
	M_Int32		len			= 0;
	M_Int32		seek		= 0;
	M_Int32		offset		= 0;
	M_Int32		tmp			= 0;
	M_Char		s[ 64 ];

	M_Int32		Chkline		= 0;	//몇번째 라인 출력 중인지 체크

	m_fntID = MC_grpGetFont(MC_GRP_FT_FACE_SYSTEM, MC_GRP_FT_SIZE_SMALL, MC_GRP_FT_STYLE_PLAIN );
	MC_grpSetContext(m_gc, MC_GRP_CONTEXT_FONT_IDX, (void*)MC_grpGetFont(MC_GRP_FT_FACE_SYSTEM, MC_GRP_FT_SIZE_SMALL, MC_GRP_FT_STYLE_PLAIN));
	
	y += 15;

	len = strlen( str );
	while( len > seek )
	{
		tmp = 0;
		while( seek < (max + offset) && len > seek )
		{
			if( *(str + seek) == '$' )	//개행문자
			{
				++seek;
				tmp = 1;
				break;
			}

			if( (*(str + seek) & 0xff) > 0x80 )
			{
				seek += 2;
				if( seek > (max + offset) ){ seek -= 2; break; }
			}
			else
				seek += 1;
		}

		memset( s, 0, 64 );
		memcpy( s, str + offset, seek-offset-tmp );
		offset = seek;

		if (Chkline >= line*page && Chkline < (line*page)+line)
		{
			if (center == 1)	DrawStringC(0, 0, y-(hy*line*page), s, r, g, b);
			else				DrawString( x, y-(hy*line*page), (const char*)s, (void*)MC_grpGetPixelFromRGB(r, g, b) );
		}

		y += hy;
		Chkline++;
	}
}

/*---------------------------------------------------------------------------*
Description :	이미지 회전 (앵커값은 이미지의 좌측 상단)
Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				angle		- 회전각
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_ROTATE == 1
void DDrawImageRotate( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32 angle )
{
	if (angle == 0)
	{
		DDrawImage( img, x, y);
	}
	else
	{
		M_Uint16* pFB_16;
		M_Uint32* pFB_32;

		M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
		M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);

		M_Int32 srcW, srcH;
		
		M_Uint16* offset_16;
		M_Uint16 pixel_16;
		
		M_Uint32* offset_32;
		M_Uint32 pixel_32;

		M_Int32 rx, ry, cw, ch;
		M_Int32 nx, ny, nw, nh, minX, minY, maxX, maxY;

		M_Int32 sinValue = fsinTbl[angle];
		M_Int32 cosValue = fcosTbl[angle];

		M_Int32 cnw;
		M_Int32 cnh;

		M_Int32 offsetY;
		M_Int32 px, py;
		M_Int32 row, col;

		if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
		else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );

		srcW = iw;
		srcH = ih;
		cw = srcW>>1;
		ch = srcH>>1;

		
		minX = maxX = 0;
		minY = maxY = 0;

		nx = srcW*cosValue;
		ny = srcW*sinValue;

		minX = umin(minX, nx);
		maxX = umax(maxX, nx);
		minY = umin(minY, ny);
		maxY = umax(maxY, ny);


		nx = -srcH*sinValue;
		ny = srcH*cosValue;
		minX = umin(minX, nx);
		maxX = umax(maxX, nx);
		minY = umin(minY, ny);
		maxY = umax(maxY, ny);

		nx = srcW*cosValue - srcH*sinValue;
		ny = srcW*sinValue + srcH*cosValue;
		minX = umin(minX, nx);
		maxX = umax(maxX, nx);
		minY = umin(minY, ny);
		maxY = umax(maxY, ny);
		

		minX /= 100;
		minY /= 100;
		maxX /= 100;
		maxY /= 100;
		

		nw = maxX - minX;
		nh = maxY - minY;


		cnw = abs( (nw - srcW)/2 );
		cnh = abs( (nh - srcH)/2 );

		if(m_BPP == 32)
		{
			offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
			
			if (m_alpha == 0)
			{
			}
			else if (m_alpha == HALF_ALPHA)
			{
				if( m_alpha < 1 ) return;
				
				for( row=minY; row < maxY; row++ )		
				{		
					py = y+row;
					if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;		
					offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
					
					for( col=minX; col<maxX; col++ )	
					{	
						px = x+col;			
						if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;			
						
						rx = ( col*cosValue + row*sinValue )/100;
						ry = ( -col*sinValue + row*cosValue )/100;
						
						if( rx < 0 || rx > srcW-1 || ry < 0 || ry > srcH-1 ) continue;			
						
						pixel_32 = *( pFB_32  + rx + iw*ry );
						if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
						
						MC_grpGetRGBFromPixel( pixel_32, &RGB1[0], &RGB1[1], &RGB1[2] );
						MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB2[0], &RGB2[1], &RGB2[2] );				
						
						ColorBlend_Average(RGB3, RGB1, RGB2);
						
						*( offset_32+col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
					}
				}
			}
			else if( m_alpha < 255 )
			{
				if( m_alpha < 1 ) return;
				
				for( row=minY; row < maxY; row++ )		
				{		
					py = y+row;
					if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;		
					offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
					
					for( col=minX; col<maxX; col++ )	
					{	
						px = x+col;			
						if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;			
						
						rx = ( col*cosValue + row*sinValue )/100;
						ry = ( -col*sinValue + row*cosValue )/100;
						
						if( rx < 0 || rx > srcW-1 || ry < 0 || ry > srcH-1 ) continue;			
						
						pixel_32 = *( pFB_32  + rx + iw*ry );
						if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
						
						MC_grpGetRGBFromPixel( pixel_32, &sR, &sG, &sB );
						MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &tR, &tG, &tB );				
						
						rR = ( sR*m_alpha + tR*(255-m_alpha) )>>8;
						rG = ( sG*m_alpha + tG*(255-m_alpha) )>>8;
						rB = ( sB*m_alpha + tB*(255-m_alpha) )>>8;
						
						*( offset_32+col + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
					}
				}
			}
			else
			{
				for( row=minY; row<maxY; row++ )		
				{		
					py = y+row;
					if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;		
					offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
					
					for( col=minX; col<maxX; col++ )	
					{	
						px = x+col;			
						if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;			
						
						rx = ( col*cosValue + row*sinValue )/100;
						ry = ( -col*sinValue + row*cosValue )/100;
						
						if( rx < 0 || rx > srcW-1 || ry < 0 || ry > srcH-1 ) continue;			
						
						pixel_32 = *( pFB_32  + rx + iw*ry );
						if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
						
						*( offset_32 + col + offsetY ) = pixel_32;	
					}
				}
			}
		}
		else
		{
			offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
			
			if (m_alpha == 0)
			{
			}
			else if (m_alpha == HALF_ALPHA)
			{
				if( m_alpha < 1 ) return;
				
				for( row=minY; row < maxY; row++ )		
				{		
					py = y+row;
					if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;		
					offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
					
					for( col=minX; col<maxX; col++ )	
					{	
						px = x+col;			
						if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;			
						
						rx = ( col*cosValue + row*sinValue )/100;
						ry = ( -col*sinValue + row*cosValue )/100;
						
						if( rx < 0 || rx > srcW-1 || ry < 0 || ry > srcH-1 ) continue;			
						
						pixel_16 = *( pFB_16  + rx + iw*ry );
						if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
						
						MC_grpGetRGBFromPixel( pixel_16, &RGB1[0], &RGB1[1], &RGB1[2] );
						MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB2[0], &RGB2[1], &RGB2[2] );				
						
						ColorBlend_Average(RGB3, RGB1, RGB2);
						
						*( offset_16+col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
					}
				}
			}
			else if( m_alpha < 255 )
			{
				if( m_alpha < 1 ) return;
				
				for( row=minY; row < maxY; row++ )		
				{		
					py = y+row;
					if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;		
					offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
					
					for( col=minX; col<maxX; col++ )	
					{	
						px = x+col;			
						if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;			
						
						rx = ( col*cosValue + row*sinValue )/100;
						ry = ( -col*sinValue + row*cosValue )/100;
						
						if( rx < 0 || rx > srcW-1 || ry < 0 || ry > srcH-1 ) continue;			
						
						pixel_16 = *( pFB_16  + rx + iw*ry );
						if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
						
						MC_grpGetRGBFromPixel( pixel_16, &sR, &sG, &sB );
						MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &tR, &tG, &tB );				
						
						rR = ( sR*m_alpha + tR*(255-m_alpha) )>>8;
						rG = ( sG*m_alpha + tG*(255-m_alpha) )>>8;
						rB = ( sB*m_alpha + tB*(255-m_alpha) )>>8;
						
						*( offset_16+col + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
					}
				}
			}
			else
			{
				for( row=minY; row<maxY; row++ )		
				{		
					py = y+row;
					if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;		
					offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
					
					for( col=minX; col<maxX; col++ )	
					{	
						px = x+col;			
						if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;			
						
						rx = ( col*cosValue + row*sinValue )/100;
						ry = ( -col*sinValue + row*cosValue )/100;
						
						if( rx < 0 || rx > srcW-1 || ry < 0 || ry > srcH-1 ) continue;			
						
						pixel_16 = *( pFB_16  + rx + iw*ry );
						if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
						
						*( offset_16 + col + offsetY ) = pixel_16;	
					}
				}
			}
		}
		
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	이미지 회전 (앵커값은 이미지의 중간)
Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				angle		- 회전각
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_CENTER_ROTATE == 1
void DDrawImageCenterRotate( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32 angle )
{
	if (angle == 0)
	{
		DDrawImage( img, x, y);
	}
	else
	{
		M_Uint16* pFB_16;
		M_Uint32* pFB_32;

		M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
		M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);

		M_Int32 srcW, srcH;
		
		M_Uint16* offset_16;
		M_Uint16 pixel_16;
		
		M_Uint32* offset_32;
		M_Uint32 pixel_32;

		M_Int32 rx, ry, cw, ch;
		M_Int32 nx, ny, nw, nh, minX, minY, maxX, maxY;

		M_Int32 sinValue = fsinTbl[angle];
		M_Int32 cosValue = fcosTbl[angle];

		M_Int32 cnw;
		M_Int32 cnh;

		M_Int32 offsetY;
		M_Int32 px, py;
		M_Int32 row, col;


		if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
		else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );

		srcW = iw;
		srcH = ih;
		cw = srcW>>1;
		ch = srcH>>1;	


		minX = maxX = 0;
		minY = maxY = 0;

		nx = (srcW<<1)*cosValue;
		ny = (srcW<<1)*sinValue;
		minX = umin(minX, nx);
		maxX = umax(maxX, nx);
		minY = umin(minY, ny);
		maxY = umax(maxY, ny);

		nx = -(srcH<<1)*sinValue;
		ny = (srcH<<1)*cosValue;
		minX = umin(minX, nx);
		maxX = umax(maxX, nx);
		minY = umin(minY, ny);
		maxY = umax(maxY, ny);

		nx = (srcW<<1)*cosValue - (srcH<<1)*sinValue;
		ny = (srcW<<1)*sinValue + (srcH<<1)*cosValue;
		minX = umin(minX, nx);
		maxX = umax(maxX, nx);
		minY = umin(minY, ny);
		maxY = umax(maxY, ny);
		
		minX /= 100;
		minY /= 100;
		maxX /= 100;
		maxY /= 100;
		

		nw = maxX - minX;
		nh = maxY - minY;

		
		cnw = abs( (nw - srcW)/2 );
		cnh = abs( (nh - srcH)/2 );

		if(m_BPP == 32)
		{
			offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
			
			if (m_alpha == 0)
			{
			}
			else if (m_alpha == HALF_ALPHA)
			{
				if( m_alpha < 1 ) return;
				
				for( row=-cnh; row<nh-cnh; row++ )		
				{		
					py = y+row;
					if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;		
					offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
					
					for( col=-cnw; col<nw-cnw; col++ )	
					{
						px = x+col;			
						if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
						rx = ( (col-cw)*cosValue + (row-ch)*sinValue )/100 + cw;
						ry = ( -(col-cw)*sinValue + (row-ch)*cosValue )/100 + ch;
						
						if( rx < 0 || rx > srcW-1 || ry < 0 || ry > srcH-1 ) continue;			
						
						pixel_32 = *( pFB_32  + rx + iw*ry );
						if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
						
						MC_grpGetRGBFromPixel( pixel_32 ,&RGB1[0], &RGB1[1], &RGB1[2] );
						MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB2[0], &RGB2[1], &RGB2[2] );				
						
						ColorBlend_Average(RGB3, RGB1, RGB2);
						
						*( offset_32+col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
					}
				}
			}
			else if( m_alpha < 255 )
			{
				if( m_alpha < 1 ) return;
				
				for( row=-cnh; row<nh-cnh; row++ )		
				{		
					py = y+row;
					if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;		
					offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
					
					for( col=-cnw; col<nw-cnw; col++ )	
					{
						px = x+col;			
						if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
						rx = ( (col-cw)*cosValue + (row-ch)*sinValue )/100 + cw;
						ry = ( -(col-cw)*sinValue + (row-ch)*cosValue )/100 + ch;
						
						if( rx < 0 || rx > srcW-1 || ry < 0 || ry > srcH-1 ) continue;			
						
						pixel_32 = *( pFB_32  + rx + iw*ry );
						if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
						
						MC_grpGetRGBFromPixel( pixel_32, &sR, &sG, &sB );
						MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &tR, &tG, &tB );				
						
						rR = ( sR*m_alpha + tR*(255-m_alpha) )>>8;
						rG = ( sG*m_alpha + tG*(255-m_alpha) )>>8;
						rB = ( sB*m_alpha + tB*(255-m_alpha) )>>8;
						
						*( offset_32+col + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
					}
				}
			}
			else
			{
				for( row=-cnh; row<nh-cnh; row++ )		
				{		
					py = y+row;
					if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;		
					offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
					
					for( col=-cnw; col<nw-cnw; col++ )	
					{
						px = x+col;			
						if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
						
						rx = ( (col-cw)*cosValue + (row-ch)*sinValue )/100 + cw;
						ry = ( -(col-cw)*sinValue + (row-ch)*cosValue )/100 + ch;
						
						
						if( rx < 0 || rx > srcW-1 || ry < 0 || ry > srcH-1 ) continue;			
						
						pixel_32 = *( pFB_32  + rx + iw*ry );
						
						if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
						
						*( offset_32+col + offsetY ) = pixel_32;
					}
				}
			}
		}
		else
		{
			offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
						
			if (m_alpha == 0)
			{
			}
			else if (m_alpha == HALF_ALPHA)
			{
				if( m_alpha < 1 ) return;
				
				for( row=-cnh; row<nh-cnh; row++ )		
				{		
					py = y+row;
					if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;		
					offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
					
					for( col=-cnw; col<nw-cnw; col++ )	
					{
						px = x+col;			
						if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
						rx = ( (col-cw)*cosValue + (row-ch)*sinValue )/100 + cw;
						ry = ( -(col-cw)*sinValue + (row-ch)*cosValue )/100 + ch;
						
						if( rx < 0 || rx > srcW-1 || ry < 0 || ry > srcH-1 ) continue;			
						
						pixel_16 = *( pFB_16  + rx + iw*ry );
						if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
						
						MC_grpGetRGBFromPixel( pixel_16 ,&RGB1[0], &RGB1[1], &RGB1[2] );
						MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB2[0], &RGB2[1], &RGB2[2] );				
						
						ColorBlend_Average(RGB3, RGB1, RGB2);
						
						*( offset_16+col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
					}
				}
			}
			else if( m_alpha < 255 )
			{
				if( m_alpha < 1 ) return;
				
				for( row=-cnh; row<nh-cnh; row++ )		
				{		
					py = y+row;
					if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;		
					offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
					
					for( col=-cnw; col<nw-cnw; col++ )	
					{
						px = x+col;			
						if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
						rx = ( (col-cw)*cosValue + (row-ch)*sinValue )/100 + cw;
						ry = ( -(col-cw)*sinValue + (row-ch)*cosValue )/100 + ch;
						
						if( rx < 0 || rx > srcW-1 || ry < 0 || ry > srcH-1 ) continue;			
						
						pixel_16 = *( pFB_16  + rx + iw*ry );
						if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
						
						MC_grpGetRGBFromPixel( pixel_16, &sR, &sG, &sB );
						MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &tR, &tG, &tB );				
						
						rR = ( sR*m_alpha + tR*(255-m_alpha) )>>8;
						rG = ( sG*m_alpha + tG*(255-m_alpha) )>>8;
						rB = ( sB*m_alpha + tB*(255-m_alpha) )>>8;
						
						*( offset_16+col + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
					}
				}
				
				
			}
			else
			{
				for( row=-cnh; row<nh-cnh; row++ )		
				{		
					py = y+row;
					if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;		
					offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
					
					for( col=-cnw; col<nw-cnw; col++ )	
					{
						px = x+col;			
						if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
						
						rx = ( (col-cw)*cosValue + (row-ch)*sinValue )/100 + cw;
						ry = ( -(col-cw)*sinValue + (row-ch)*cosValue )/100 + ch;
						
						
						if( rx < 0 || rx > srcW-1 || ry < 0 || ry > srcH-1 ) continue;			
						
						pixel_16 = *( pFB_16  + rx + iw*ry );
						
						if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
						
						*( offset_16+col + offsetY ) = pixel_16;
					}
				}
			}
		}
	}
}
#endif


/*---------------------------------------------------------------------------*
Description :	지정한 컬러 만으로 이미지를 그린다.
Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				color		- 변경할 색상
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_FILL_COLOR == 1
void DDrawImageFillColor( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32 color )
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;

	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih, sw = iw;
	M_Uint16 sx = 0, sy = 0;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 px, py;
	M_Int32 row, col;
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );

	if (m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		if (m_alpha == 0)
		{
		}
		else if (m_alpha == HALF_ALPHA)
		{
			if( m_alpha < 1 ) return;
			
			for( row = sy; row < sy+sh; row++ )
			{
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue; ///< 가로축 화면영역 채크
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = sx; col<sx+sw; col++ )
				{						
					px = x+col;			
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue; ///< 새로축 화면영역 채크
					
					pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
					
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					
					pixel_32 = color;			
					
					MC_grpGetRGBFromPixel( pixel_32, &RGB1[0], &RGB1[1], &RGB1[2] );
					MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB2[0], &RGB2[1], &RGB2[2] );
					
					ColorBlend_Average(RGB3, RGB1, RGB2);
					
					*( offset_32+col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
				}
			}
		}
		else if( m_alpha < 255 )
		{
			if( m_alpha < 1 ) return;
			
			for( row = sy; row < sy+sh; row++ )
			{
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue; ///< 가로축 화면영역 채크
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = sx; col<sx+sw; col++ )
				{						
					px = x+col;			
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue; ///< 새로축 화면영역 채크
					
					pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
					
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					
					pixel_32 = color;			
					
					MC_grpGetRGBFromPixel( pixel_32, &sR, &sG, &sB );
					MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &tR, &tG, &tB );				
					
					rR = ( sR*m_alpha + tR*(255-m_alpha) )>>8;
					rG = ( sG*m_alpha + tG*(255-m_alpha) )>>8;
					rB = ( sB*m_alpha + tB*(255-m_alpha) )>>8;
					
					*( offset_32+col + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
				}
			}
		}
		else
		{
			for( row = sy; row < sy+sh; row++ )
			{
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue; ///< 가로축 화면영역 채크
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = sx; col<sx+sw; col++ )
				{						
					px = x+col;			
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue; ///< 새로축 화면영역 채크
					
					pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
					
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					
					pixel_32 = color;			
					
					*( offset_32+col + offsetY ) = pixel_32;
				}
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		if (m_alpha == 0)
		{
		}
		else if (m_alpha == HALF_ALPHA)
		{
			if( m_alpha < 1 ) return;
			
			for( row = sy; row < sy+sh; row++ )
			{
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue; ///< 가로축 화면영역 채크
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = sx; col<sx+sw; col++ )
				{						
					px = x+col;			
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue; ///< 새로축 화면영역 채크
					
					pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
					
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					
					pixel_16 = color;			
					
					MC_grpGetRGBFromPixel( pixel_16, &RGB1[0], &RGB1[1], &RGB1[2] );
					MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB2[0], &RGB2[1], &RGB2[2] );
					
					ColorBlend_Average(RGB3, RGB1, RGB2);
					
					*( offset_16+col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
				}
			}
		}
		else if( m_alpha < 255 )
		{
			if( m_alpha < 1 ) return;
			
			for( row = sy; row < sy+sh; row++ )
			{
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue; ///< 가로축 화면영역 채크
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = sx; col<sx+sw; col++ )
				{						
					px = x+col;			
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue; ///< 새로축 화면영역 채크
					
					pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
					
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					
					pixel_16 = color;			
					
					MC_grpGetRGBFromPixel( pixel_16, &sR, &sG, &sB );
					MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &tR, &tG, &tB );				
					
					rR = ( sR*m_alpha + tR*(255-m_alpha) )>>8;
					rG = ( sG*m_alpha + tG*(255-m_alpha) )>>8;
					rB = ( sB*m_alpha + tB*(255-m_alpha) )>>8;
					
					*( offset_16+col + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
				}
			}
			
		}
		else
		{
			for( row = sy; row < sy+sh; row++ )
			{
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue; ///< 가로축 화면영역 채크
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = sx; col<sx+sw; col++ )
				{						
					px = x+col;			
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue; ///< 새로축 화면영역 채크
					
					pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
					
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
					
					pixel_16 = color;			
					
					*( offset_16+col + offsetY ) = pixel_16;
				}
			}
		}
	}
}
#endif


/*---------------------------------------------------------------------------*
Description :	바꿀 픽셀값과 바뀔 픽셀값의 배열주소를 전달하여 팔레트변환을 한다.
Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				pDestColor	- 바꿀 필셀값
				pToColor	- 바뀔 픽셀값
				length		- 픽셀 길이
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_COLOR_TO == 1
void DDrawImageColorTo( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32* pDestColor, M_Int32* pToColor , M_Int32 length )
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;

	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);

	M_Uint16 sh = ih, sw = iw;
	M_Uint16 sx = 0, sy = 0;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;

	M_Int32 offsetY, ry;
	M_Int32 px, py;
	M_Int32 row, col, n;

	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );

	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		if( m_alpha < 255 )
		{
			if( m_alpha < 1 ) return;
			
			for( row = sy; row < sy+sh; row++ )
			{
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue; ///< 가로축 화면영역 채크
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = sx; col<sx+sw; col++ )
				{						
					px = x+col;			
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue; ///< 새로축 화면영역 채크
					
					pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
					
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
					
					for( n=0; n<length; n++ )
					{
						if( pixel_32 == *(pDestColor+n) ) 
						{
							pixel_32 = *(pToColor+n);					
							break;
						}
					}
					
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
					
					MC_grpGetRGBFromPixel( pixel_32, &sR, &sG, &sB );
					MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &tR, &tG, &tB );				
					
					rR = ( sR*m_alpha + tR*(255-m_alpha) )>>8;
					rG = ( sG*m_alpha + tG*(255-m_alpha) )>>8;
					rB = ( sB*m_alpha + tB*(255-m_alpha) )>>8;
					
					*( offset_32+col + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
				}
			}
			
		}
		else
		{
			for( row = sy; row < sy+sh; row++ )
			{
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue; ///< 가로축 화면영역 채크
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = sx; col<sx+sw; col++ )
				{						
					px = x+col;			
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue; ///< 새로축 화면영역 채크
					pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
					
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
					
					for( n=0; n<length; n++ )
					{
						if( pixel_32 == *(pDestColor+n) ) 
						{
							pixel_32 = *(pToColor+n);					
							break;
						}
					}
					
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
					
					*( offset_32+col + offsetY ) = pixel_32;
				}
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		if( m_alpha < 255 )
		{
			if( m_alpha < 1 ) return;
			
			for( row = sy; row < sy+sh; row++ )
			{
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue; ///< 가로축 화면영역 채크
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = sx; col<sx+sw; col++ )
				{						
					px = x+col;			
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue; ///< 새로축 화면영역 채크
					
					pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
					
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
					
					for( n=0; n<length; n++ )
					{
						if( pixel_16 == *(pDestColor+n) ) 
						{
							pixel_16 = *(pToColor+n);					
							break;
						}
					}
					
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
					
					MC_grpGetRGBFromPixel( pixel_16, &sR, &sG, &sB );
					MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &tR, &tG, &tB );				
					
					rR = ( sR*m_alpha + tR*(255-m_alpha) )>>8;
					rG = ( sG*m_alpha + tG*(255-m_alpha) )>>8;
					rB = ( sB*m_alpha + tB*(255-m_alpha) )>>8;
					
					*( offset_16+col + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
				}
			}
			
		}
		else
		{
			for( row = sy; row < sy+sh; row++ )
			{
				py = y+row;
				if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue; ///< 가로축 화면영역 채크
				ry = iw*row; ///< 참조할 이미지소스의 y좌표
				offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
				
				for( col = sx; col<sx+sw; col++ )
				{						
					px = x+col;			
					if( px < 0 || px > m_LCD_WIDTH_M1 ) continue; ///< 새로축 화면영역 채크
					pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
					
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
					
					for( n=0; n<length; n++ )
					{
						if( pixel_16 == *(pDestColor+n) ) 
						{
							pixel_16 = *(pToColor+n);					
							break;
						}
					}
					
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
					
					*( offset_16+col + offsetY ) = pixel_16;
				}
			}
		}
	}
}
#endif

#if DRAW_BUFFRESIZE == 1
/*---------------------------------------------------------------------------*
Description :	화면 확대/축소를 초기화 한다.
Arguments :		void
Returns :		void
*---------------------------------------------------------------------------*/
MC_GrpFrameBuffer	m_ReSize_BGBuff;
void DDrawBuffReSizeInit( void )
{
	m_ReSize_BGBuff = MC_grpCreateOffScreenFrameBuffer(MC_GRP_GET_FRAME_BUFFER_WIDTH(MC_grpGetScreenFrameBuffer(0)), MC_GRP_GET_FRAME_BUFFER_HEIGHT(MC_grpGetScreenFrameBuffer(0)));
}

/*---------------------------------------------------------------------------*
Description :	화면 확대/축소를 릴리즈 한다.
Arguments :		void
Returns :		void
*---------------------------------------------------------------------------*/
void DDrawBuffReSizeRelese( void )
{
	MC_grpDestroyOffScreenFrameBuffer(m_ReSize_BGBuff);
}

/*---------------------------------------------------------------------------*
Description :	화면 확대/축소
Arguments :		x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				nw			- 확대/축소 될 가로사이즈
				nh			- 확대/축소 될 세로사이즈
Returns :		void
*---------------------------------------------------------------------------*/
void DDrawBuffReSize( M_Int32 x, M_Int32 y, M_Int32 nw, M_Int32 nh )
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;

	M_Uint16 ih = m_LCD_HEIGHT; 
	M_Uint16 iw = m_LCD_WIDTH;
    
	M_Int32 srcW, srcH, rx, ry;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY;
	M_Int32 row, col, px, py;
		
	srcW = iw;
	srcH = ih;

	#if LGT_APP == 1
	{
		M_Uint16* pMainFB;
		M_Uint16* pBackFB;
		
		pMainFB = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( *m_ReSize_BGBuff );
		pBackFB = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( *m_offScreenBuffer );
		
		memcpy( pMainFB, pBackFB, m_LCD_WIDTH * m_LCD_HEIGHT * 2 );
	}
	#else
	MC_grpCopyFrameBuffer( m_ReSize_BGBuff, 0, 0, m_LCD_WIDTH, m_LCD_HEIGHT, *m_offScreenBuffer, 0, 0, m_gc);
	#endif

	//오프스크린을 직접 접근하면 화면 절반 정도가 깨진다.
	/*if(m_BPP == 32)		pFB_32 = ((M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( *m_offScreenBuffer ));
	else				pFB_16 = ((M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( *m_offScreenBuffer ));*/

	if(m_BPP == 32)		pFB_32 = ((M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( m_ReSize_BGBuff ));
	else				pFB_16 = ((M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( m_ReSize_BGBuff ));
		
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 주어진 xy 좌표에 의한 프레임버퍼에 그릴 시작위치
		
		for( row=0; row<nh; row++ )
		{		
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;
			
			ry = (srcH*row/nh)*iw; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col=0; col<nw; col++ )
			{		
				px = x+col;			
				if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
				
				rx = srcW*col/nw; ///< 참조할 이미지소스의 x좌표
				pixel_32 = *( pFB_32  + rx + ry ); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
				
				*( offset_32+col + offsetY ) = pixel_32;
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 주어진 xy 좌표에 의한 프레임버퍼에 그릴 시작위치

		for( row=0; row<nh; row++ )
		{		
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;
			
			ry = (srcH*row/nh)*iw; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col=0; col<nw; col++ )
			{		
				px = x+col;			
				if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
				
				rx = srcW*col/nw; ///< 참조할 이미지소스의 x좌표
				pixel_16 = *( pFB_16  + rx + ry ); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
				
				*( offset_16 + col + offsetY ) = pixel_16;
			}
		}
	}
}
#endif


/*---------------------------------------------------------------------------*
Description :	지정한 컬러 만으로 이미지를 그린다.
Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				refraction	- 굴절률 (120 추천)
Returns :		void

  속도개선을 하려면 Y축 굴절을 생략하면 된다.
*---------------------------------------------------------------------------*/
#if DRAW_DKEFF == 1
void DDrawDKEff( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int16 refraction )
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;

	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
    
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	M_Int32 sx = 0, sy = 0, dx = x, dy = y;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;

	
	M_Int32 offsetY, ry;
	M_Int32 row, col, px, py;

	M_Int32 refractionX = (m_LCD_WIDTH*refraction)/100;	//X축 굴절률
	M_Int32 refractionY = (m_LCD_HEIGHT*refraction)/100;	//Y축 굴절률

	M_Int32 LCDWIDTHxY = y*m_LCD_WIDTH;
		
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );

	//이미지가 화면상에서 완전히 나갔을 때 출력 루틴 종료
	if ( y < -ih)			return;
	if ( y > m_LCD_HEIGHT)	return;
	if ( x < -iw)			return;
	if ( x > m_LCD_WIDTH)	return;
	
	///< 이미지가 LCD 왼쪽 경계에 걸릴경우
	if( x < 0 )
	{
		sx = -x;			
		dx = 0;
		
		sw = iw + x;
		if( sw > m_LCD_WIDTH ) ///< 이미지소스가 LCD폭보다 더 크다면
		{
			sw = m_LCD_WIDTH; ///< 이미지소스 참조는 실제 화면에 그려지는 LCD_WIDTH 만큼만 참조한다
		}
	}
	///< 이미지가 LCD 오른쪽에 걸릴경우
	else if( x + iw > m_LCD_WIDTH ) 
	{
		sw = m_LCD_WIDTH - x;
	}
	
	
	///< 이미지가 위쪽에 걸릴경우
	if( y < 0 )
	{
		sy = -y;
		dy = 0;
		
		sh = ih + y;
		if( sh > m_LCD_HEIGHT )
		{
			sh = m_LCD_HEIGHT; ///< 이미지소스 높이가 화면크기보다 크면 실제 화면에 그릴수 있는 LCD_HEIGHT만큼만 참조한다.
		}
	}
	///< 이미지가 아래쪽에 걸릴경우
	else if( y + ih > m_LCD_HEIGHT )
	{			
		sh = m_LCD_HEIGHT - y;			
	}

	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 주어진 xy 좌표에 의한 프레임버퍼에 그릴 시작위치
		
		for( row=0; row<sy+sh; row++ )
		{		
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col=0; col<sx+sw; col++ )
			{		
				px = x+col;			
				if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
				
				//rx = srcW*col/nw; ///< 참조할 이미지소스의 x좌표
				pixel_32 = *( pFB_32  + col + ry ); ///< 이미지소스에서 칼라값 얻어옴
				
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다

				//pixel_32 = *( m_pBackFB_32 + (x+col) + ((y*m_LCD_WIDTH)+(row*m_LCD_WIDTH)) ); ///< 이미지소스에서 칼라값 얻어옴

				/////////////////////////////////////////////////	일반
				/*pixel_32 = *( m_pBackFB_32 + 
					(x+(refractionX*col/m_LCD_WIDTH)) + 
					(LCDWIDTHxY+((refractionY*row/m_LCD_HEIGHT)*m_LCD_WIDTH)) ); ///< 이미지소스에서 칼라값 얻어옴*/

				/////////////////////////////////////////////////	속도 개선
				pixel_32 = *( m_pBackFB_32 + 
					(x+(refractionX*col/m_LCD_WIDTH)) + 
					((y*m_LCD_WIDTH)+(row*m_LCD_WIDTH)) ); ///< 이미지소스에서 칼라값 얻어옴
				
				*( offset_32 + col + offsetY ) = pixel_32;
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 주어진 xy 좌표에 의한 프레임버퍼에 그릴 시작위치
		
		for( row=0; row<sy+sh; row++ )
		{		
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col=0; col<sx+sw; col++ )
			{		
				px = x+col;			
				if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
				
				//rx = srcW*col/nw; ///< 참조할 이미지소스의 x좌표
				pixel_16 = *( pFB_16  + col + ry ); ///< 이미지소스에서 칼라값 얻어옴
				
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다

				//pixel_16 = *( m_pBackFB_16 + (x+col) + ((y*m_LCD_WIDTH)+(row*m_LCD_WIDTH)) ); ///< 이미지소스에서 칼라값 얻어옴

				/////////////////////////////////////////////////	일반
				/*pixel_16 = *( m_pBackFB_16 + 
					(x+(refractionX*col/m_LCD_WIDTH)) + 
					(LCDWIDTHxY+((refractionY*row/m_LCD_HEIGHT)*m_LCD_WIDTH)) ); ///< 이미지소스에서 칼라값 얻어옴*/

				/////////////////////////////////////////////////	속도 개선
				pixel_16 = *( m_pBackFB_16 + 
					(x+(refractionX*col/m_LCD_WIDTH)) + 
					((y*m_LCD_WIDTH)+(row*m_LCD_WIDTH)) ); ///< 이미지소스에서 칼라값 얻어옴
				
				*( offset_16 + col + offsetY ) = pixel_16;
			}
		}
	}
}
#endif


/*---------------------------------------------------------------------------*
Description :	모자이크
Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				Mos			- 모자이크 배율
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_MOSAIC == 1
void DDrawImageMosaic( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32 Mos)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;

	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
    
	M_Int32 rx, ry;

	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	M_Int32 sx = 0, sy = 0, dx = x, dy = y;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY;
	M_Int32 row, col, px, py;
		
		
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );

	//이미지가 화면상에서 완전히 나갔을 때 출력 루틴 종료
	if ( y < -ih)			return;
	if ( y > m_LCD_HEIGHT)	return;
	if ( x < -iw)			return;
	if ( x > m_LCD_WIDTH)	return;
	
	///< 이미지가 LCD 왼쪽 경계에 걸릴경우
	if( x < 0 )
	{
		sx = -x;			
		dx = 0;
		
		sw = iw + x;
		if( sw > m_LCD_WIDTH ) ///< 이미지소스가 LCD폭보다 더 크다면
		{
			sw = m_LCD_WIDTH; ///< 이미지소스 참조는 실제 화면에 그려지는 LCD_WIDTH 만큼만 참조한다
		}
	}
	///< 이미지가 LCD 오른쪽에 걸릴경우
	else if( x + iw > m_LCD_WIDTH ) 
	{
		sw = m_LCD_WIDTH - x;
	}
	
	
	///< 이미지가 위쪽에 걸릴경우
	if( y < 0 )
	{
		sy = -y;
		dy = 0;
		
		sh = ih + y;
		if( sh > m_LCD_HEIGHT )
		{
			sh = m_LCD_HEIGHT; ///< 이미지소스 높이가 화면크기보다 크면 실제 화면에 그릴수 있는 LCD_HEIGHT만큼만 참조한다.
		}
	}
	///< 이미지가 아래쪽에 걸릴경우
	else if( y + ih > m_LCD_HEIGHT )
	{			
		sh = m_LCD_HEIGHT - y;			
	}

	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + dx + m_LCD_WIDTH*dy; ///< 주어진 xy 좌표에 의한 프레임버퍼에 그릴 시작위치
		
		for( row = sy; row < sy+sh; row++ )
		{
			ry = iw*((row/Mos)*Mos); ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = sx; col < sx+sw; col++ )
			{
				pixel_32 = *(pFB_32 + ((col/Mos)*Mos) + ry); ///< 이미지소스에서 칼라값 얻어옴
				
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
				*( offset_32 +  (col-sx) + offsetY ) = pixel_32;
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + dx + m_LCD_WIDTH*dy; ///< 주어진 xy 좌표에 의한 프레임버퍼에 그릴 시작위치

		for( row = sy; row < sy+sh; row++ )
		{
			ry = iw*((row/Mos)*Mos); ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = sx; col < sx+sw; col++ )
			{
				pixel_16 = *(pFB_16 + ((col/Mos)*Mos) + ry); ///< 이미지소스에서 칼라값 얻어옴

				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
				*( offset_16 +  (col-sx) + offsetY ) = pixel_16;
			}
		}
	}
}
#endif


/*---------------------------------------------------------------------------*
Description :	그레이스케일
Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_GRAY == 1
void DDrawImageGray( MC_GrpImage img, M_Int32 x, M_Int32 y)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;

	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
    
	M_Int32 rx, ry;

	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	M_Int32 sx = 0, sy = 0, dx = x, dy = y;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY;
	M_Int32 row, col, px, py;

	M_Int32 GrayColor;
		
		
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );

	//이미지가 화면상에서 완전히 나갔을 때 출력 루틴 종료
	if ( y < -ih)			return;
	if ( y > m_LCD_HEIGHT)	return;
	if ( x < -iw)			return;
	if ( x > m_LCD_WIDTH)	return;
	
	///< 이미지가 LCD 왼쪽 경계에 걸릴경우
	if( x < 0 )
	{
		sx = -x;			
		dx = 0;
		
		sw = iw + x;
		if( sw > m_LCD_WIDTH ) ///< 이미지소스가 LCD폭보다 더 크다면
		{
			sw = m_LCD_WIDTH; ///< 이미지소스 참조는 실제 화면에 그려지는 LCD_WIDTH 만큼만 참조한다
		}
	}
	///< 이미지가 LCD 오른쪽에 걸릴경우
	else if( x + iw > m_LCD_WIDTH ) 
	{
		sw = m_LCD_WIDTH - x;
	}
	
	
	///< 이미지가 위쪽에 걸릴경우
	if( y < 0 )
	{
		sy = -y;
		dy = 0;
		
		sh = ih + y;
		if( sh > m_LCD_HEIGHT )
		{
			sh = m_LCD_HEIGHT; ///< 이미지소스 높이가 화면크기보다 크면 실제 화면에 그릴수 있는 LCD_HEIGHT만큼만 참조한다.
		}
	}
	///< 이미지가 아래쪽에 걸릴경우
	else if( y + ih > m_LCD_HEIGHT )
	{			
		sh = m_LCD_HEIGHT - y;			
	}

	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + dx + m_LCD_WIDTH*dy; ///< 주어진 xy 좌표에 의한 프레임버퍼에 그릴 시작위치

		for( row = sy; row < sy+sh; row++ )
		{
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = sx; col<sx+sw; col++ )
			{				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( pixel_32, &sR, &sG, &sB );
				
				GrayColor = (sR+sG+sB)/3;
				
				*( offset_32 +  (col-sx) + offsetY ) = MC_grpGetPixelFromRGB( GrayColor, GrayColor, GrayColor );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + dx + m_LCD_WIDTH*dy; ///< 주어진 xy 좌표에 의한 프레임버퍼에 그릴 시작위치

		for( row = sy; row < sy+sh; row++ )
		{
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = sx; col<sx+sw; col++ )
			{				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( pixel_16, &sR, &sG, &sB );
				
				GrayColor = (sR+sG+sB)/3;
				
				*( offset_16 +  (col-sx) + offsetY ) = MC_grpGetPixelFromRGB( GrayColor, GrayColor, GrayColor );
			}
		}
	}
}
#endif


/*---------------------------------------------------------------------------*
Description :	명도 조절
Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				v			- -255 ~ 255
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_BRIGHTNESS == 1
void DDrawImageBrightness( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int16 v)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;

	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
    
	M_Int32 rx, ry;

	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	M_Int32 sx = 0, sy = 0, dx = x, dy = y;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY;
	M_Int32 row, col, px, py;

	M_Int32 GrayColor;
		
		
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );

	//이미지가 화면상에서 완전히 나갔을 때 출력 루틴 종료
	if ( y < -ih)			return;
	if ( y > m_LCD_HEIGHT)	return;
	if ( x < -iw)			return;
	if ( x > m_LCD_WIDTH)	return;
	
	///< 이미지가 LCD 왼쪽 경계에 걸릴경우
	if( x < 0 )
	{
		sx = -x;			
		dx = 0;
		
		sw = iw + x;
		if( sw > m_LCD_WIDTH ) ///< 이미지소스가 LCD폭보다 더 크다면
		{
			sw = m_LCD_WIDTH; ///< 이미지소스 참조는 실제 화면에 그려지는 LCD_WIDTH 만큼만 참조한다
		}
	}
	///< 이미지가 LCD 오른쪽에 걸릴경우
	else if( x + iw > m_LCD_WIDTH ) 
	{
		sw = m_LCD_WIDTH - x;
	}
	
	
	///< 이미지가 위쪽에 걸릴경우
	if( y < 0 )
	{
		sy = -y;
		dy = 0;
		
		sh = ih + y;
		if( sh > m_LCD_HEIGHT )
		{
			sh = m_LCD_HEIGHT; ///< 이미지소스 높이가 화면크기보다 크면 실제 화면에 그릴수 있는 LCD_HEIGHT만큼만 참조한다.
		}
	}
	///< 이미지가 아래쪽에 걸릴경우
	else if( y + ih > m_LCD_HEIGHT )
	{			
		sh = m_LCD_HEIGHT - y;			
	}

	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + dx + m_LCD_WIDTH*dy; ///< 주어진 xy 좌표에 의한 프레임버퍼에 그릴 시작위치

		for( row = sy; row < sy+sh; row++ )
		{
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = sx; col<sx+sw; col++ )
			{				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( pixel_32, &sR, &sG, &sB );
				
				rR = sR+v;
				rG = sG+v;
				rB = sB+v;
				
				if (rR > 255)		rR = 255;
				else if (rR < 0)	rR = 0;
				
				if (rG > 255)		rG = 255;
				else if (rG < 0)	rG = 0;
				
				if (rB > 255)		rB = 255;
				else if (rB < 0)	rB = 0;
				
				*( offset_32 +  (col-sx) + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + dx + m_LCD_WIDTH*dy; ///< 주어진 xy 좌표에 의한 프레임버퍼에 그릴 시작위치

		for( row = sy; row < sy+sh; row++ )
		{
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = sx; col<sx+sw; col++ )
			{				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( pixel_16, &sR, &sG, &sB );
								
				rR = sR+v;
				rG = sG+v;
				rB = sB+v;

				if (rR > 255)		rR = 255;
				else if (rR < 0)	rR = 0;

				if (rG > 255)		rG = 255;
				else if (rG < 0)	rG = 0;

				if (rB > 255)		rB = 255;
				else if (rB < 0)	rB = 0;
				
				*( offset_16 +  (col-sx) + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
			}
		}
	}
}
#endif


/*---------------------------------------------------------------------------*
Description :	이미지 래스터연산
Arguments :		src			- 출력할 이미지데이터
				dx			- 스크린상의 출력할 위치 (x좌표)
				dy			- 스크린상의 출력할 위치 (y좌표)

				flag		-	ROP_SRCINVERT,		//[Src^Des]배경과 설정값을 XOR 연산하여 출력한다.
								ROP_SRCAND,			//[Src&Des]배경과 설정값을 AND 연산하여 출력한다.
								ROP_SCRPAINT,		//[Src|Des]배경과 설정값을 OR 연산하여 출력한다.
								ROP_DSTINVERT,		//[~Des]배경을 Inverse한다.	
								ROP_NOTSRCCOPY,		//[~Src]설정값을 Inverse하여 출력 한다.
								ROP_SRCERASE,		//[Src&~Des]배경값을 Inverse한 다음 설정값과 AND 연산을 한다.	
								ROP_MERGEPAINT,		//[~Src|Des]설정값을 Inverse한 다음 배경값과 OR 연산을 한다.
								ROP_NOTSRCERASE,	//[~(Src|Des)]배경과 설정할 값을 OR 연산을 하고 난 후에 그 값을 Inverse한다.
								ROP_BLACKNESS,		//[0]검은색 출력
								ROP_WHITENESS,		//[255]흰색 출력
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_RASTER == 1
void DDrawImageRaster( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int8 flag)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;

	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);

	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	M_Int32 sx = 0, sy = 0, dx = x, dy = y;

	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;

	M_Int32 offsetY, ry;
	M_Int32 row, col;


	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );

	//이미지가 화면상에서 완전히 나갔을 때 출력 루틴 종료
	if ( y < -ih)			return;
	if ( y > m_LCD_HEIGHT)	return;
	if ( x < -iw)			return;
	if ( x > m_LCD_WIDTH)	return;

	///< 이미지가 LCD 왼쪽 경계에 걸릴경우
	if( x < 0 )
	{
		sx = -x;			
		dx = 0;

		sw = iw + x;
		if( sw > m_LCD_WIDTH ) ///< 이미지소스가 LCD폭보다 더 크다면
		{
			sw = m_LCD_WIDTH; ///< 이미지소스 참조는 실제 화면에 그려지는 LCD_WIDTH 만큼만 참조한다
		}
	}
	///< 이미지가 LCD 오른쪽에 걸릴경우
	else if( x + iw > m_LCD_WIDTH ) 
	{
		sw = m_LCD_WIDTH - x;
	}


	///< 이미지가 위쪽에 걸릴경우
	if( y < 0 )
	{
		sy = -y;
		dy = 0;

		sh = ih + y;
		if( sh > m_LCD_HEIGHT )
		{
			sh = m_LCD_HEIGHT; ///< 이미지소스 높이가 화면크기보다 크면 실제 화면에 그릴수 있는 LCD_HEIGHT만큼만 참조한다.
		}
	}
	///< 이미지가 아래쪽에 걸릴경우
	else if( y + ih > m_LCD_HEIGHT )
	{			
		sh = m_LCD_HEIGHT - y;			
	}

	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + dx + m_LCD_WIDTH*dy; ///< 프레임버퍼에 그릴 시작위치

		for( row = sy; row < sy+sh; row++ )
		{
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = sx; col<sx+sw; col++ )
			{				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( pixel_32, &sR, &sG, &sB );
				MC_grpGetRGBFromPixel( *( offset_32 +  col-sx + offsetY ), &tR, &tG, &tB );				
				
				switch (flag)
				{
				case ROP_SRCINVERT:
					rR = ( sR^tR );
					rG = ( sG^tG );
					rB = ( sB^tB );
					break;
				case ROP_SRCAND:
					rR = ( sR&tR );
					rG = ( sG&tG );
					rB = ( sB&tB );
					break;
				case ROP_SCRPAINT:
					rR = ( sR|tR );
					rG = ( sG|tG );
					rB = ( sB|tB );
					break;
				case ROP_DSTINVERT:
					rR = ( ~tR );
					rG = ( ~tG );
					rB = ( ~tB );
					break;
				case ROP_NOTSRCCOPY:
					rR = ( ~sR );
					rG = ( ~sG );
					rB = ( ~sB );
					break;
				case ROP_SRCERASE:
					rR = ( sR&~tR );
					rG = ( sG&~tG );
					rB = ( sB&~tB );
					break;
				case ROP_MERGEPAINT:
					rR = ( ~sR|tR );
					rG = ( ~sG|tG );
					rB = ( ~sB|tB );
					break;
				case ROP_NOTSRCERASE:
					rR = ~( sR|tR );
					rG = ~( sG|tG );
					rB = ~( sB|tB );
					break;
				case ROP_BLACKNESS:
					rR = 0;
					rG = 0;
					rB = 0;
					break;
				case ROP_WHITENESS:
					rR = 255;
					rG = 255;
					rB = 255;
					break;
				}
				
				
				*( offset_32 +  (col-sx) + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + dx + m_LCD_WIDTH*dy; ///< 프레임버퍼에 그릴 시작위치

		for( row = sy; row < sy+sh; row++ )
		{
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = sx; col<sx+sw; col++ )
			{				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( pixel_16, &sR, &sG, &sB );
				MC_grpGetRGBFromPixel( *( offset_16 +  col-sx + offsetY ), &tR, &tG, &tB );				
				
				switch (flag)
				{
				case ROP_SRCINVERT:
					rR = ( sR^tR );
					rG = ( sG^tG );
					rB = ( sB^tB );
					break;
				case ROP_SRCAND:
					rR = ( sR&tR );
					rG = ( sG&tG );
					rB = ( sB&tB );
					break;
				case ROP_SCRPAINT:
					rR = ( sR|tR );
					rG = ( sG|tG );
					rB = ( sB|tB );
					break;
				case ROP_DSTINVERT:
					rR = ( ~tR );
					rG = ( ~tG );
					rB = ( ~tB );
					break;
				case ROP_NOTSRCCOPY:
					rR = ( ~sR );
					rG = ( ~sG );
					rB = ( ~sB );
					break;
				case ROP_SRCERASE:
					rR = ( sR&~tR );
					rG = ( sG&~tG );
					rB = ( sB&~tB );
					break;
				case ROP_MERGEPAINT:
					rR = ( ~sR|tR );
					rG = ( ~sG|tG );
					rB = ( ~sB|tB );
					break;
				case ROP_NOTSRCERASE:
					rR = ~( sR|tR );
					rG = ~( sG|tG );
					rB = ~( sB|tB );
					break;
				case ROP_BLACKNESS:
					rR = 0;
					rG = 0;
					rB = 0;
					break;
				case ROP_WHITENESS:
					rR = 255;
					rG = 255;
					rB = 255;
					break;
				}
				
				
				*( offset_16 +  (col-sx) + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
			}
		}
	}
}
#endif


/*---------------------------------------------------------------------------*
Description :	랜덤디더링
Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)

				flag		- 0 : 그레이 디더링
							- 1 : 컬러 디더링
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_RANDOM_DITHERING == 1
void DDrawImageRandomDithering( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32 flag)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;

	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
    
	M_Int32 rx, ry;

	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	M_Int32 sx = 0, sy = 0, dx = x, dy = y;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY;
	M_Int32 row, col, px, py;

	M_Int32 mean, rnd;
		
		
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );

	//이미지가 화면상에서 완전히 나갔을 때 출력 루틴 종료
	if ( y < -ih)			return;
	if ( y > m_LCD_HEIGHT)	return;
	if ( x < -iw)			return;
	if ( x > m_LCD_WIDTH)	return;
	
	///< 이미지가 LCD 왼쪽 경계에 걸릴경우
	if( x < 0 )
	{
		sx = -x;			
		dx = 0;
		
		sw = iw + x;
		if( sw > m_LCD_WIDTH ) ///< 이미지소스가 LCD폭보다 더 크다면
		{
			sw = m_LCD_WIDTH; ///< 이미지소스 참조는 실제 화면에 그려지는 LCD_WIDTH 만큼만 참조한다
		}
	}
	///< 이미지가 LCD 오른쪽에 걸릴경우
	else if( x + iw > m_LCD_WIDTH ) 
	{
		sw = m_LCD_WIDTH - x;
	}
	
	
	///< 이미지가 위쪽에 걸릴경우
	if( y < 0 )
	{
		sy = -y;
		dy = 0;
		
		sh = ih + y;
		if( sh > m_LCD_HEIGHT )
		{
			sh = m_LCD_HEIGHT; ///< 이미지소스 높이가 화면크기보다 크면 실제 화면에 그릴수 있는 LCD_HEIGHT만큼만 참조한다.
		}
	}
	///< 이미지가 아래쪽에 걸릴경우
	else if( y + ih > m_LCD_HEIGHT )
	{			
		sh = m_LCD_HEIGHT - y;			
	}

	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 주어진 xy 좌표에 의한 프레임버퍼에 그릴 시작위치

		rnd = (WRand()%128);
		for( row = sy; row < sy+sh; row++ )
		{
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = sx; col<sx+sw; col++ )
			{				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( pixel_32, &sR, &sG, &sB );
				
				switch (flag)
				{
				case 0:
					mean = (sR+sG+sB)/3;
					if (WRand()%256 < mean)		mean = 0;
					else						mean = 255;
					*( offset_32 +  (col-sx) + offsetY ) = MC_grpGetPixelFromRGB( mean, mean, mean );
					break;
				case 1:
					rR = (sR*(WRand()%255))/255;
					rG = (sG*(WRand()%255))/255;
					rB = (sB*(WRand()%255))/255;
					
					*( offset_32 +  (col-sx) + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
					break;
				}
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 주어진 xy 좌표에 의한 프레임버퍼에 그릴 시작위치

		rnd = (WRand()%128);
		for( row = sy; row < sy+sh; row++ )
		{
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = sx; col<sx+sw; col++ )
			{				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( pixel_16, &sR, &sG, &sB );

				switch (flag)
				{
				case 0:
					mean = (sR+sG+sB)/3;
					if (WRand()%256 < mean)		mean = 0;
					else						mean = 255;
					*( offset_16 +  (col-sx) + offsetY ) = MC_grpGetPixelFromRGB( mean, mean, mean );
					break;
				case 1:
					rR = (sR*(WRand()%255))/255;
					rG = (sG*(WRand()%255))/255;
					rB = (sB*(WRand()%255))/255;

					*( offset_16 +  (col-sx) + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
					break;
				}
			}
		}
	}
}
#endif


/*---------------------------------------------------------------------------*
Description :	ColorDodge
				응용분야 : 빛 계열(전격계 포함)의 효과 및 마법에 가장 적합한 효과입니다.

				알고리즘 : 표현되는 결과물(Dodge)을 위해 필요한 자재를 바탕색(BG)과 덧색
               (SPR)으로 나누었을때 RGB별 공통되는 연산식은 ...

                        Dodge=(BG*MAXDAC)/(MAXDAC-SPR)

               입니다. 이는 바탕색의 DAC가 높으면 높을수록 Dodge 본연의 색상
               이 나와주는 것이며 또한 반대로 바탕색이 어두우면 어두울수록 D
               odge 효과가 약해진다는 것을 살펴볼 수 있습니다. 또한 Dodge 로
               강렬한 표현을 하려면 배경과 마찬가지로 SPR 의 전체적인 밝기가
               높아야만 원래 추구하는 이미지를 전달하게 됩니다.

               주의할점은 Dodge 의 표현상 원래 SPR의 섬세한 이미지 전달을 불
               가능하다는 것입니다. ( SPR 이미지의 심한 왜곡 )


Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_COLORDODGE == 1
void DDrawImageColorDodge(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_ColorDodge(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_ColorDodge(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	Lighten
				응용분야 : SPRITE, 원래의 색상이 필요한 -색의 왜곡없는- 다중 투명 효과.

			  알고리즘 : 일단 알고리즘은 상당히 간단합니다. RGB 별로 BG, SPR 중 큰 DAC
			  값을 취하면 그 값이 바로 Lighten 을 구현하는 값이 됩니다.

				Lighten = max ( BG, SPR )
	
			  이 Lighten 효과는 Table을 필요로 하지 않기 때문에 속도상에 약
			  간의 이점을 가지게 됩니다. 다른 효과들과는 달리 색상의 왜곡이
			  전혀 없으므로 원래 SPR 에 가장 근접한 이미지를 전달할 수가 있
			  다는 장점도 있습니다. 하지만 알고리즘상의 문제점으로 BG 의 밝
			  기에 따라 생략되는 부분도 있을 수 있어 실제 SPR 이미지의 크기
			  중 많은 부분이 축소/생략 되는 경향또한 있습니다.
	  

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_LIGHTEN == 1
void DDrawImageLighten(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Lighten(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Lighten(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif


/*---------------------------------------------------------------------------*
Description :	Screen
				응용분야 : 연기같이 부드럽고-Soft touch- 연한 소재의 효과로 적합합니다.
				(굴뚝에서 피어나는 연기따위...)

				알고리즘 : Lighten의 개량형이라 생각하면 됩니다. 연산식을 살펴보면 ...
  
				TempSum=(MAXDAC-max(BG,SPR))/MAXDAC*min(BG,SPR)
				Screen=max(BG,SPR)+TempSum
			
			  다시 말하면 Lighten은 자신(SPR)의 색상을 과장하지 않지만 Scr
			  een 은 어느정도 (어느정도라지만 일정한 왜곡률을 위한 계산식이
			  있다) 자신의 색상을 높여서 표현하기 때문에 원래 이미지의 축소
			  /생략되는 부분이 없어,실제 구현되는 이미지의 영역도 Lighten보
			  다 넓습니다.
	  

	  

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_SCREEN == 1
void DDrawImageScreen(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Screen(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Screen(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	Difference

				응용분야 : 암흑 마법 발동이나 독, 마비등의 Effect 응용 가능.

				알고리즘 : BG 와 SPR 색상의 편차를 고스란히 대입하는 간단한 알고리즘.

                   Different=(max(BG,SPR)-min(BG,SPR))

               이를 이용하여 텍스쳐 형식으로 캐릭터에게 독립적인 표현이 가능
               하므로 상태 이상시 캐릭터 Status 표현으로도 응용할 수 있습니다.


	  

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_DIFFERENCE == 1
void DDrawImageDifference(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Difference(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Difference(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif


/*---------------------------------------------------------------------------*
Description :	Darken

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_DARKEN == 1
void DDrawImageDarken(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Darken(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Darken(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif


/*---------------------------------------------------------------------------*
Description :	Multiply

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_MULTIPLY == 1
void DDrawImageMultiply(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Multiply(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Multiply(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif


/*---------------------------------------------------------------------------*
Description :	Add Bland

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_ADDBLEND == 1
void DDrawImageAddBlend(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Add(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Add(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	Subtract

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_SUBTRACT == 1
void DDrawImageSubtract(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Subtract(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Subtract(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	Negation

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_NEGATION == 1
void DDrawImageNegation(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Negation(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Negation(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	Exclusion

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_EXCLUSION == 1
void DDrawImageExclusion(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Exclusion(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Exclusion(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	Overlay

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_OVERLAY == 1
void DDrawImageOverlay(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;

	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);

	M_Uint16 sh = ih;
	M_Uint16 sw = iw;

	M_Int32 px, py;

	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;

	M_Int32 offsetY, ry;
	M_Int32 row, col;

	M_Int32 TempSum;


	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치

		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;

			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;

				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Overlay(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치

		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;

			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;

				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Overlay(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	SoftLight

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_SOFTLIGHT == 1
void DDrawImageSoftLight(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_SoftLight(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_SoftLight(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	HardLight

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_HARDLIGHT == 1
void DDrawImageHardLight(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_HardLight(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_HardLight(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	ColorBurn

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_COLORBURN == 1
void DDrawImageColorBurn(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_ColorBurn(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_ColorBurn(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	LinearDodge

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_LINEARDODGE == 1
void DDrawImageLinearDodge(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_LinearDodge(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_LinearDodge(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	LinearBurn

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_LINEARBURN == 1
void DDrawImageLinearBurn(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_LinearBurn(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_LinearBurn(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	LinearLight

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_LINEARLIGHT == 1
void DDrawImageLinearLight(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_LinearLight(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_LinearLight(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	VividLight

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_VIVIDLIGHT == 1
void DDrawImageVividLight(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_VividLight(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_VividLight(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	PinLight

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_PINLIGHT == 1
void DDrawImagePinLight(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_PinLight(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_PinLight(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	HardMix

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_HARDMIX == 1
void DDrawImageHardMix(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_HardMix(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_HardMix(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	Reflect

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_REFLECT == 1
void DDrawImageReflect(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Reflect(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Reflect(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	Glow

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_GLOW == 1
void DDrawImageGlow(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Glow(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Glow(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	Phoenix

Arguments :		img			- 출력할 이미지데이터
				x			- 스크린상의 출력할 위치 (x좌표)
				y			- 스크린상의 출력할 위치 (y좌표)
				alpha		- FLAG_ADD_BLAND값이 1일 경우만 적용
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_PHOENIX == 1
void DDrawImagePhoenix(MC_GrpImage img, M_Int32 x, M_Int32 y, M_Uint8 alpha)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;
	
	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
	
	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	
	M_Int32 px, py;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY, ry;
	M_Int32 row, col;
	
	M_Int32 TempSum;
	
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	
	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_32 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_32, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Phoenix(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_32 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = 0; row < ih; row++ )
		{
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT-1 ) continue;
			
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = 0; col < iw; col++ )
			{
				px = x+col;
				if( px < 0 || px > m_LCD_WIDTH-1 ) continue;
				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다							
				
				MC_grpGetRGBFromPixel( *( offset_16 +  col + offsetY ), &RGB1[0], &RGB1[1], &RGB1[2] );
				MC_grpGetRGBFromPixel( pixel_16, &RGB2[0], &RGB2[1], &RGB2[2] );
				
				ColorBlend_Phoenix(RGB3, RGB1, RGB2);
				
#if FLAG_ADD_BLAND == 1
				RGB3[0] = ( RGB3[0]*alpha + RGB1[0]*(255-alpha) )>>8;
				RGB3[1] = ( RGB3[1]*alpha + RGB1[1]*(255-alpha) )>>8;
				RGB3[2] = ( RGB3[2]*alpha + RGB1[2]*(255-alpha) )>>8;
#endif
				*( offset_16 +  col + offsetY ) = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	랜턴 효과
Arguments :		ImgBG	- 랜턴 이미지
				x		- x좌표
				y		- y좌표
				w		- 랜턴이미지의 가로 크기
				h		- 랜턴이미지의 세로 크기
				R,G,B	- 색상값
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_LANTERN == 1
void DrawLantern(MC_GrpImage Img, int x, int y, int w, int h, M_Int8 R, M_Int8 G, M_Int8 B)
{
	DFillRect(0, 0, m_LCD_WIDTH, y, (void*)MC_grpGetPixelFromRGB(0, 0, 0) );				//상단 가로
	DFillRect(0, y+h, m_LCD_WIDTH, m_LCD_HEIGHT-(y+h), (void*)MC_grpGetPixelFromRGB(0, 0, 0) );	//하단 가로
	DFillRect(0, y, x, h, (void*)MC_grpGetPixelFromRGB(0, 0, 0) );						//왼쪽
	DFillRect(x+w, y, m_LCD_WIDTH - (x+w), h, (void*)MC_grpGetPixelFromRGB(0, 0, 0) );		//오른쪽

	MC_grpSetContext( GetGC(), MC_GRP_CONTEXT_ALPHA_IDX, 60 ); // set blue color
	DFillRect(x, y, w, h, (void*)MC_grpGetPixelFromRGB(R, G, B) );
	MC_grpInitContext( GetGC() );
	
	DrawImage(Img, x, y);
}
#endif

/*---------------------------------------------------------------------------*
Description :	이미지의 색을 반전한다.
Arguments :		ImgBG	- 랜턴 이미지
				x		- x좌표
				y		- y좌표
				w		- 랜턴이미지의 가로 크기
				h		- 랜턴이미지의 세로 크기
				R,G,B	- 색상값
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_INVERT == 0
void DrawInvert(MC_GrpImage img, int x, int y)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;

	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);

	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	M_Int32 sx = 0, sy = 0, dx = x, dy = y;

	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;

	M_Int32 offsetY, ry;
	M_Int32 row, col;


	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );

	//이미지가 화면상에서 완전히 나갔을 때 출력 루틴 종료
	if ( y < -ih)			return;
	if ( y > m_LCD_HEIGHT)	return;
	if ( x < -iw)			return;
	if ( x > m_LCD_WIDTH)	return;

	///< 이미지가 LCD 왼쪽 경계에 걸릴경우
	if( x < 0 )
	{
		sx = -x;			
		dx = 0;

		sw = iw + x;
		if( sw > m_LCD_WIDTH ) ///< 이미지소스가 LCD폭보다 더 크다면
		{
			sw = m_LCD_WIDTH; ///< 이미지소스 참조는 실제 화면에 그려지는 LCD_WIDTH 만큼만 참조한다
		}
	}
	///< 이미지가 LCD 오른쪽에 걸릴경우
	else if( x + iw > m_LCD_WIDTH ) 
	{
		sw = m_LCD_WIDTH - x;
	}


	///< 이미지가 위쪽에 걸릴경우
	if( y < 0 )
	{
		sy = -y;
		dy = 0;

		sh = ih + y;
		if( sh > m_LCD_HEIGHT )
		{
			sh = m_LCD_HEIGHT; ///< 이미지소스 높이가 화면크기보다 크면 실제 화면에 그릴수 있는 LCD_HEIGHT만큼만 참조한다.
		}
	}
	///< 이미지가 아래쪽에 걸릴경우
	else if( y + ih > m_LCD_HEIGHT )
	{			
		sh = m_LCD_HEIGHT - y;			
	}

	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + dx + m_LCD_WIDTH*dy; ///< 프레임버퍼에 그릴 시작위치

		for( row = sy; row < sy+sh; row++ )
		{
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = sx; col < sx+sw; col++ )
			{				
				pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
				
				MC_grpGetRGBFromPixel( pixel_32, &sR, &sG, &sB );
				
				rR = 255 - sR;
				rG = 255 - sG;
				rB = 255 - sB;
				
				*( offset_32 +  (col-sx) + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + dx + m_LCD_WIDTH*dy; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = sy; row < sy+sh; row++ )
		{
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = sx; col < sx+sw; col++ )
			{				
				pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
				
				/*sR = ((pixel_16)>>11);
				sG = ((pixel_16)>>5)&0x3f;
				sB = ((pixel_16)&0x1f);
				
				rR = 255 - sR;
				rG = 255 - sG;
				rB = 255 - sB;
				
				*( offset_16 +  (col-sx) + offsetY ) = ((rR<<11 & 0xf800) | (rG<<5 &0x07e0) | (rB&0x1f));*/

				MC_grpGetRGBFromPixel( pixel_16, &sR, &sG, &sB );
				
				rR = 255 - sR;
				rG = 255 - sG;
				rB = 255 - sB;
				
				*( offset_16 +  (col-sx) + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
			}
		}
	}
}
#endif


/*---------------------------------------------------------------------------*
Description :	블러 효과
Arguments :		ImgBG	- 랜턴 이미지
				x		- x좌표
				y		- y좌표
				val		- 블러값 (값이 클수록 느려진다.) : 최소값을 3이상 넣을것
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_BLUR == 1
void DrawBlur(MC_GrpImage img, int x, int y, int val)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;

	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);

	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	M_Int32 sx = 0, sy = 0, dx = x, dy = y;

	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;

	M_Int32 offsetY, ry;
	M_Int32 row, col, r2, c2;


	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );

	//이미지가 화면상에서 완전히 나갔을 때 출력 루틴 종료
	if ( y < -ih)			return;
	if ( y > m_LCD_HEIGHT)	return;
	if ( x < -iw)			return;
	if ( x > m_LCD_WIDTH)	return;

	///< 이미지가 LCD 왼쪽 경계에 걸릴경우
	if( x < 0 )
	{
		sx = -x;			
		dx = 0;

		sw = iw + x;
		if( sw > m_LCD_WIDTH ) ///< 이미지소스가 LCD폭보다 더 크다면
		{
			sw = m_LCD_WIDTH; ///< 이미지소스 참조는 실제 화면에 그려지는 LCD_WIDTH 만큼만 참조한다
		}
	}
	///< 이미지가 LCD 오른쪽에 걸릴경우
	else if( x + iw > m_LCD_WIDTH ) 
	{
		sw = m_LCD_WIDTH - x;
	}


	///< 이미지가 위쪽에 걸릴경우
	if( y < 0 )
	{
		sy = -y;
		dy = 0;

		sh = ih + y;
		if( sh > m_LCD_HEIGHT )
		{
			sh = m_LCD_HEIGHT; ///< 이미지소스 높이가 화면크기보다 크면 실제 화면에 그릴수 있는 LCD_HEIGHT만큼만 참조한다.
		}
	}
	///< 이미지가 아래쪽에 걸릴경우
	else if( y + ih > m_LCD_HEIGHT )
	{			
		sh = m_LCD_HEIGHT - y;			
	}

	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + dx + m_LCD_WIDTH*dy; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = sy; row < sy+sh; row++ )
		{
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = sx; col < sx+sw; col++ )
			{
				if(col==0 || col == sx+sw-val || row==0 || row== sy+sh-val)
				{
				}
				else
				{
					pixel_32 = *(pFB_32 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
					
					if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
					
					rR = rG = rB = 0;
					for(r2=-1; r2<val-1; r2++)
					{
						for(c2=-1; c2<val-1; c2++)
						{
							MC_grpGetRGBFromPixel( *(pFB_32+(col+c2)+(iw*(row+r2))), &tR, &tG, &tB );
							
							rR += tR;
							rG += tG;
							rB += tB;
						}
					}
					rR = rR / (val*val);
					rG = rG / (val*val);
					rB = rB / (val*val);
					
					*( offset_32 +  (col-sx) + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
				}
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + dx + m_LCD_WIDTH*dy; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = sy; row < sy+sh; row++ )
		{
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = sx; col < sx+sw; col++ )
			{
				if(col==0 || col == sx+sw-val || row==0 || row== sy+sh-val)
				{
				}
				else
				{
					pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
					
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
					
					rR = rG = rB = 0;
					for(r2=-1; r2<val-1; r2++)
					{
						for(c2=-1; c2<val-1; c2++)
						{
							MC_grpGetRGBFromPixel( *(pFB_16+(col+c2)+(iw*(row+r2))), &tR, &tG, &tB );
							
							rR += tR;
							rG += tG;
							rB += tB;
						}
					}
					rR = rR / (val*val);
					rG = rG / (val*val);
					rB = rB / (val*val);
					
					*( offset_16 +  (col-sx) + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
				}
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	커널필터 적용 (부동소수점 연산이 되지 않아 제대로 작동하지 않음)
Arguments :		ImgBG	- 랜턴 이미지
				x		- x좌표
				y		- y좌표
				Kernel	- 커널 필터값
				flag	-	0x01:9로 나누기
							0x10:평균값 더하기
							0x11:평균값 더하기 & 9로나누기
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_KERNEL == 1
void DrawKernel(MC_GrpImage img, int x, int y, int* Kernel, int flag)
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;

	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);

	M_Uint16 sh = ih;
	M_Uint16 sw = iw;
	M_Int32 sx = 0, sy = 0, dx = x, dy = y;

	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;

	M_Int32 offsetY, ry;
	M_Int32 row, col, r2, c2;

	M_Int8  d;
	M_Int32 aR, aG, aB;
	M_Int32 cnt;
	

	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );

	//이미지가 화면상에서 완전히 나갔을 때 출력 루틴 종료
	if ( y < -ih)			return;
	if ( y > m_LCD_HEIGHT)	return;
	if ( x < -iw)			return;
	if ( x > m_LCD_WIDTH)	return;

	///< 이미지가 LCD 왼쪽 경계에 걸릴경우
	if( x < 0 )
	{
		sx = -x;			
		dx = 0;

		sw = iw + x;
		if( sw > m_LCD_WIDTH ) ///< 이미지소스가 LCD폭보다 더 크다면
		{
			sw = m_LCD_WIDTH; ///< 이미지소스 참조는 실제 화면에 그려지는 LCD_WIDTH 만큼만 참조한다
		}
	}
	///< 이미지가 LCD 오른쪽에 걸릴경우
	else if( x + iw > m_LCD_WIDTH ) 
	{
		sw = m_LCD_WIDTH - x;
	}


	///< 이미지가 위쪽에 걸릴경우
	if( y < 0 )
	{
		sy = -y;
		dy = 0;

		sh = ih + y;
		if( sh > m_LCD_HEIGHT )
		{
			sh = m_LCD_HEIGHT; ///< 이미지소스 높이가 화면크기보다 크면 실제 화면에 그릴수 있는 LCD_HEIGHT만큼만 참조한다.
		}
	}
	///< 이미지가 아래쪽에 걸릴경우
	else if( y + ih > m_LCD_HEIGHT )
	{			
		sh = m_LCD_HEIGHT - y;			
	}

	////////////////////////////////////////////////////////	평균값 구하기
	if (flag == 0x01 || flag == 0x11)	d = 9;
	else								d = 1;
	
	for(r2=0; r2<3; r2++)
	{
		for(c2=0; c2<3; c2++)
		{
			Kernel[r2+(c2*3)] /= d;
		}
	}

	if (flag == 0x10 || flag == 0x11)
	{
		if (m_BPP == 32)
		{
		}
		else
		{
			offset_16 = m_pBackFB_16 + dx + m_LCD_WIDTH*dy;
			
			aR = aG = aB = cnt = 0;
			for( row = sy; row < sy+sh; row++ )
			{
				ry = iw*row;
				offsetY = m_LCD_WIDTH*(row-sy);
				
				for( col = sx; col < sx+sw; col++ )
				{
					cnt++;
					MC_grpGetRGBFromPixel( *(pFB_16 + col + ry), &tR, &tG, &tB );
					aR = aR+tR;
					aG = aG+tG;
					aB = aB+tB;
				}
			}
			
			aR /= cnt;
			aG /= cnt;
			aB /= cnt;
		}
	}
	/////////////////////////////////////////////////////////

	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + dx + m_LCD_WIDTH*dy; ///< 프레임버퍼에 그릴 시작위치

		for( row = sy; row < sy+sh; row++ )
		{
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = sx; col < sx+sw; col++ )
			{				
				pixel_32 = *(pFB_32 + col + ry);		///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
				*( offset_32 +  (col-sx) + offsetY ) = pixel_32;
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + dx + m_LCD_WIDTH*dy; ///< 프레임버퍼에 그릴 시작위치
		
		for( row = sy; row < sy+sh; row++ )
		{
			ry = iw*row; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*(row-sy); ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col = sx; col < sx+sw; col++ )
			{
				if(col==0 || col == sx+sw || row==0 || row== sy+sh)
				{
				}
				else
				{
					pixel_16 = *(pFB_16 + col + ry); ///< 이미지소스에서 칼라값 얻어옴
					
					if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다
					
					rR = rG = rB = 0;
					for(r2=-1; r2<2; r2++)
					{
						for(c2=-1; c2<2; c2++)
						{
							MC_grpGetRGBFromPixel( *(pFB_16+(col+c2)+(iw*(row+r2))), &tR, &tG, &tB );
							
							rR = (rR + tR + Kernel[r2+(c2*3)]);
							rG = (rG + tG + Kernel[r2+(c2*3)]);
							rB = (rB + tB + Kernel[r2+(c2*3)]);
						}
					}

					rR += aR;
					rG += aG;
					rB += aB;

					if (rR < 0)			rR = 0;
					else if (rR > 255)	rR = 255;

					if (rG < 0)			rG = 0;
					else if (rG > 255)	rG = 255;

					if (rB < 0)			rB = 0;
					else if (rB > 255)	rB = 255;
					
					*( offset_16 +  (col-sx) + offsetY ) = MC_grpGetPixelFromRGB( rR, rG, rB );
				}
			}
		}
	}
}
#endif


/*---------------------------------------------------------------------------*
Description :	그라데이션 출력
Arguments :		x		- x좌표
				y		- y좌표
				whith	- 가로크기
				height	- 세로크기
				cColor	- 시작컬러(예 - 0xFF0000 : 붉은색)
				eColor	- 마지막컬러
				flag	- 1 : 가로 그라데이션, 0: 세로 그라데이션
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_GRADIENT == 1
void GDrawGradientLine(M_Int32 x, M_Int32 y, M_Int32 whith, M_Int32 height, M_Int32 cColor, M_Int32 eColor, M_Int8 flag)
{
	M_Int32 i, Cr,Cg,Cb;
	M_Int32 Cr1,Cg1,Cb1;
	M_Int32 mr,mg,mb;
	M_Int32 mc;
	
	Cr=(cColor&0xff0000)>>16;
	Cg=(cColor&0xff00)>>8;
	Cb=cColor&0xff;		
	Cr1=(eColor&0xff0000)>>16;
	Cg1=(eColor&0xff00)>>8;
	Cb1=eColor&0xff;

	if(flag)
	{
		for(i = 0; i < whith; i++)
		{
			mr=((Cr1-Cr)*i/(whith)+Cr);
			mg=((Cg1-Cg)*i/(whith)+Cg);
			mb=((Cb1-Cb)*i/(whith)+Cb);
			
			DDrawLine( x+i, y, x+i, y+height, (void*)MC_grpGetPixelFromRGB(mr, mg, mb) );
		}
	}
	else
	{
		for(i = 0; i < height; i++)
		{
			mr=((Cr1-Cr)*i/(height)+Cr);
			mg=((Cg1-Cg)*i/(height)+Cg);
			mb=((Cb1-Cb)*i/(height)+Cb);
			
			DDrawLine( x, y+i, x+whith, y+i, (void*)MC_grpGetPixelFromRGB(mr, mg, mb) );
		}
	}
}
#endif
/*---------------------------------------------------------------------------*
Description :	확장 그라데이션 출력
Arguments :		x		- 그라데이션 시작 x좌표
				y		- 그라데이션 시작 y좌표
				x2		- 그라데이션 종료 x좌표
				y2		- 그라데이션 종료 y좌표
				whith	- 가로크기
				height	- 세로크기
				cColor	- 시작컬러(예 - 0xFF0000 : 붉은색)
				eColor	- 마지막컬러
				flag	- 1 : 가로 그라데이션, 0: 세로 그라데이션
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_EX_GRADIENT == 1
void GDrawExGradientLine(M_Int32 x, M_Int32 y, M_Int32 x2, M_Int32 y2, M_Int32 length, M_Int32 height, M_Int32 cColor, M_Int32 eColor, M_Int8 flag)
{
	M_Int32 i, Cr,Cg,Cb;
	M_Int32 Cr1,Cg1,Cb1;
	M_Int32 mr,mg,mb;
	M_Int32 mc;
	
	Cr=(cColor&0xff0000)>>16;
	Cg=(cColor&0xff00)>>8;
	Cb=cColor&0xff;		
	Cr1=(eColor&0xff0000)>>16;
	Cg1=(eColor&0xff00)>>8;
	Cb1=eColor&0xff;
	
	for(i=0;i<(height>>1);i++)
	{
		mr=(Cr1-Cr)*i/(height>>1)+Cr;
		mg=(Cg1-Cg)*i/(height>>1)+Cg;
		mb=(Cb1-Cb)*i/(height>>1)+Cb;
		
		if(flag)
		{
			DDrawLine( x, y-(height>>1)+i, x2, y2-(height>>1)+i, (void*)MC_grpGetPixelFromRGB(mr, mg, mb) );
			DDrawLine( x, y+(height>>1)-1-i, x2, y2+(height>>1)-1-i, (void*)MC_grpGetPixelFromRGB(mr, mg, mb) );
		}
		else
		{
			DDrawLine( x-(height>>1)+i, y+i, x2-(height>>1)+i, y2, (void*)MC_grpGetPixelFromRGB(mr, mg, mb) );
			DDrawLine( x+(height>>1)-2-i, y, x2+(height>>1)-2-i, y2, (void*)MC_grpGetPixelFromRGB(mr, mg, mb) );
		}
	}
}
#endif
/*---------------------------------------------------------------------------*
Description :	이미지에 기울기를 추가한다.
Arguments :		img			- 출력할 이미지데이터
x			- 스크린상의 출력할 위치 (x좌표)
y			- 스크린상의 출력할 위치 (y좌표)
nw			- 확대/축소 될 가로사이즈
nh			- 확대/축소 될 세로사이즈
z			- 기울기
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_Z_RESIZE == 1
void DDrawImageZReSize( MC_GrpImage img, M_Int32 x, M_Int32 y, M_Int32 nw, M_Int32 nh, M_Int32 z )
{
	M_Uint16* pFB_16;
	M_Uint32* pFB_32;

	M_Uint16 ih = MC_grpGetImageProperty(img, MC_GRP_IMAGE_HEIGHT); 
	M_Uint16 iw = MC_grpGetImageProperty(img, MC_GRP_IMAGE_WIDTH);
    
	M_Int32 srcW, srcH, rx, ry;
	
	M_Uint16* offset_16;
	M_Uint16 pixel_16;
	
	M_Uint32* offset_32;
	M_Uint32 pixel_32;
	
	M_Int32 offsetY;
	M_Int32 row, col, px, py;

	M_Int32 pp = 0;
	
	
	srcW = iw;
	srcH = ih;
	
	if(m_BPP == 32)	pFB_32 = (M_Uint32*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );
	else			pFB_16 = (M_Uint16*)MC_GRP_GET_FRAME_BUFFER_POINTER( MC_grpGetImageFrameBuffer(img) );

	if(m_BPP == 32)
	{
		offset_32 = m_pBackFB_32 + x + m_LCD_WIDTH*y; ///< 주어진 xy 좌표에 의한 프레임버퍼에 그릴 시작위치

		for( row=0; row<nh; row++ )
		{		
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;
			
			ry = (srcH*row/nh)*iw; ///< 참조할 이미지소스의 y좌표
			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset
			
			for( col=0; col<nw; col++ )
			{		
				px = x+col;			
				if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
				
				rx = srcW*col/nw; ///< 참조할 이미지소스의 x좌표
				pixel_32 = *( pFB_32  + rx + ry ); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_32 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
				
				*( offset_32+col + offsetY ) = pixel_32;
			}
		}
	}
	else
	{
		offset_16 = m_pBackFB_16 + x + m_LCD_WIDTH*y; ///< 주어진 xy 좌표에 의한 프레임버퍼에 그릴 시작위치
		
		for( row=0; row<nh; row++ )
		{		
			py = y+row;
			if( py < 0 || py > m_LCD_HEIGHT_M1 ) continue;
			
			//pp += (z*nh)/nh;
			//pp = (z*nh)/nh;
			//pp+=(z);
			//pp = nh+(row*100)/nh;
			/*if (row%z == 0)
			{
				pp++;
				//pp+=20;
			}*/

			//pp+=(z);
			//pp+=(z*fcosTbl[row%18]);
			//pp = (srcH*fcosTbl[row%18]);

			//pp = srcH-row;
			//pp = row;
			//pp = srcH;

			//ry = (srcH*row/nh)*iw; ///< 참조할 이미지소스의 y좌표
			//ry = (srcH*row/nh)*iw*(z*fsinTbl[3]);
			//ry = (srcH*row/(pp/100))*iw;
			//ry = (srcH*row/((pp/100)+1))*iw;
			//ry = (srcH*row/(nh/z))*iw;

			//ry = (srcH*row/pp)*iw;
			//ry = (pp*row/nh)*iw;
			//ry = ((pp/100)*row/nh)*iw;
			//ry = (srcH*fcosTbl[row%36]/nh)*iw;

			pp++;
			ry = (pp*row/nh)*iw;

			offsetY = m_LCD_WIDTH*row; ///< 프레임버퍼에 그릴 y좌표 offset

			
			
			for( col=0; col<nw; col++ )
			{		
				px = x+col;			
				if( px < 0 || px > m_LCD_WIDTH_M1 ) continue;
				
				rx = srcW*col/nw; ///< 참조할 이미지소스의 x좌표
				pixel_16 = *( pFB_16  + rx + ry ); ///< 이미지소스에서 칼라값 얻어옴
				if( pixel_16 == COLOR_TRANS ) continue; ///< 투명칼라면 그리지 않는다			
				
				*( offset_16 + col + offsetY ) = pixel_16;
			}
		}
	}
}
#endif


/*---------------------------------------------------------------------------*
Description :	전체화면의 색을 반전시킨다.
Arguments :		void
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_FILL_INVERT == 1
void DFillInvertRect(M_Int32 x, M_Int32 y, M_Int32 width, M_Int32 height)
{
	if(m_BPP == 32)		DFillInvertRect32(*m_offScreenBuffer, x, y, width, height);
	else				DFillInvertRect16(*m_offScreenBuffer, x, y, width, height);
}

/*---------------------------------------------------------------------------*
Description :	DFillInvertRect 16bit컬러 버전
Arguments :		...
Returns :		void
*---------------------------------------------------------------------------*/
void DFillInvertRect16(MC_GrpFrameBuffer g, M_Int32 x, M_Int32 y, M_Int32 width, M_Int32 height)
{
	M_Uint16* dst, *saveDst;
	
	M_Int32 i, j;
	M_Int32 curDesX, curDesY;
	
	M_Int32 rgb1[3];
	M_Int32 rgb2[3];
	
	M_Int16 destCol;
	
	if(x<0) 
	{
		width+=x;
		x = 0;
	}
	if(x+width>=m_LCD_WIDTH) 
	{
		width = m_LCD_WIDTH-x;
	}
	
	dst = (M_Uint16*)(MC_GRP_GET_FRAME_BUFFER_POINTER(g));
	
	dst += (x+y*m_LCD_WIDTH);
	saveDst = dst;
	
	curDesX = x;
	curDesY = y;
	
	for(j=0;j<height;j++)
	{
		curDesX = x;
		
		for(i=0;i<width;i++)
		{
			if(curDesX >= 0 && curDesY >= 0 && curDesX < m_LCD_WIDTH && curDesY < m_LCD_HEIGHT)
			{
				MC_grpGetRGBFromPixel( *dst, &rgb2[0], &rgb2[1], &rgb2[2] );
				
				rgb1[0] = 255 - rgb2[0];
				rgb1[1] = 255 - rgb2[1];
				rgb1[2] = 255 - rgb2[2];
				
				destCol = MC_grpGetPixelFromRGB(rgb1[0], rgb1[1], rgb1[2]);
				*dst = destCol;
			}
			dst++;
		}
		dst= saveDst + m_LCD_WIDTH*(j+1);
		curDesY++;
	}
}

/*---------------------------------------------------------------------------*
Description :	DFillInvertRect 32bit컬러 버전
Arguments :		...
Returns :		void
*---------------------------------------------------------------------------*/
void DFillInvertRect32(MC_GrpFrameBuffer g, M_Int32 x, M_Int32 y, M_Int32 width, M_Int32 height)
{
	M_Int16 i , j ;
	M_Uint32* dst;
	M_Int32 rgb1[3];
	M_Int32 rgb2[3];
	M_Int32 destCol;
	
	dst = (M_Uint32*)(MC_GRP_GET_FRAME_BUFFER_POINTER(g));
	
	for(i=0; i < height ; i++)
	{
		for(j=0;j<width;j++)
		{
			if(j==0 || j == width-1 || i==0 || i== height -1)
			{
				
			}
			else
			{
				MC_grpGetRGBFromPixel( *dst, &rgb2[0], &rgb2[1], &rgb2[2] );
				
				rgb1[0] = 255 - rgb2[0];
				rgb1[1] = 255 - rgb2[1];
				rgb1[2] = 255 - rgb2[2];
				
				destCol = MC_grpGetPixelFromRGB(rgb1[0], rgb1[1], rgb1[2]);
				*dst = destCol;
			}
			dst++;
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	전체화면에 블로효과 적용
Arguments :		void
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_FILL_BLUR == 1
void DFillBlurRect(void)
{
	if(m_BPP == 32)		DFillBlurRect32(*m_offScreenBuffer, 0, 0, m_LCD_WIDTH, m_LCD_HEIGHT);
	else				DFillBlurRect16(*m_offScreenBuffer, 0, 0, m_LCD_WIDTH, m_LCD_HEIGHT);
}

/*---------------------------------------------------------------------------*
Description :	DFillBlurRect 16bit컬러 버전
Arguments :		...
Returns :		void
*---------------------------------------------------------------------------*/
void DFillBlurRect16(MC_GrpFrameBuffer g, M_Int32 x, M_Int32 y, M_Int32 width, M_Int32 height)
{
	M_Int16 i , j , k , l;
	M_Uint16* dst;
	M_Int32 rgb1[3];
	M_Int32 rgb2[3];
	M_Int16 destCol;
	dst = (M_Uint16*)(MC_GRP_GET_FRAME_BUFFER_POINTER(g));
	
	for(i=0; i < height ; i++)
	{
		for(j=0;j<width;j++)
		{
			if(j==0 || j == width-1 || i==0 || i== height -1)
			{
				
			}
			else
			{
				rgb1[0] = rgb1[1] = rgb1[2] = 0;
				for(k=-1;k<2;k++)
				{
					for(l=-1;l<2;l++)
					{
						MC_grpGetRGBFromPixel( *(dst+((i+k)*m_LCD_WIDTH)+j+l), &rgb2[0], &rgb2[1], &rgb2[2] );
						
						rgb1[0] += rgb2[0];
						rgb1[1] += rgb2[1];
						rgb1[2] += rgb2[2];
					}
				}
				rgb1[0] = rgb1[0] / 9;
				rgb1[1] = rgb1[1] / 9;
				rgb1[2] = rgb1[2] / 9;
				
				destCol = MC_grpGetPixelFromRGB(rgb1[0], rgb1[1], rgb1[2]);
				*(dst+i*m_LCD_WIDTH+j) = destCol;
			}
		}
	}
}

/*---------------------------------------------------------------------------*
Description :	DFillBlurRect 32bit컬러 버전
Arguments :		...
Returns :		void
*---------------------------------------------------------------------------*/
void DFillBlurRect32(MC_GrpFrameBuffer g, M_Int32 x, M_Int32 y, M_Int32 width, M_Int32 height)
{
	M_Int16 i , j , k , l;
	M_Uint32* dst;
	M_Int32 rgb1[3];
	M_Int32 rgb2[3];
	M_Int32 destCol;
	dst = (M_Uint32*)(MC_GRP_GET_FRAME_BUFFER_POINTER(g));
	
	for(i=0; i < height ; i++)
	{
		for(j=0;j<width;j++)
		{
			if(j==0 || j == width-1 || i==0 || i== height -1)
			{
				
			}
			else
			{
				rgb1[0] = rgb1[1] = rgb1[2] = 0;
				for(k=-1;k<2;k++)
				{
					for(l=-1;l<2;l++)
					{
						MC_grpGetRGBFromPixel( *(dst+((i+k)*m_LCD_WIDTH)+j+l), &rgb2[0], &rgb2[1], &rgb2[2] );
						
						rgb1[0] += rgb2[0];
						rgb1[1] += rgb2[1];
						rgb1[2] += rgb2[2];
					}
				}
				rgb1[0] = rgb1[0] / 9;
				rgb1[1] = rgb1[1] / 9;
				rgb1[2] = rgb1[2] / 9;
				
				destCol = MC_grpGetPixelFromRGB(rgb1[0], rgb1[1], rgb1[2]);
				*(dst+i*m_LCD_WIDTH+j) = destCol;
			}
		}
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	지정된 화면에 알파값이 포함된 사각형을 그린다.
Arguments :		...
Returns :		void
*---------------------------------------------------------------------------*/
#if DRAW_FILL_RECTEX == 1
void DFillRectEx(M_Int32 dstX, M_Int32 dstY, M_Int32 width, M_Int32 height, M_Int32 color, M_Uint16 alpha)
{
	if(m_BPP == 32)		DFillRectEx32(*m_offScreenBuffer, dstX, dstY, width, height, color, alpha);
	else				DFillRectEx16(*m_offScreenBuffer, dstX, dstY, width, height, color, alpha);
}

/*---------------------------------------------------------------------------*
Description :	DFillRectEx 16bit컬러 버전
Arguments :		...
Returns :		void
*---------------------------------------------------------------------------*/
void DFillRectEx16(MC_GrpFrameBuffer g, M_Int32 dstX, M_Int32 dstY, M_Int32 width, M_Int32 height, M_Int32 color, M_Uint16 alpha)
{
	M_Uint16  color16;
	M_Uint16* dst, *saveDst;
	
	M_Int32 i, j,ii;
	M_Int32 curDesX, curDesY;
	
	M_Int32 rgb1[3];
	M_Int32 rgb2[3];
	
	M_Uint16 destDepth;
	if(alpha<0||alpha>255) alpha = 256;
	if(dstX<0) 
	{
		width+=dstX;
		dstX = 0;
	}
	if(dstX+width >= m_LCD_WIDTH) 
	{
		width = m_LCD_WIDTH-dstX;
	}
	
	destDepth = 256-alpha;
	
	dst = (M_Uint16*)(MC_GRP_GET_FRAME_BUFFER_POINTER(g));
	
	
	rgb1[0] = (color>>16)&0xff ;
	rgb1[1] = (color>>8)&0xff ;
	rgb1[2] = (color&0xff);
	
	color16 = MC_grpGetPixelFromRGB( rgb1[0], rgb1[1], rgb1[2] );
	
	dst += (dstX+dstY*m_LCD_WIDTH);
	saveDst = dst;
	
	curDesX = dstX;
	curDesY = dstY;
	
	for(j=0;j<height;j++)
	{
		curDesX = dstX;		
		
		for(i=0;i<width;i++)
		{
			if(curDesX >= 0 && curDesY >= 0 && curDesX < m_LCD_WIDTH && curDesY < m_LCD_HEIGHT)
			{
				if (alpha == 0)
				{
				}
				else if (alpha == HALF_ALPHA)
				{
					MC_grpGetRGBFromPixel( *dst, &rgb2[0], &rgb2[1], &rgb2[2] );
					
					ColorBlend_Average(RGB3, RGB1, RGB2);

					*dst = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
				}
				else if(alpha<256)
				{
					MC_grpGetRGBFromPixel( *dst, &rgb2[0], &rgb2[1], &rgb2[2] );
					
					*dst = MC_grpGetPixelFromRGB( 
						( rgb1[0]*alpha + rgb2[0]*(255-alpha) )>>8, 
						( rgb1[1]*alpha + rgb2[1]*(255-alpha) )>>8, 
						( rgb1[2]*alpha + rgb2[2]*(255-alpha) )>>8 );
				}
				else
				{
					*dst = color16;
				}
			}
			dst++;
		}
		dst= saveDst + m_LCD_WIDTH*(j+1);
		curDesY++;
	}
}

/*---------------------------------------------------------------------------*
Description :	DFillRectEx 32bit컬러 버전
Arguments :		...
Returns :		void
*---------------------------------------------------------------------------*/
void DFillRectEx32(MC_GrpFrameBuffer g, M_Int32 dstX, M_Int32 dstY, M_Int32 width, M_Int32 height, M_Int32 color, M_Uint16 alpha)
{
	M_Uint32  color32;
	M_Uint32* dst, *saveDst;
	
	M_Int32 i, j,ii;
	M_Int32 curDesX, curDesY;
	
	M_Int32 rgb1[3];
	M_Int32 rgb2[3];
	
	M_Uint32 destDepth;
	if(alpha<0||alpha>255) alpha = 256;
	if(dstX<0) 
	{
		width+=dstX;
		dstX = 0;
	}
	if(dstX+width >= m_LCD_WIDTH) 
	{
		width = m_LCD_WIDTH-dstX;
	}
	
	destDepth = 256-alpha;
	
	dst = (M_Uint32*)(MC_GRP_GET_FRAME_BUFFER_POINTER(g));
	
	
	rgb1[0] = (color>>16)&0xff ;
	rgb1[1] = (color>>8)&0xff ;
	rgb1[2] = (color&0xff);
	
	color32 = MC_grpGetPixelFromRGB( rgb1[0], rgb1[1], rgb1[2] );
	
	dst += (dstX+dstY*m_LCD_WIDTH);
	saveDst = dst;
	
	curDesX = dstX;
	curDesY = dstY;
	
	for(j=0;j<height;j++)
	{
		curDesX = dstX;		
		
		for(i=0;i<width;i++)
		{
			if(curDesX >= 0 && curDesY >= 0 && curDesX < m_LCD_WIDTH && curDesY < m_LCD_HEIGHT)
			{
				if (alpha == 0)
				{
				}
				else if (alpha == HALF_ALPHA)
				{
					MC_grpGetRGBFromPixel( *dst, &rgb2[0], &rgb2[1], &rgb2[2] );
					
					ColorBlend_Average(RGB3, RGB1, RGB2);
					
					*dst = MC_grpGetPixelFromRGB( RGB3[0], RGB3[1], RGB3[2] );
				}
				else if(alpha<256)
				{
					MC_grpGetRGBFromPixel( *dst, &rgb2[0], &rgb2[1], &rgb2[2] );
					
					*dst = MC_grpGetPixelFromRGB( 
						( rgb1[0]*alpha + rgb2[0]*(255-alpha) )>>8, 
						( rgb1[1]*alpha + rgb2[1]*(255-alpha) )>>8, 
						( rgb1[2]*alpha + rgb2[2]*(255-alpha) )>>8 );
				}
				else
				{
					*dst = color32;
				}
			}
			dst++;
		}
		dst= saveDst + m_LCD_WIDTH*(j+1);
		curDesY++;
	}
}
#endif

/*---------------------------------------------------------------------------*
Description :	폰트를 설정하는 함수 굴림을 기본으로 설정하고 만약 폰에 해당 폰트가 없을 경우 고딕으로 그렇지 못할 경우 기본 시스템 폰트를 사용한다.
Arguments :		void
Returns :		void
*---------------------------------------------------------------------------*/
void SetFontInfo(void)
{
	m_fntID = OEMC_grpGetFontEx("굴림", MC_GRP_FT_SIZE_MEDIUM, MC_GRP_FT_STYLE_PLAIN);
	
	if( m_fntID == M_E_NOTSUP)
	{
		m_fntID = OEMC_grpGetFontEx("고딕", MC_GRP_FT_SIZE_MEDIUM, MC_GRP_FT_STYLE_PLAIN);
	}
	
	if( m_fntID == M_E_NOTSUP ) // 확장폰트로 제공되지 않는 폰트인경우
	{
		// OEM 폰트를 사용한다. (기본 시스템폰트)
		m_fntID = MC_grpGetFont(MC_GRP_FT_FACE_SYSTEM, MC_GRP_FT_SIZE_MEDIUM, MC_GRP_FT_STYLE_PLAIN);
	}
}

/*---------------------------------------------------------------------------*
Description :	문자열을 크기별로 맞추어 그려줄때 설정하는 함수로 그려주기 전 한번만 설정해주고 설정 내용이 변경되면 다시 호출해준다.
Arguments :		seek_start	- 몇 바이트부터 시작하게 할 것인지를 설정(처음부터 그릴 것이면, 0) - 문자열의 시작위치
				wdx			- 한줄에 그려줄 텍스트의 가로 크기
				ldy			- 줄과 줄 사이의 여백 크기 (자간)
				max_line	- 최대 라인수를 지정한다. 만약 텍스트가 지정한 라인수를 넘긴다면, Key_StrEx() 함수를 키처리부에
                              삽입해야 한다.
				vRGB		- 폰트의 색상 값. (0xFFFFFF)
Returns :		void
*---------------------------------------------------------------------------*/
void Set_StrEx(M_Int32 seek_start, M_Int32 wdx, M_Int32 ldy, M_Int32 max_line, M_Int32 vRGB)
{
	lStr.w_seek_start = seek_start;	
	lStr.max_dx = wdx;
	lStr.max_line_count = max_line;
	lStr.w_dy = ldy; 
	lStr.vRGB = vRGB;
}

/*---------------------------------------------------------------------------*
Description :	문자열의 시작 바이트를 설정한 처음 부분으로 리셋한다.
Arguments :		void
Returns :		void
*---------------------------------------------------------------------------*/
void ResetSeek_StrEx(void)
{
	lStr.w_seek = lStr.w_seek_tmp = lStr.w_seek_start;
}

/*---------------------------------------------------------------------------*
Description :	만약 페이지가 여러 페이지로 나뉠때 페이지의 전환을 처리해주는 함수 키에 처리를 해주어야 함.
Arguments :		void
Returns :		TRUE	- 해당 문자열을 끝까지 모두 그렸을 경우 넘어온다.
				FALSE	- 해당 문자열을 끝까지 모두 그리지 못했을 경우 넘어오면, 이때 다음페이지로 전환이 된다.
*---------------------------------------------------------------------------*/
M_Boolean Key_StrEx(void)
{
	if(lStr.w_seek_tmp == -1) //라인 종료
	{
		lStr.w_seek = lStr.w_seek_tmp = lStr.w_seek_start;
		return TRUE;		
	}
	else
	{
		lStr.w_seek = lStr.w_seek_tmp;
		return FALSE;
	}
}

/*---------------------------------------------------------------------------*
Description :	텍스트를 그려주는 함수, Set_StrEx()에서 설정한 내용으로 텍스트를 지정한 좌표에 그려준다.
Arguments :		str		- 문자열
				x		- 문자열을 그려줄 x 좌표
				y		- 문자열을 그려줄 y 좌표
Returns :		void

사용법

  void main()
  {
	if(state == 0)
	{
		//텍스트를 그려주기 전 상태 
	}
	else if(state == 1)
	{
		Set_StrEx(0, 100, 15, 2, 0xFFFFFF);	//Draw_StrEx() 위에 해줄 필요는 없다. 설정이 변경되지 않는다면 Draw_StrEx()그리기 전 한번만 호출해주면 됨.
		Draw_StrEx("가나다라 마바사 아 자 차카 타파하~ 릴리리맘보 앗싸!! 오예~ 피피피피~", 10, 10);
	}
	else if(state == 2)
	{
		Draw_StrEx("룰루랄라 앗싸리 앗싸! 이얍! ", 10, 10);
	}
	}

	void key()
	{	
		if(state == 0)
		{
			SetFontInfo();					// 텍스트를 그려주기 전에 기본 폰트를 설정한다.
			ResetSeek_StrEx(); 				// Draw_StrEx()을 처음 부터 그리기 위해 시작 BYTE를 초기화 할 필요가 있다. 처음부터 그려주려 할때 한번 호출되어야 함.
			state = 1;	
		}
		else if(state == 1)
		{
			if(Key_StrEx() == TRUE)	//해당 라인의 문자열의 끝까지 모두 그려주었을 경우 TRUE가 넘어온다. FALSE가 넘어오면 끝까지 표시를 하지 못한 것으로 다음 페이지로 이동하게 된다.
			{
				ResetSeek_StrEx(); 			// Draw_StrEx()을 처음 부터 그리기 위해 시작 BYTE를 초기화 할 필요가 있다. 처음부터 그려주려 할때 한번 호출되어야 함.
				Set_StrEx(0, 200, 15, 3, 0xFFFFFF);	//Draw_StrEx()을 호출하기전 한번 호출한다.
				state = 2;
			}
		}
		
	}
*---------------------------------------------------------------------------*/
void Draw_StrEx(char* str, int x, int y)
{
	M_Int32		len		= 0;
	M_Int32		seek		= 0;
	M_Int32		offset		= 0;
	M_Int16		str_end		= 0;
	M_Char		s[64];
	M_Int32     line_count  = 0;
	M_Int32		tw;
	M_Int32		w_type = 0;
	
	MC_grpSetContext(m_gc, MC_GRP_CONTEXT_FG_PIXEL_IDX, (void*)MC_grpGetPixelFromRGB((lStr.vRGB&0x00ff0000)>>16, (lStr.vRGB&0x0000ff00)>>8, lStr.vRGB&0x000000ff) );
	MC_grpSetContext(m_gc, MC_GRP_CONTEXT_FONT_IDX,		(void*)m_fntID);
	
	len = strlen(str);
	
	seek = offset = lStr.w_seek;
	
	while(len > seek)
	{
		str_end = FALSE; 
		
		line_count++;
		
		if(line_count> lStr.max_line_count)
		{
			break;
		}
		
		while( len > seek)
		{
			if( *(str+seek) == '$')
			{
				++seek;
				str_end = 1;
				break;
			}
			
			if(offset == seek)
			{
				if( *(str+seek) == ' ')
				{			
					seek ++;
					offset = seek;
				}
			}
			
			if( (*(str + seek) & 0xff) > 0x80 )
			{
				seek += 2;
				w_type = 2;
			}
			else
			{
				seek   += 1;
				w_type =  1;
			}
			
			memset( s, 0, 64 );
			memcpy( s, str + offset, seek-offset);
			
			tw = MC_grpGetStringWidth(m_fntID, (const char*)s, -1);
			
			if(lStr.max_dx < tw)
			{
				seek -= w_type;
				break;
			}
		}
		
		memset( s, 0, 64 );
		memcpy( s, str + offset, seek-offset-str_end );
		offset = seek;
		
		DrawString( x, y, s, (void*) MC_grpGetPixelFromRGB((lStr.vRGB&0x00ff0000)>>16, (lStr.vRGB&0x0000ff00)>>8, lStr.vRGB&0x000000ff) );
		
		y += lStr.w_dy;
	}
	
	if(str_end == 1)	lStr.w_seek_tmp = -1;
	else				lStr.w_seek_tmp = seek;
}