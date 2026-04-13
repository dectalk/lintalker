
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "WavInOut.h"
#include "resource.h"

#ifndef __SPEECHSAY__
	#include "MT4.h"
#endif

#ifndef __SPEECHMACINTOSH__
	#include "Macintosh.h"
#endif


HINSTANCE hInst;   // current instance

LPCTSTR lpszAppName = "WinTalker";
LPCTSTR lpszTitle   = "WinTalker"; 

shellVarPtr		gInstanceStorage;
unsigned long	gComponentRefcon;
short			gDoDT;
short			gSpeechIsDone;
char 			msg[MSG_LEN+1];
short			g_CurrentVoice;
short			gSpeechIsActive;

char	*voiceNames[] =
{
	"Fred",
	"Kathy",
	"Princess",
	"Junior",
	"Ralph",
	"Whisper",
	"Zarvox",
	"Trinoids",
	"Bubbles",
	"Boing",
	"Bells",
	"Hysterical",
	"Deranged",
	"GoodNews",
	"BadNews",
	"PipeOrgan",
	"Cellos"
};	


HINSTANCE 		hInst;
char			textBuf[255];

HWND  	speakButton;

HWND  	listCntrl;
HWND  	listCntrlF;

HWND  	editCntrl;
HWND  	editCntrlF;

HBRUSH	backColorBrush;

HBITMAP			g_BackBitMap;


// global variables
//.................

enum Status
{
  StatusOkay,
  StatusError,
  StatusDone,
} eStatus;


#define	WM_START_SPEECH		(WM_USER +1)
#define USR_INBLOCK           (WM_USER+101)
#define USR_OUTBLOCK          (WM_USER+102)

#define kSpeakButton        0
#define kVoiceCntrl         1
#define kEditCntrl          2
#define kVoiceLabel         3
#define kEditLabel          4

#define	kFredVoice			0

//-----------------------------
// EDIT
//-----------------------------
#define	kEdit_X		42
#define	kEdit_Y		89
#define	kEdit_WD	((tm.tmAveCharWidth * 32) + GetSystemMetrics (SM_CXVSCROLL))
//#define	kEdit_HT	((tm.tmHeight * 8) + GetSystemMetrics (SM_CXHSCROLL))
#define	kEdit_HT	((tm.tmHeight * 9) -5)

// Lable
#define	kEditL_X	kEdit_X + 77
#define	kEditL_Y	kEdit_Y - 22

	
//-----------------------------
// LIST
//-----------------------------
#define	kList_X		(kEdit_X + 333)
#define	kList_Y		kEdit_Y
#define	kList_HT	(tm.tmHeight * 9)
#define	kList_WD	(tm.tmAveCharWidth * 16 + GetSystemMetrics (SM_CXVSCROLL))

// Lable
#define	kListL_X	kList_X + 49
#define	kListL_Y	kEditL_Y


//-----------------------------
// SPEAK BUTTON
//-----------------------------
#define	kSpeak_WD	(9 * tm.tmAveCharWidth)
#define	kSpeak_X	(kEdit_X + ((kEdit_WD) >> 1) - (kSpeak_WD >> 1))
#define	kSpeak_Y	(kEdit_Y + kEdit_HT) + 47


//-----------------------------
// Forward
//-----------------------------
BOOL RegisterWin95( CONST WNDCLASS* lpwc );
void	CloseSpeech ();



//----------------------
// Imports
//----------------------
extern	long 		_OpenSpeech		(HWND hWnd);
extern	long 		_CloseSpeech	(shellVarPtr svv);
extern	void   		SM_CallBack 	(shellVarPtr svv, LPWAVEHDR lpwh);
extern	long 		_UseVoice ( shellVarPtr svv, short index );
extern	long 		_SpeakBuffer (shellVarPtr svv, Ptr textBuf, long byteLen, long controlFlags); 



#define	kFrameWind_X	20
#define	kFrameWind_Y	20
#define	kFrameWind_WD	600
#define	kFrameWind_HT	400



int APIENTRY WinMain	( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                      		LPTSTR lpCmdLine, int nCmdShow)
{
   MSG      msg;
   HWND     hWnd; 
   WNDCLASS wc;

	//backColorBrush = CreateSolidBrush (RGB (0xD0, 0xD0, 0xD0));
	
   wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
   wc.lpfnWndProc   = (WNDPROC)WndProc;       
   wc.cbClsExtra    = 0;                      
   wc.cbWndExtra    = 0;                      
   wc.hInstance     = hInstance;              
   wc.hIcon         = LoadIcon (hInstance, lpszAppName); 
   wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wc.hbrBackground = (HBRUSH) GetStockObject (LTGRAY_BRUSH) ;
   //wc.hbrBackground = backColorBrush;
   //wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
   wc.lpszMenuName  = MAKEINTRESOURCE (IDR_MENU2);              
   wc.lpszClassName = lpszAppName;              

  if ( !RegisterWin95( &wc ) )
     return( FALSE );

   hInst = hInstance; 

   hWnd = CreateWindowEx (	WS_EX_OVERLAPPEDWINDOW, 
							lpszAppName, 
							lpszTitle,    
							WS_OVERLAPPEDWINDOW, 
							kFrameWind_X, kFrameWind_Y, 
							kFrameWind_WD, kFrameWind_HT,  
							NULL,              
							NULL,              
							hInstance,         
							NULL               
						  );

   if ( !hWnd ) 
      return( FALSE );

	hInst = hInstance;

#if 0
 				//------------------------------
				// Create VOICE Label
				//------------------------------
        	    listCntrlL = CreateWindow ("static", "Voice",
                              WS_CHILD | WS_VISIBLE | SS_LEFT,
                              kListL_X, kListL_Y, 40, 16,
                              hWnd, (HMENU) kVoiceLabel, hInst, NULL) ;
                              
 				//------------------------------
				// Create VOICE Label
				//------------------------------
        	    editCntrlL = CreateWindow ("static", "Text-to-Speech",
                              WS_CHILD | WS_VISIBLE | SS_LEFT,
                              kEditL_X, kEditL_Y, 104, 20,
                              hWnd, (HMENU) kEditLabel, hInst, NULL) ;
#endif
               


   ShowWindow( hWnd, nCmdShow ); 
   UpdateWindow( hWnd );
   PostMessage (hWnd, WM_START_SPEECH, 0, 0);
   
         

   while( GetMessage( &msg, NULL, 0, 0) )   
   {
      TranslateMessage( &msg ); 
      DispatchMessage( &msg );  
   }

   return( msg.wParam ); 
}


BOOL RegisterWin95( CONST WNDCLASS* lpwc )
{
	WNDCLASSEX wcex;

   wcex.style         = lpwc->style;
   wcex.lpfnWndProc   = lpwc->lpfnWndProc;
   wcex.cbClsExtra    = lpwc->cbClsExtra;
   wcex.cbWndExtra    = lpwc->cbWndExtra;
   wcex.hInstance     = lpwc->hInstance;
   wcex.hIcon         = lpwc->hIcon;
   wcex.hCursor       = lpwc->hCursor;
   wcex.hbrBackground = lpwc->hbrBackground;
   wcex.lpszMenuName  = lpwc->lpszMenuName;
   wcex.lpszClassName = lpwc->lpszClassName;

   // Added elements for Windows 95.
   //...............................
   wcex.cbSize = sizeof(WNDCLASSEX);
   wcex.hIconSm = LoadImage(wcex.hInstance, lpwc->lpszClassName, 
                            IMAGE_ICON, 16, 16,
                            LR_DEFAULTCOLOR );
			
   return RegisterClassEx( &wcex );
}





void	PaintBkgrndPat (HDC hDC, RECT *r)
{
	HDC			memDC;
	//long		width, length;
	long		x,y;
	long		wd, ht;

	//----------------------------
	// Paint background pattern
	//----------------------------
	//width = r->right - r->left;
	//length = r->bottom - r->top;

	memDC = CreateCompatibleDC (hDC);
	SelectObject (memDC, g_BackBitMap);
	for (y = r->top; y < r->bottom; y += 32)
		{
		ht = r->bottom - y;
		if (ht > 32)
			ht = 32;
		for (x = r->left; x < r->right; x += 32)
			{
			wd = r->right - x;
			if (wd > 32)
				wd = 32;
			BitBlt (hDC, x, y, wd, ht, memDC, 0, 0, SRCCOPY);
			}
		}
	DeleteDC (memDC);
}






void CenterDialog (HWND hWnd) 
{

	RECT			cr, pr;
	long			cWD, cHT, pWD, pHT;

	GetWindowRect (hWnd, &cr);
	cWD = cr.right - cr.left;
	cHT = cr.bottom - cr.top;
	GetWindowRect (GetParent(hWnd), &pr);
	pWD = pr.right - pr.left;
	pHT = pr.bottom - pr.top;

	MoveWindow	(hWnd, pr.left + ((pWD - cWD) / 2), 
				pr.top + ((pHT - cHT) / 2),
				cWD, cHT, true);
}





/*

void	SpeakToFile ()
{
	short			result;
	char			wavFileName[256] = "Untitled.wav";
	char			wavFileTitle[256] = "";
	char			openFilter[] =	"Audio File (*.WAV)\0*.wav\0";
	OPENFILENAME	ofn;


	if ((m_pAudioData->m_sample_count == 0) || (m_pAudioData->m_pData == NULL))
		{
		MessageBox ( 0, "Nothing to save.", NULL, MB_OK| MB_ICONEXCLAMATION);
		goto EXIT_SAVE;
		}


	memset (&ofn, 0, sizeof(ofn));
	ofn.lStructSize			= sizeof (OPENFILENAME) ;
	//ofn.hwndOwner         = g_WaveEditWnd ;
	ofn.hInstance			= m_hInstance;
	ofn.lpstrCustomFilter	= NULL ;
	ofn.nMaxCustFilter		= 0 ;
	ofn.nFilterIndex		= 0 ;
	ofn.lpstrFile			= &wavFileName[0] ;          // Set in Open and Close functions
	ofn.nMaxFile			= sizeof(wavFileName);
	ofn.lpstrFileTitle		= &wavFileTitle[0] ;          // Set in Open and Close functions
	ofn.nMaxFileTitle		= sizeof(wavFileTitle);
	ofn.lpstrInitialDir		= NULL ;
	ofn.lpstrTitle			= NULL ;
	ofn.Flags				= OFN_OVERWRITEPROMPT ;
	ofn.nFileOffset			= 0 ;
	ofn.nFileExtension		= 0 ;
	ofn.lpstrDefExt			= "wav" ;
	ofn.lCustData			= 0L ;
	ofn.lpfnHook			= NULL ;
	ofn.lpTemplateName		= NULL ;
	ofn.lpstrFilter			= &openFilter[0];

	//---------------------------------------
	// Get the WAV file name
	//---------------------------------------
	result = (short)GetSaveFileName (&ofn);
	if (result)
		{
		WavFileInfo		wInfo;

		wInfo.bufPtr		= m_pAudioData->m_pData;
		wInfo.len			= m_pAudioData->m_sample_count;
		wInfo.sampleRate	= ::g_SessionSRate;
		wInfo.numOfChan		= 2;
		wInfo.fileNamePtr	= wavFileName;

		if (::WriteMMWave (&wInfo))
			{
			//---------------------------------
			// ERROR!!!
			//---------------------------------
			MessageBox ( 0, "Can't write file.", NULL, MB_OK| MB_ICONEXCLAMATION);
			goto EXIT_SAVE;
			}
		}

EXIT_SAVE:
	InputAudioState (true);
	StartMeterTimer ();
	return;
}


*/








LRESULT CALLBACK AboutProc( HWND hDlg,           
                        UINT message,        
                        WPARAM wParam,       
                        LPARAM lParam)
{
   switch (message) 
   {
       case WM_INITDIALOG: 
 			CenterDialog (hDlg);
			return (TRUE);

       case WM_COMMAND:                              
               if (LOWORD(wParam) == ID_ABOUT_OK)    
               {
               EndDialog(hDlg, TRUE);        
               return (TRUE);
               }
               break;
   }

   return (FALSE); 
}







/*****************************************************************************
*  WndProc
*
*
*
******************************************************************************/


LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
   long		err;
//static char		speakStr[] = {"\pMy first name is Sam."};

     HDC          	hdc ;
     TEXTMETRIC		tm ;
     int			i;
     PAINTSTRUCT 	ps ;
	 RECT			bounds;


   switch( uMsg )
   {
           case WM_CREATE :
               hdc = GetDC (hWnd) ;
               SelectObject (hdc, GetStockObject (SYSTEM_FIXED_FONT)) ;
               GetTextMetrics (hdc, &tm) ;
               ReleaseDC (hWnd, hdc) ;

				//------------------------------
				// Create SPEAK Button
				//------------------------------
                speakButton = CreateWindow ("button", "Speak",
                          WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                          kSpeak_X, kSpeak_Y,
                          kSpeak_WD, 
                          7 * (tm.tmHeight + tm.tmExternalLeading) / 4,
                          hWnd, (HMENU) kSpeakButton,
                          hInst, NULL) ;

				//------------------------------
				// Create VOICE List
				//------------------------------
               listCntrl = CreateWindowEx (WS_EX_CLIENTEDGE, "listbox", NULL,
                              WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL | WS_BORDER,
                              kList_X, kList_Y,
                              kList_WD,
                              kList_HT,
                              hWnd, (HMENU) kVoiceCntrl,
                              hInst, NULL) ;

				
				

              for (i = 0; i < 17; i++)
                    {
                    SendMessage (listCntrl, LB_ADDSTRING, 0, (LPARAM) voiceNames[i]) ;
                    }
                    
				i = SendMessage (listCntrl, LB_SETCURSEL, kFredVoice, 0) ;

				//------------------------------
				// Create EDIT Control
				//------------------------------
               editCntrl = CreateWindowEx (WS_EX_CLIENTEDGE, "edit", NULL,
                          //WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL |
                        WS_CHILD | WS_VISIBLE | WS_VSCROLL |
                              WS_BORDER | ES_LEFT | ES_MULTILINE |
                              ES_NOHIDESEL | ES_AUTOVSCROLL,
                         kEdit_X, kEdit_Y, 
                         kEdit_WD, 
                         kEdit_HT, 
                         hWnd, (HMENU) kEditCntrl, hInst, NULL) ;

               SendMessage (editCntrl, EM_LIMITTEXT, 250, 0L) ;
               SetWindowText (editCntrl, "Hello, this is WinTalker.");

			g_BackBitMap = LoadBitmap (hInst, MAKEINTRESOURCE (IDB_BACKPAT));


               break;

     case WM_COMMAND :
              switch( LOWORD( wParam ) )
              {
                 case kSpeakButton :
                        if (HIWORD( wParam ) == BN_CLICKED)
                        	{
							gSpeechIsActive = true;
							gSpeechIsDone = false;
							EnableWindow (speakButton, FALSE);
							
							i = GetWindowTextLength (editCntrl) + 1;
							if (i > 250)
								i = 250;

							GetWindowText (editCntrl, &textBuf[0], i) ;

                        	_UseVoice (gInstanceStorage, g_CurrentVoice);
							(void) _SpeakBuffer (gInstanceStorage, &textBuf[0], i, 0);
                        	}
                        break;
                 case kVoiceCntrl :
                        if (HIWORD( wParam ) == LBN_SELCHANGE)
                        	{
                        	g_CurrentVoice = (short)SendMessage (listCntrl, LB_GETCURSEL, 0, 0);
							}
                        break;
                 case IDM_TEST :
                        break;

 				case IDM_HELP_ABOUT:
					{
					DialogBox( NULL, MAKEINTRESOURCE (IDD_ABOUT), hWnd, (DLGPROC)AboutProc );
					break;
					}

				case IDM_FILE_SPEAK:
					//SpeakToFile();
					break;

                 case IDM_FILE_QUIT:
                 		CloseSpeech ();
                        DestroyWindow( hWnd );
                        break;
              }
              break;

          case WM_PAINT :
               hdc = BeginPaint (hWnd, &ps) ;

				//-----------------------------------
				// Paint Background
				//-----------------------------------
				SetRect (&bounds,  0, 0, kFrameWind_WD, kFrameWind_HT);
				PaintBkgrndPat (hdc, &bounds);

				SetBkMode (hdc, TRANSPARENT);
				
				//SetTextColor (hdc, RGB (0, 0, 0));
				//TextOut (hdc, kEditL_X, kEditL_Y, "Text-to-Speech", 14);
				SetTextColor (hdc, RGB (0x44, 0x44, 0x44));
				TextOut (hdc, kEditL_X, kEditL_Y, "Text-to-Speech", 14);

				//SetTextColor (hdc, RGB (0, 0, 0));
				//TextOut (hdc, kListL_X, kListL_Y, "Voice", 5);
				//SetTextColor (hdc, RGB (0x99, 0x99, 0x99));
				TextOut (hdc, kListL_X, kListL_Y, "Voice", 5);

				SetTextColor (hdc, RGB (0, 0, 0));

				SetBkMode (hdc, OPAQUE);

				EndPaint (hWnd, &ps) ;
               break ;

#if 0
          case WM_CTLCOLORSTATIC :
                    //SetTextColor ((HDC) wParam, GetSysColor (COLOR_BTNSHADOW)) ;
                    //SetBkColor ((HDC) wParam, GetSysColor (COLOR_MENU));
                    
                    
                    return (LRESULT) GetStockObject (LTGRAY_BRUSH);
               break ;
#endif


		case WM_START_SPEECH:
			{
			g_CurrentVoice = kFredVoice;
		   	gInstanceStorage = NULL;
			gComponentRefcon = 0;
			gSpeechIsDone = true;
			gSpeechIsActive = false;
			if ( 	( (err = _OpenSpeech (hWnd) ) == kNoError) &&
					( (err = _UseVoice (gInstanceStorage, g_CurrentVoice) ) == kNoError) ) 
				{
				gSpeechIsDone = false;
				SetWindowText (hWnd, "TTS Demo");			// ####
				}
			break;
			}

      case MM_WOM_DONE :
              {
                 PostMessage (hWnd, USR_OUTBLOCK, 0, lParam);
                 break;
              }

		case USR_OUTBLOCK :
			{
			if (!gSpeechIsDone)
				SM_CallBack (gInstanceStorage, (LPWAVEHDR)lParam);
			else
				{
				gSpeechIsActive = false;
				EnableWindow (speakButton, TRUE);
				}
			break;
			}
        
      case WM_DESTROY :

	  		DeleteObject (g_BackBitMap);

              PostQuitMessage(0);
              break;

      default :
            return( DefWindowProc( hWnd, uMsg, wParam, lParam ) );
   }

   return( 0L );               
}





void	CloseSpeech ()
{
	if (gInstanceStorage)
		{
		(void) _CloseSpeech ( gInstanceStorage);
		gInstanceStorage = NULL;
		}

}





