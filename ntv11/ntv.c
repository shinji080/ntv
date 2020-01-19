/* NTV.c for ntv(NihongoTextViewer) */
/* Author : Shinji Miyoshi shinji080@amiga.ne.jp
** ver0.1   930709 first code.
** ver0.2   930711 some bug fixed, readable MSDOS text.
** ver0.3   930711 some bug fixed, readable ProDOS,MacOS text.
** ver0.4   930820 some bug.fixed, use K14.font,S:AKTERM.CFG              
** ver0.4a  931002 not display 0x0d for *hisada-san.
** ver0.4b  931023 able to read a text by setting default tools in icon.
** ver0.5   931028 select(right mouse) button is page-up. 
**                 add short-cut key for 'open' and 'quit'. 
** ver0.51  931219 fixed bug.
** ver0.6   940524 change menu text from Topaz to Screen Font. bug fixed. 
** ver0.7   940528 bug fixed for refresh window and more.
** ver0.8   940602 added Font Menu.
** ver0.9   940611 added line scroll.
** ver0.91  940611 CR-LF of MSDOS text can feed only ONE line.   
** ver0.92  940618 BS and DEL code can work correctly.
** ver1.0   941228 When S:AKTerm is not found, could run default-mode.
** ver1.1   951120 Under default-mode, K14 fonts are in FONTS: drawer.
** ------   200119 Commit to GitHub.
*/
/*
** Many parts of this source from 
**     AMIGA ROM kernel reference manual LIBRARIES.
**
** drawCharaImages() is refered from AKTerm (Amiga Kanji Terminal) by DAH.
** Thank you, DAH.
**
** And thank Yonkito-Hono to find bug and give ideas.
*/

/* Enforce use of new prefixed Intuition flag names */
#define INTUI_V36_NAMES_ONLY

#define CONFIGFILE      ("S:AKTERM.CFG")

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <graphics/text.h>
#include <dos/dos.h>
#include <intuition/screens.h>
#include <exec/libraries.h>
#include <libraries/asl.h>
#include <workbench/startup.h>

#include <clib/exec_protos.h>
#include <clib/layers_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#include <clib/asl_protos.h>
#include <clib/icon_protos.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef LATTICE
int CXBRK(void)     {return(0);}
int chkabort(void)  {return(0);}
#endif

/* GADGET */
#define UP_DOWN_GADGET      (0)
#define LEFT_RIGHT_GADGET   (1)
#define NO_GADGET           (2)
#define MAXPROPVAL  (0xFFFFL)
#define GADGETID(x)   (((struct Gadget *)(msg->IAddress))->GadgetID)

/* WINDOW */
#define WIDTH_MAXBITMAP     (1280)
#define HEIGHT_MAXBITMAP    (800)
#define DEFWIN_WIDTH        (640)
#define DEFWIN_HEIGHT       (400)
#define MYTEXT_LEFT         (0)
#define MYTEXT_TOP          (0)

/* MENU */
#define MENUWIDTH   (80+24)
#define MENUHEIGHT  (10)

/* AK font */
#define AKFONT_LEFT        (0)
#define AKFONT_TOP         (0)
#define AKFONT_WIDTH       (16)
#define AKFONT_WIDTHHALF   (8)
#define AKFONT_HEIGHT      (16)
#define AKFONT_DEPTH       (1)
#define len_AKkanjiFont1   (196608)
#define len_AKkanjiFont2   (98304)
#define len_AKankFont      (3568)
#define AKANKFONT          (0)
#define AKKANJIFONT        (1)

/* TEXT DISPLAY FORMAT */
#define COLUMN             (80)
#define LF                 (10)
#define ROLLDOWN           (0)
#define ROLLUP             (1)
#define FOREWARD           (0)
#define BACKWARD           (1)

/* REQUESTER */
#define ASLLEFTEDGE (0)
#define ASLTOPEDGE  (0)
#define ASLWIDTH    (320)
#define ASLHEIGHT   (200)



/* TITLE */
UBYTE   vers[] = "$VER: ntv 1.1";
UBYTE   titleStr[64] = "ntv 1.1";

/* LIBRARY */
struct  Library *GfxBase = NULL;
struct  Library *IntuitionBase = NULL;
struct  Library *LayersBase = NULL;

/* AKFONT */
UBYTE   *AKankFontName    = NULL;
UBYTE   *AKkanjiFont1Name = NULL;
UBYTE   *AKkanjiFont2Name = NULL;

UBYTE   *AKankFont     = NULL;
UBYTE   *AKkanjiFont1  = NULL;
UBYTE   *AKkanjiFont2  = NULL;
UWORD   *AKankFontW    = NULL;
UWORD   *AKkanjiFont1W = NULL; 
UWORD   *AKkanjiFont2W = NULL; 
ULONG   AKkanjiPtr     = NULL;
ULONG   AKankPtr       = NULL;

BPTR    AKkanjiFont1File = NULL;
BPTR    AKkanjiFont2File = NULL;
BPTR    AKankFontFile    = NULL;

UBYTE   *AKHFont;
UBYTE   *AKZFont;
UBYTE   *AKFEP;
UBYTE   *AKLOG;
UBYTE   *AKMODE;

/* REQUESTER */
UBYTE   *ASLTextDir        = NULL;
UBYTE   *ASLAKankFontDir   = NULL;
UBYTE   *ASLAKkanjiFontDir = NULL;

/* TEXT */
UBYTE   *FName = NULL;
BPTR    TextFile = {0};
UBYTE   *TextBuffer = NULL;
ULONG   lenTextBuffer = 0;
ULONG   charaNum,lineNum,charaMax = 0,lineNumMax = 0,LFnum = 0;
UBYTE   *textPtr = NULL;
ULONG   textSize = 0L;

/* WINDOW */
struct  Window      *Win=NULL;
struct  Screen      *myscreen = NULL;
struct  BitMap      *myBitMap = NULL;
ULONG   WidthBitMap,HeightBitMap;
ULONG   pens[] = {~0};
WORD    planeNum = 0;
UBYTE   depth = 1;

/* BINALY TEXT ACCESS */
BOOL    binFlug; /* binaly or text */

/* MSDOS AVAILABLE */
BOOL    NO0D_flug = TRUE;

/* TEXT DISPLAY FORMAT */
ULONG   linePos = 0;
ULONG   lineOfPage;
UWORD   cursolX,cursolY;

/* COMMAND LINE AND 'CALLED BY DATA' IS AVAILABLE */
BOOL    called_flug = FALSE;

/* REQUESTER */
struct  Library     *AslBase = NULL;

/* GADGET */
struct  PropInfo    SideGadInfo={0};
struct  Image       SideGadImage={0};
struct  Gadget      SideGad={0};

/* FONT IMAGE (AK,ANS) */
UBYTE   myFont_Left      = AKFONT_LEFT;
UBYTE   myFont_Top       = AKFONT_TOP;
UBYTE   myFont_Width     = AKFONT_WIDTH;
UBYTE   myFont_WidthHalf = AKFONT_WIDTHHALF;
UBYTE   myFont_Height    = AKFONT_HEIGHT;
UBYTE   myFont_Depth     = AKFONT_DEPTH;
 
/* MENU */
struct IntuiText projMenuIText[] = 
{
/* Project */
    {0, 1,  JAM2,   0,  1, NULL,   "Open...",  NULL},
    {0, 1,  JAM2,   0,  1, NULL,   "Quit   ",  NULL}
};

struct IntuiText fontMenuIText[] =
{
/* Font    */
    {0, 1,  JAM2,   0,  1, NULL,   "AKank     ",  NULL},
    {0, 1,  JAM2,   0,  1, NULL,   "AKKanji   ",  NULL}
};

struct MenuItem projMenu[] = 
/* Project */
{
    {/* Open... */
    &projMenu[1],  0,  0,  MENUWIDTH,  0,
    ITEMTEXT | COMMSEQ | MENUTOGGLE | ITEMENABLED | HIGHCOMP,
    0,  (APTR)&projMenuIText[0], NULL,   'O',   NULL,   MENUNULL
    },
    {/* Quit */
    NULL,  0,  0,  MENUWIDTH,  0,
    ITEMTEXT | COMMSEQ | MENUTOGGLE | ITEMENABLED | HIGHCOMP,
    0,  (APTR)&projMenuIText[1], NULL,   'Q',   NULL,   MENUNULL
    }
};

struct MenuItem fontMenu[] =
/* Font */
{
    {/* AKankFont */
    &fontMenu[1],  0,  0,  MENUWIDTH,  0,
    ITEMTEXT | COMMSEQ | MENUTOGGLE | ITEMENABLED | HIGHCOMP,
    0,  (APTR)&fontMenuIText[0], NULL,   'A',   NULL,   MENUNULL
    },
    {/* AKkanjiFont */
    NULL,  0,  0,  MENUWIDTH,  0,
    ITEMTEXT | COMMSEQ | MENUTOGGLE | ITEMENABLED | HIGHCOMP,
    0,  (APTR)&fontMenuIText[1], NULL,   'K',   NULL,   MENUNULL
    }
};

#define NUM_MENUS 2
STRPTR menutitle[] = {"Project","AKFont"};
struct Menu menuStrip[] =
{
/* Project */
    {
    &menuStrip[1],  /* Next Menu        */
    0,  0,          /* LeftEdge,TopEdge */
    0,  MENUHEIGHT, /* Width,Height     */
    MENUENABLED,    /* Flag             */
    NULL,           /* Title            */
    &projMenu[0]    /* First item       */
    },

/* Font */
    {
    NULL,           /* Next Menu        */
    MENUWIDTH,  0,  /* LeftEdge,TopEdge */
    0,  MENUHEIGHT, /* Width,Height     */
    MENUENABLED,    /* Flag             */
    NULL,           /* Title            */
    &fontMenu[0]    /* First item       */
    }
};

/* FUNCTION PROTOTYPE */
VOID initBorderProps(struct Screen *myscreen);
VOID doNewSize(void);
VOID doDrawKanjiText(void);
VOID load_AKankFont(void);
VOID load_AKkanjiFont(void);
VOID doMsgLoop(void);
VOID processWindow(struct Screen *myscreen);
VOID getTextSize(void);
VOID cleanup(void);
VOID loadText(void);
VOID checkGadget(UWORD gadgetID);
BYTE getFilename(void);
BYTE Bin_or_Text(void);
VOID loadConfig(void);
BYTE getAKFontName(BYTE);
VOID makeAKFontName(void);
VOID scrollUp(void);
VOID scrollDown(void);
BYTE drawCharaImages(UWORD *KanjiImageData, struct Image *myFontImage,
    UBYTE direction);
VOID doDrawKanjiLine(UBYTE drawLine);
VOID scrollPageUp(void);
VOID scrollPageDown(void);


/*
** main
** Open all required libraries and get a pointer to the default 
** public screen.
** Cleanup when done or on error.
*/
VOID main(int argc, char *argv[])
{
/*
** open all of required libraries for the program.
**
** required version 37 of the Intuition library.
*/
struct  WBStartup   *argmsg; 
struct  WBArg       *wb_arg;
LONG    olddir = -1;

FName = AllocMem(256,MEMF_ANY | MEMF_CLEAR);
AKkanjiFont1Name = AllocMem(256,MEMF_ANY | MEMF_CLEAR);
AKkanjiFont2Name = AllocMem(256,MEMF_ANY | MEMF_CLEAR);
AKankFontName    = AllocMem(256,MEMF_ANY | MEMF_CLEAR);
ASLAKankFontDir  = AllocMem(256,MEMF_ANY | MEMF_CLEAR); 
ASLAKkanjiFontDir= AllocMem(256,MEMF_ANY | MEMF_CLEAR);  
ASLTextDir       = AllocMem(256,MEMF_ANY | MEMF_CLEAR); 

if(argc == 0) /* When run under the Workbench. */
    {
    argmsg = (struct WBStartup *)argv;
    if(argmsg->sm_NumArgs > 1)
        {
        wb_arg = argmsg->sm_ArgList;
        wb_arg++;
        olddir = CurrentDir(wb_arg->wa_Lock);
        strcpy(FName,wb_arg->wa_Name);
        called_flug = TRUE;
        }
    }

if(argc == 2)
    {
    if(strcmp(argv[1],"-D")==1) 
        NO0D_flug = FALSE;
    else
        {       
        strcpy(FName,argv[1]);
        called_flug = TRUE;
        }
    }


if(argc != 1 && called_flug == TRUE)
    {
    getTextSize();
    loadText();
    }

loadConfig();
makeAKFontName();

printf("loading Font...\n");
load_AKkanjiFont();
load_AKankFont();

IntuitionBase=OpenLibrary("intuition.library",37L);
if(IntuitionBase==NULL) cleanup();

GfxBase=OpenLibrary("graphics.library",33L);
if(GfxBase==NULL) cleanup();

LayersBase=OpenLibrary("layers.library",33L);
if(LayersBase==NULL) cleanup();

/*
** LockScreen()/UnlockScreen is only available under V36
** and later... Use GetScreenData() under V34 system to get a
** copy of the screen structure...
*/

myscreen = LockPubScreen(NULL);
if(myscreen==NULL) cleanup();

processWindow(myscreen);
if(olddir != -1) CurrentDir(olddir);
cleanup();
}

VOID cleanup(void)
{
if(myscreen != NULL) UnlockPubScreen(NULL,myscreen);
if(Win != NULL) 
    {
    ClearMenuStrip(Win);
    CloseWindow(Win);
    }

/* free only the bitplanes actually allocated... */
if(myBitMap != NULL)
    {
    FreeMem(myBitMap,sizeof(struct BitMap));
    for(planeNum=0; planeNum<depth;planeNum++)
        {
        if (NULL!=myBitMap->Planes[planeNum])
            FreeRaster(myBitMap->Planes[planeNum],
                WIDTH_MAXBITMAP,HEIGHT_MAXBITMAP);
        }
    }
/*if(myscreen!=NULL)    CloseScreen(myscreen);*/
if(LayersBase!=NULL)    CloseLibrary(LayersBase);
if(GfxBase!=NULL)       CloseLibrary(GfxBase);
if(IntuitionBase!=NULL) CloseLibrary(IntuitionBase);

if(AKkanjiFont1Name != NULL) FreeMem(AKkanjiFont1Name,256);
if(AKkanjiFont2Name != NULL) FreeMem(AKkanjiFont2Name,256);
if(AKankFontName != NULL) FreeMem(AKankFontName,256);
if(AKkanjiFont1 != NULL) FreeMem(AKkanjiFont1,len_AKkanjiFont1);
if(AKkanjiFont2 != NULL) FreeMem(AKkanjiFont2,len_AKkanjiFont2);
if(AKankFont != NULL) FreeMem(AKankFont,len_AKankFont*2);
if(TextBuffer!=NULL) FreeMem(TextBuffer,lenTextBuffer);
if(FName!=NULL) FreeMem(FName,256);
if(AKHFont != NULL) FreeMem(AKHFont,256);
if(AKZFont != NULL) FreeMem(AKZFont,256);
if(AKFEP != NULL) FreeMem(AKFEP,16);
if(AKLOG != NULL) FreeMem(AKLOG,16);
if(AKMODE != NULL) FreeMem(AKMODE,16);
if(ASLAKankFontDir != NULL) FreeMem(ASLAKankFontDir,256);
if(ASLAKkanjiFontDir != NULL) FreeMem(ASLAKkanjiFontDir,256);
if(ASLTextDir != NULL) FreeMem(ASLTextDir,256);

exit(0);
}

/* Create initialize and process the bitmap window.
** Cleanup if any error
*/
VOID processWindow(struct Screen *myscreen)
{
WORD    allocatedBitMaps;
UWORD   left=2,m;

/* set-up the border prop gadgets for the OpenWindow() call. */

initBorderProps(myscreen);
/* if code relies on the allocation of the BitMap structure with
** the MEMF_CLEAR flag. This allows the assumption that all of the
** bitmap pointers are NULL, except those successfully allocated
** by the program.
*/

myBitMap=AllocMem(sizeof(struct BitMap),MEMF_ANY | MEMF_CLEAR);
if(myBitMap!=NULL)
    {
    WidthBitMap=DEFWIN_WIDTH;
    HeightBitMap=(DEFWIN_HEIGHT - myscreen->Font->ta_YSize * 2 - 4) 
                / myFont_Height * myFont_Height
                + myscreen->Font->ta_YSize + 4;
    InitBitMap(myBitMap,depth,WIDTH_MAXBITMAP,HEIGHT_MAXBITMAP);
        allocatedBitMaps=TRUE;
    for(planeNum=0;
        (planeNum<depth) && (allocatedBitMaps==TRUE);
        planeNum++)
        {
        myBitMap->Planes[planeNum]
            =AllocRaster(WIDTH_MAXBITMAP,HEIGHT_MAXBITMAP);
        if(NULL == myBitMap->Planes[planeNum])
            {
            allocatedBitMaps=FALSE;
            printf("Not allocated BitMap's memory.\n");
            }
        }
    
    /* Only open the window if the bitplanes were successfully
    ** allocated. Fail silently if they were not.
    */
        
    if(TRUE==allocatedBitMaps)
        {
        /* OpenWindowTags() and OpenWindowTagList() are only available
        ** when the library version is at least V36. Under earier
        ** versions of Intuition, use OpenWindow() with a NewWindow
        ** structure.
        */
        strcat(titleStr,FName);
        if(NULL!=(Win=OpenWindowTags(NULL,
            WA_Width,   DEFWIN_WIDTH,
            WA_Height,  (DEFWIN_HEIGHT - myscreen->Font->ta_YSize*2 - 4)
                        / myFont_Height * myFont_Height
                        + myscreen->Font->ta_YSize + 4,
            WA_MaxWidth,    WIDTH_MAXBITMAP,
            WA_MaxHeight,   HEIGHT_MAXBITMAP,
            WA_MinWidth,    100,
            WA_MinHeight,   30,
            WA_IDCMP,   IDCMP_MENUPICK | IDCMP_GADGETUP | IDCMP_GADGETDOWN 
                | IDCMP_NEWSIZE | IDCMP_CLOSEWINDOW | IDCMP_ACTIVEWINDOW |
                IDCMP_INACTIVEWINDOW | IDCMP_REFRESHWINDOW |
                IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY,
            WA_Flags,   WFLG_SIZEGADGET | WFLG_SIZEBRIGHT | 
                WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | 
                WFLG_SIMPLE_REFRESH | WFLG_GIMMEZEROZERO | WFLG_ACTIVATE, 
            WA_Gadgets,  &(SideGad),
            WA_Title,   titleStr,
            WA_PubScreen,   myscreen,
            TAG_DONE)))
            {

            projMenuIText->ITextFont = fontMenuIText->ITextFont
                = myscreen->Font;
            projMenu[0].Height = projMenu[1].TopEdge = projMenu[1].Height
                = myscreen->Font->ta_YSize + 2;
            fontMenu[0].Height = fontMenu[1].TopEdge = fontMenu[1].Height
                = myscreen->Font->ta_YSize + 2;
            for(m=0; m<NUM_MENUS; m++)
                {
                menuStrip[m].LeftEdge = left;
                menuStrip[m].MenuName = menutitle[m];
                menuStrip[m].Width = TextLength(&Win->WScreen->RastPort,
                    menutitle[m],strlen(menutitle[m])) + 8;
                left += menuStrip[m].Width;
                }
            SetMenuStrip(Win,menuStrip);
            SetRast(Win->RPort,0);

            if(called_flug == FALSE)
                {
                getFilename();
                getTextSize();
                loadText();
                }

            SetWindowTitles(Win,FName,(UBYTE *)(-1));
            linePos = 0;
            lineNum = 0;
            doNewSize();
            if(called_flug == TRUE)
                doDrawKanjiText();
            doMsgLoop();

            ClearMenuStrip(Win);
            CloseWindow(Win);
            Win = NULL;
            }
        }
    }
}






/* Set-up the prop gadgets--initialize them to values that fit
** into the window border. The height of the prop gadget on the side 
** of the window takes the height if the titl bar into account in its
** set-up.note the initialization assumes a fixes size "sizing" gadget..
**
** Note also, that the size of the sizing gadget is dependent on the
** screen resolutuion. The numbers given here are only valid if the 
** screen is NOT lo-res. These values must be re-worked slightly
** for lo-res screens.
**
** The PROPNEWLOOK flag is ignored by 1.3
*/

VOID initBorderProps(struct Screen *myscreen)
{
/* initialize the two prop gadgets.
**
** Note where the PROPNEWLOOK flag goes. Adding this flag requires
** no extra storage, but tells the systemthat our program is
** expecting the new-look prop gadget under 2.0.
*/
SideGadInfo.Flags    = AUTOKNOB | FREEVERT | PROPNEWLOOK;
SideGadInfo.HorizPot =0;
SideGadInfo.VertPot  =0;
SideGadInfo.HorizBody=65535;
SideGadInfo.VertBody =65535;

/* NOTE the TopEdge adjustment for the border and the font for V36.*/
SideGad.LeftEdge     =-14;
SideGad.TopEdge      =myscreen->WBorTop + myscreen->Font->ta_YSize + 2;
SideGad.Width        =12;
SideGad.Height       =-SideGad.TopEdge -11;
SideGad.Flags        =GFLG_RELRIGHT | GFLG_RELHEIGHT;
SideGad.Activation   =GACT_RELVERIFY | GACT_IMMEDIATE | GACT_RIGHTBORDER;
SideGad.GadgetType   =GTYP_PROPGADGET | GTYP_GZZGADGET;
SideGad.GadgetRender =(APTR)&(SideGadImage);
SideGad.SpecialInfo  =(APTR)&(SideGadInfo);
SideGad.GadgetID     =UP_DOWN_GADGET;
}





/*
** Update the prop gadgets and bitmap positioning when the size changes.
*/
VOID doNewSize(void)
{
ULONG VPot,VBody;

if(lineNum == lineNumMax - lineOfPage)    
    {
    lineOfPage = Win->GZZHeight / myFont_Height;
    lineNum = lineNumMax - lineOfPage;
    }
else
    lineOfPage = Win->GZZHeight / myFont_Height;

if(lineNum < lineNumMax - lineOfPage)
    VPot = MAXPROPVAL 
        / (lineNumMax - lineOfPage) * lineNum;
else
    VPot = MAXPROPVAL;
        
if(lineOfPage < lineNumMax)
    {
    VBody = MAXPROPVAL / lineNumMax * lineOfPage;
    }
else
    VBody = MAXPROPVAL;


/*printf("Win->GZZHeight %ld VBody %ld\n",Win->GZZHeight,VBody);*/


NewModifyProp(&(SideGad),Win,NULL,AUTOKNOB | FREEVERT,
    NULL,                       /* HorizPot */
    VPot,                       /* VertPot  */
    MAXPROPVAL,                 /* HorizBody*/ 
    VBody,                      /* VertBody */    
    1);

HeightBitMap = Win->GZZHeight;
}





/*
** Process the currently selected gadget.
** This is called from IDCMP_INTUITICKS and when the gadget is released
** IDCMP_GADGETUP.
*/
VOID checkGadget(UWORD gadgetID)
{
ULONG tmp;

switch(gadgetID)
    {
    case UP_DOWN_GADGET:
        tmp =  SideGadInfo.VertPot
            * (lineNumMax - lineOfPage) / MAXPROPVAL;
        break;
    }

if(tmp<=0)
    lineNum = 0;
else
    lineNum = tmp;

if(lineOfPage<lineNumMax)
    doDrawKanjiText();

}


/*
** Main messsage loop for the window
*/
VOID doMsgLoop(void)
{
struct IntuiMessage *msg;
WORD flag = TRUE;
UWORD currentGadget = NO_GADGET;
UWORD menuNumber;
ULONG class;
USHORT menuNum;
USHORT itemNum;
BOOL secondRefresh = FALSE;

while(flag)
    {
    /* Whenever you want to wait on just one message port */
    /* you can use WaitPort(). WaitPort() doesn't require */
    /* the setting of a signal bit. The only argument it  */
    /* requires is the pointer to the window's UserPort   */

    Wait(1L << Win->UserPort->mp_SigBit);

    while(msg=(struct IntuiMessage *)GetMsg(Win->UserPort))
        {
            menuNumber  = msg->Code;
            class       = msg->Class;
            printf("--- IDCMP code %ld ---\n",class);
            switch (class)
                {
                case IDCMP_CLOSEWINDOW:
                    printf("CLOSEWINDOW\n");
                    flag=FALSE;
                    break;
                case IDCMP_NEWSIZE:
                    printf("NEWSIZE\n");
                    doNewSize();
                    break;
                case IDCMP_GADGETDOWN:
                    printf("GADGETDOWN\n");
                    currentGadget=GADGETID(msg);
                    break;
                case IDCMP_GADGETUP:
                    printf("GADGETUP\n");
                    checkGadget(currentGadget);
                    currentGadget=NO_GADGET;
                    break;
                case IDCMP_INTUITICKS:
                    printf("INTUITICKS\n");
                    break;
                case IDCMP_REFRESHWINDOW:
                    printf("REFRESHWINDOW\n");
                    if(secondRefresh == FALSE)
                        doDrawKanjiText();
                    else
                        secondRefresh = FALSE; 
                    break;
                case IDCMP_ACTIVEWINDOW:
                    printf("ACTIVEWINDOW\n");
                    break;
                case IDCMP_INACTIVEWINDOW:
                    printf("INACTIVEWINDOW\n");
                    break;
                 case IDCMP_MOUSEBUTTONS:
                    printf("MOUSEBUTTONS\n");
                    if(menuNumber==SELECTDOWN)
                        {
                        if(lineNum < lineNumMax - lineOfPage)
                            scrollPageUp();
                        }
                    break;                

                case IDCMP_MENUPICK:
                    printf("MENUPICK\n");
                    menuNum = MENUNUM(menuNumber);
                    itemNum = ITEMNUM(menuNumber);
                    if(menuNum == 0) /* Project menu */
                        {
                        if(itemNum == 1) flag=FALSE;
                        if(itemNum == 0) 
                            {
                            if(getFilename() != -1)
                                {
                                FreeMem(TextBuffer,lenTextBuffer);
                                getTextSize();
                                loadText();
                                SetWindowTitles(Win,FName,(UBYTE *)(-1));
                                linePos = 0;
                                lineNum = 0;
                                doNewSize();
                                }
                            }
                        }
                    else if(menuNum == 1) /* AKFont menu */
                        {
                        if(itemNum == 0)
                            {
                            getAKFontName(AKANKFONT);
                            load_AKankFont();
                    
                            }
                        else if(itemNum == 1)
                            {
                            getAKFontName(AKKANJIFONT);
                            load_AKkanjiFont();
                            secondRefresh = TRUE;
                            }
                        }        
                    break;

                case IDCMP_RAWKEY:
                    printf("IDCMP_RAWKEY\n");
                    if(msg->Code == 0x4d)         /* scroll up 1 line */
                        scrollUp();
                    else if(msg->Code == 0x4c)    /* scroll down 1 line */
                        scrollDown();            
                    else if(msg->Code == 0x3f)    /* scroll up 1 page */    
                        {
                        if(lineNum < lineNumMax - lineOfPage)
                            scrollPageUp();
                        }
                    else if(msg->Code == 0x1f)    /* scroll down 1 page */
                        scrollPageDown();
                    break;
                }
            ReplyMsg((struct Message *)msg);
        }
    }
}

            



BYTE Bin_or_Text(void)
{
int i;
UBYTE c;

TextFile=Open(FName,MODE_OLDFILE);
if(NULL==TextFile)
    {
    printf("Not found file %s\n",FName);
    cleanup();
    }
else
    {
    for(i=0;i<16;i++)
        {
        c = (UBYTE)FGetC(TextFile);    
        if((c < 0x20) && (c != 0x0a) && 
            (c!= 0x0d) && (c!= 0x1a) && (c!= 0x09))
            {
            Close(TextFile);
            return TRUE; /* Binaly */
            }
        }
    Close(TextFile);
    return FALSE;
    }
}





VOID getTextSize(void)
{
ULONG  cLong;
BOOL doubtMSDOSflug = FALSE;

TextFile=Open(FName,MODE_OLDFILE);
if(NULL==TextFile)
    {
    printf("Not found file %s\n",FName);
    cleanup();
    }
else
    {
    textSize = 0L;
    charaMax = 0;
    charaNum = 0;
    lineNumMax = 0;
    LFnum = 0;

    for(textSize=0L;
        cLong=FGetC(TextFile), 
        cLong != -1 && cLong != (ULONG)0xff && cLong!= (ULONG)0x1a;
        textSize++)
        {
        if(cLong == (ULONG)0x0a)
            {
            if(doubtMSDOSflug != TRUE)
                {
                if(charaNum > charaMax) charaMax = charaNum;
                charaNum = 0;
                LFnum++;
                lineNumMax++;
                }
            doubtMSDOSflug = FALSE;
            }
        else if(cLong == (ULONG)0x0d)
            {
            if(charaNum > charaMax) charaMax = charaNum;
            charaNum = 0;
            LFnum++;
            lineNumMax++;
            doubtMSDOSflug = TRUE;
            }
        else if(cLong == (ULONG)0x08 || cLong == (ULONG)0x7f)
            {
            charaNum--;
            }
        else
            {
            charaNum++;
            if(charaNum >= COLUMN)
                {
                lineNumMax++;
                charaNum = 0;
                }
            doubtMSDOSflug = FALSE;
            }
        }    

    printf("charaMax %ld lineNumMax %ld textSize %ld\n"
        ,charaMax,lineNumMax,textSize);
    
    Close(TextFile);
    }
}





VOID loadText(void)
{
ULONG cLong;
ULONG m;
BOOL doubtMSDOSflug = FALSE;

TextFile=Open(FName,MODE_OLDFILE);
if(TextFile == NULL)
    {
    printf("Not found file %s\n",FName);
    cleanup();
    }
else
    {
    lenTextBuffer = textSize + lineNumMax - LFnum;
    TextBuffer=AllocMem(lenTextBuffer,MEMF_ANY | MEMF_CLEAR);
    if(TextBuffer==NULL)
        {
        Close(TextFile);
        printf("Not allocated memory.\n");
        cleanup();
        }
    else /*initialize */
        {
        for(m=0L; m<lenTextBuffer; m++)
            *(TextBuffer + m) = 0x20;
        }
    charaNum = 0;
    m = 0L;
    cLong  = 0L;   
    while(-1)
        {
        if(charaNum > COLUMN)
            {
            charaNum = 0;
            *(TextBuffer + m) = 0x0a;
            m++;
            }
        cLong=FGetC(TextFile);
        if(cLong == -1 || cLong == (ULONG)0xff || cLong == (ULONG)0x1a)
            break;
        if(cLong == (ULONG)0x0a || cLong == (ULONG)0x0d)
            charaNum = 0;
        if(!(cLong == (ULONG)0x0a && doubtMSDOSflug == TRUE))
            {            
            if(cLong == (ULONG)0x08 || cLong == (ULONG)0x7f)
                {
                m--;
                }
            else
                {
                *(TextBuffer + m) = (UBYTE)cLong;
                m++;
                if(cLong == (ULONG)0x0d)
                    doubtMSDOSflug = TRUE;
                else
                    doubtMSDOSflug = FALSE; /* reset doubtMSDOSflug */
                }
            }
        else
            doubtMSDOSflug = FALSE;
        }
    Close(TextFile);
    textPtr=(UBYTE *)TextBuffer;
    }

    binFlug = Bin_or_Text();
    if(binFlug == TRUE)
        {
        printf("Binaly file.\n");
        }
}



VOID makeAKFontName(void)
{
strcpy(AKkanjiFont1Name,AKZFont);
strcat(AKkanjiFont1Name,"_KANJI_16A");
strcpy(AKkanjiFont2Name,AKZFont);
strcat(AKkanjiFont2Name,"_KANJI_16B");
strcpy(AKankFontName,AKHFont);
strcat(AKankFontName,"_ank_16");

printf("AKkanjiFont1Name = %s\n",AKkanjiFont1Name);
printf("AKkanjiFont2Name = %s\n",AKkanjiFont2Name);
printf("AKankFontName    = %s\n",AKankFontName);
}



VOID load_AKkanjiFont(void)
{
printf("Kanji 1/2 set\n");
AKkanjiFont1File=(BPTR)Open(AKkanjiFont1Name,MODE_OLDFILE);
if(AKkanjiFont1File==NULL)
    {
    printf("Not found %s Font File\n",AKkanjiFont1Name);
    cleanup();
    }
if(AKkanjiFont1 != NULL)
    FreeMem(AKkanjiFont1,len_AKkanjiFont1);
AKkanjiFont1=AllocMem(len_AKkanjiFont1,MEMF_ANY);
if(AKkanjiFont1==NULL)
    {
    printf("Error : not enough memory\n");
    Close(AKkanjiFont1File);
    cleanup();
    }
else
    {
    Read(AKkanjiFont1File,AKkanjiFont1,len_AKkanjiFont1);
    Close(AKkanjiFont1File);
    AKkanjiFont1W = (UWORD *)AKkanjiFont1;
    }

printf("Kanji 2/2 set\n");
AKkanjiFont2File=(BPTR)Open(AKkanjiFont2Name,MODE_OLDFILE);
if(AKkanjiFont2File==NULL)
    {
    printf("Not found %s Font File\n",AKkanjiFont2Name);
    cleanup();
    }
if(AKkanjiFont2 != NULL)
    FreeMem(AKkanjiFont2,len_AKkanjiFont2);
AKkanjiFont2=AllocMem(len_AKkanjiFont2,MEMF_ANY);
if(AKkanjiFont2==NULL)
    {
    printf("Error : not enough memory\n");
    Close(AKkanjiFont2File);
    cleanup();
    }
else
    {
    Read(AKkanjiFont2File,AKkanjiFont2,len_AKkanjiFont2);
    Close(AKkanjiFont2File);
    AKkanjiFont2W = (UWORD *)AKkanjiFont2;
    }
}     




VOID load_AKankFont(void)
{
UBYTE *buf;
UWORD i;

if(AKankFont != NULL)
    FreeMem(AKankFont,len_AKankFont*2);
printf("ank set\n");
AKankFontFile=(BPTR)Open(AKankFontName,MODE_OLDFILE);
if(AKankFontFile==NULL)
    {
    printf("Not found %s Font File\n",AKankFontName);
    cleanup();
    }
buf=AllocMem(len_AKankFont,MEMF_ANY);
AKankFont=AllocMem(len_AKankFont*2,MEMF_ANY);
if(AKankFont==NULL) 
    {
    Close(AKankFontFile);
    cleanup();
    }
else
    {
    Read(AKankFontFile,buf,len_AKankFont);
    Close(AKankFontFile);
    for(i=0;i<len_AKankFont;i++)
        {
        *(AKankFont+i*2)=*(buf+i);
        *(AKankFont+i*2+1)=0x00;
        }
    FreeMem(buf,len_AKankFont);
    AKankFontW = (UWORD *)AKankFont;
    }
}

BYTE getFilename(void)
{
struct FileRequester *fr;

static UBYTE ASLDir[256];
 
if(AslBase = OpenLibrary("asl.library",37L))
    {
    if(fr=(struct FileRequester *)AllocFileRequest())
        {
        strcpy(ASLDir,ASLTextDir);
        if(AslRequestTags(fr,
            ASL_Hail,       (ULONG)"click file name for read.",
            ASL_Height,     ASLHEIGHT,
            ASL_Width,      ASLWIDTH,
            ASL_LeftEdge,   ASLLEFTEDGE,
            ASL_TopEdge,    ASLTOPEDGE,
            ASL_OKText,     (ULONG)"OK",
            ASL_CancelText, (ULONG)"Cancel",
            ASL_File,       (ULONG)"",
            ASL_Dir,        (ULONG)ASLDir,
            TAG_DONE))
            {
            strcpy(FName,fr->rf_Dir);
            AddPart(FName,fr->rf_File,256);
            strcpy(ASLTextDir,fr->rf_Dir);
            }
        FreeFileRequest(fr);
        }
    CloseLibrary(AslBase);
    }
printf("FName = %s\n",FName);
if(strlen(FName) == 0) return -1;

return 0;
}





BYTE getAKFontName(BYTE AKFontType)
{
struct FileRequester *fr;
STATIC UBYTE ASLFontDir[256];
BOOL notChangeFont = FALSE;

struct TagItem frtagsAKkanjiFont1[] =
    {
    ASL_Hail,       (ULONG)"click name of AK-Kanji-Font-A.",
    ASL_Height,     ASLHEIGHT,
    ASL_Width,      ASLWIDTH,
    ASL_LeftEdge,   ASLLEFTEDGE,
    ASL_TopEdge,    ASLTOPEDGE,
    ASL_OKText,     (ULONG)"OK",
    ASL_CancelText, (ULONG)"Cancel",
    ASL_File,       (ULONG)"",
    ASL_Dir,        (ULONG)ASLFontDir,
    TAG_DONE
    };

struct TagItem frtagsAKkanjiFont2[] =
    {
    ASL_Hail,       (ULONG)"click name of AK-Kanji-Font-B.",
    ASL_Height,     ASLHEIGHT,
    ASL_Width,      ASLWIDTH,
    ASL_LeftEdge,   ASLLEFTEDGE,
    ASL_TopEdge,    ASLTOPEDGE,
    ASL_OKText,     (ULONG)"OK",
    ASL_CancelText, (ULONG)"Cancel",
    ASL_File,       (ULONG)"",
    ASL_Dir,        (ULONG)ASLFontDir,
    TAG_DONE
    };

struct TagItem frtagsAKankFont[] =
    {
    ASL_Hail,       (ULONG)"click name of AK-ank-Font.",
    ASL_Height,     ASLHEIGHT,
    ASL_Width,      ASLWIDTH,
    ASL_LeftEdge,   ASLLEFTEDGE,
    ASL_TopEdge,    ASLTOPEDGE,
    ASL_OKText,     (ULONG)"OK",
    ASL_CancelText, (ULONG)"Cancel",
    ASL_File,       (ULONG)"",
    ASL_Dir,        (ULONG)ASLFontDir,
    TAG_DONE
    };

if(AslBase = OpenLibrary("asl.library",37L))
    {
    if(AKFontType == AKKANJIFONT)
        {
        strcpy(ASLFontDir,ASLAKkanjiFontDir);
        if(fr=(struct FileRequester *)
            AllocAslRequest(ASL_FileRequest,frtagsAKkanjiFont1))
            {
            if(AslRequest(fr,NULL))
                {
                strcpy(AKkanjiFont1Name,fr->rf_Dir);
                AddPart(AKkanjiFont1Name,fr->rf_File,256);
                if(strcmp(ASLAKkanjiFontDir,fr->rf_Dir) == 0)
                    notChangeFont = TRUE;
                strcpy(ASLAKkanjiFontDir,fr->rf_Dir);    
                }
            FreeFileRequest(fr);       
            }
            
        strcpy(ASLFontDir,ASLAKkanjiFontDir);
        if(fr=(struct FileRequester *)
            AllocAslRequest(ASL_FileRequest,frtagsAKkanjiFont2))
            {
            if(AslRequest(fr,NULL))
                {
                strcpy(AKkanjiFont2Name,fr->rf_Dir);
                AddPart(AKkanjiFont2Name,fr->rf_File,256);
                if(strcmp(ASLAKkanjiFontDir,fr->rf_Dir) == 0)
                    notChangeFont = TRUE;
                strcpy(ASLAKkanjiFontDir,fr->rf_Dir);
                }
            FreeFileRequest(fr);
            }
        }
        
    if(AKFontType == AKANKFONT)
        {
        strcpy(ASLFontDir,ASLAKankFontDir);
        if(fr=(struct FileRequester *)
            AllocAslRequest(ASL_FileRequest,frtagsAKankFont))
            {
            if(AslRequest(fr,NULL))
                {
                strcpy(AKankFontName,fr->rf_Dir);
                AddPart(AKankFontName,fr->rf_File,256);
                if(strcmp(ASLAKankFontDir,fr->rf_Dir) == 0)
                    notChangeFont = TRUE;
                strcpy(ASLAKankFontDir,fr->rf_Dir);
                }
            FreeAslRequest(fr);
            }
        }

    CloseLibrary(AslBase);
    }

printf("AKkanjiFont1Name = %s\n",AKkanjiFont1Name);
printf("AKkanjiFont2Name = %s\n",AKkanjiFont2Name);
printf("AKankFontName    = %s\n",AKankFontName);

if(notChangeFont == TRUE)
    return -1;
else
    return 0;
}



VOID loadConfig(void)
{
BPTR    configFile;
UBYTE   *buf,*buf2;
int     i,j,k;

#define LENBUF (80)

AKHFont = AllocMem(256,MEMF_ANY | MEMF_CLEAR);
AKZFont = AllocMem(256,MEMF_ANY | MEMF_CLEAR);
AKFEP  = AllocMem(16,MEMF_ANY | MEMF_CLEAR);
AKLOG  = AllocMem(16,MEMF_ANY | MEMF_CLEAR);
AKMODE  = AllocMem(16,MEMF_ANY | MEMF_CLEAR);

configFile = Open(CONFIGFILE,MODE_OLDFILE);
if(configFile == NULL)
    {
    printf("%s that config file for AKTerm is not found. \n",CONFIGFILE);
    printf("No problem, I start under the default mode.\n");
    strcpy(AKZFont,"FONTS:K14");
    strcpy(AKHFont,"FONTS:K14");
    strcpy(AKFEP,"NO");
    strcpy(AKLOG,"NO");
    strcpy(AKMODE,"HUGE");
    }
else
    {
    buf    = AllocMem(LENBUF,MEMF_ANY | MEMF_CLEAR);
    buf2   = AllocMem(LENBUF,MEMF_ANY | MEMF_CLEAR);
    for(i=0;i<5;i++)
        {
        if(FGets(configFile,buf,LENBUF) == NULL) break;
        j=0;k=0;
        while(j<=strlen(buf))           /* remove blank */
            {
            if((*(buf+k) != ' ') && (*(buf+k) != '\t') && (*(buf+k) != '\n'))
                {
                *(buf2+j) = *(buf+k);
                j++;
                }
            if(*(buf+k) == '\0') break;
            k++;
            }
        printf("%s\n",buf2);

        if(strncmp(buf2,"HFONT=",6)==0)
            strncpy(AKHFont,(buf2+6),strlen(buf2)-6);
        if(strncmp(buf2,"ZFONT=",6)==0)
            strncpy(AKZFont,(buf2+6),strlen(buf2)-6);
        if(strncmp(buf2,"FEP=",4)==0)
            strncpy(AKFEP,(buf2+4),strlen(buf2)-4);
        if(strncmp(buf2,"LOG=",4)==0)   
            strncpy(AKLOG,(buf2+4),strlen(buf2)-4);
        if(strncmp(buf2,"MODE=",5)==0)  
            strncpy(AKMODE,(buf2+5),strlen(buf2)-5);
        }
    FreeMem(buf,LENBUF);
    FreeMem(buf2,LENBUF);
    Close(configFile);
    }
}



VOID doDrawKanjiText(void)
{
UWORD *KanjiImageData;
ULONG l;
struct  Image *myFontImage = NULL;

KanjiImageData=AllocMem(32, MEMF_CHIP | MEMF_CLEAR);

myFontImage=(struct Image *)AllocMem(sizeof(struct Image), 
    MEMF_ANY | MEMF_CLEAR);    
myFontImage->LeftEdge    = myFont_Left;
myFontImage->TopEdge     = myFont_Top;
myFontImage->Width       = myFont_Width;
myFontImage->Height      = myFont_Height;
myFontImage->Depth       = myFont_Depth;
myFontImage->PlanePick   = 0x1;  /* use first bit-plane      */  
myFontImage->NextImage   = NULL;

SetAPen(Win->RPort,Win->RPort->BgPen);

cursolX = myFont_Left;
cursolY = myFont_Top;
RectFill(Win->RPort,0,0,WidthBitMap,HeightBitMap);

if(linePos < 0 || lineOfPage>=lineNumMax) 
    {
    linePos = 0;
    lineNum = 0;
    }
    
textPtr=(UBYTE *)TextBuffer;

if(binFlug == TRUE) 
    textPtr += (ULONG)(COLUMN * lineNum);
else
    for(l=0L;
        (l < lineNum) && (textPtr < TextBuffer+lenTextBuffer);)
        {
        if( (*(textPtr)==0x0a && *(textPtr-1)!=0x0d) 
            || (*(textPtr)==0x0a) || (*(textPtr)==0x0d))
            l++;
        textPtr++;
        }

while((cursolY < Win->GZZHeight) && (textPtr < TextBuffer+lenTextBuffer))
    {
/*
    if(cursolX > Win->GZZWidth - myFont_Width)
        {
        cursolX = myFont_Left;
        cursolY +=myFont_Height;
        }            
*/
    if(drawCharaImages(KanjiImageData,myFontImage,FOREWARD) == LF)
        l++;
    }

FreeMem(myFontImage,sizeof(struct Image));
FreeMem(KanjiImageData,32);
}



BYTE drawCharaImages(UWORD *KanjiImageData, struct Image *myFontImage,
    UBYTE direction)
{
UBYTE code,binCode,dumCode;
UWORD kCode;
int   i;
UBYTE dx,dxHalf,dy,x0;

  if(direction == FOREWARD)
      {
      x0 = myFont_Left;
      dx = myFont_Width;
      dxHalf = myFont_WidthHalf;
      dy = myFont_Height;
      code = *(textPtr++);
      }
  else if(direction == BACKWARD)
      {
      x0 = myFont_Left;
      dx = myFont_Width;
      dxHalf = myFont_WidthHalf;
      dy = myFont_Height;
      code = *(textPtr++);
      }
     
  if(code>=0x81 && code<0x9F)
        {
        kCode=(((code-0x80)<<8) & 0xFF00)
            +((*(textPtr++)-0x40) & 0x00FF);
        AKkanjiPtr=(kCode-((kCode & 0xFF00) >>2)) * 32 + (ULONG)AKkanjiFont1;
        CopyMemQuick((ULONG *)AKkanjiPtr,(ULONG *)KanjiImageData,32);
        myFontImage->ImageData   = KanjiImageData;
        DrawImage(Win->RPort,myFontImage,cursolX,cursolY);
        cursolX += dx;
        }
    else if(code>=0xE0 && code<0xEF)
        {
        kCode=(((code-0xE0)<<8) & 0xFF00)
            +((*(textPtr++)-0x40) & 0x00FF);
        AKkanjiPtr=(kCode-((kCode & 0xFF00) >>2)) * 32 + (ULONG)AKkanjiFont2;
        CopyMemQuick((ULONG *)AKkanjiPtr,(ULONG *)KanjiImageData,32);
        myFontImage->ImageData   = KanjiImageData;
        DrawImage(Win->RPort,myFontImage,cursolX,cursolY);
        cursolX += dx;
        }
    else if((code>=0x20 && code<0x7F) || (code>=0xA0 && code<=0xDF))
        {   
        AKankPtr=(code-0x20) * 32 + (ULONG)AKankFont;
        myFontImage->Width = myFont_WidthHalf;
        CopyMemQuick((ULONG *)AKankPtr,(ULONG *)KanjiImageData,32);
        myFontImage->ImageData   = KanjiImageData;
        DrawImage(Win->RPort,myFontImage,cursolX,cursolY);
        cursolX += dxHalf;
        myFontImage->Width = myFont_Width;
        }
    else if(code==0x0a)
        {
        cursolX = x0;
        cursolY += dy;
        goto RETURN_LF;
        }
    else if(code==0x0d)
        {
        cursolX = x0;
        cursolY += dy;
        goto RETURN_LF;
        }

    else if(code==0x09) /* TAB to 4 spaces */
        {
        for(i=0;i<4;i++)
            {
            cursolX += dxHalf;
            if(cursolX > Win->GZZWidth-myFont_Width)
                {
                cursolX = x0;
                cursolY +=dy;
                }            
            }
        }
    else 
        {
        if((NO0D_flug == TRUE) && (code == 0x0d)) goto RETURN_LF;
        if(code == 0x0a) goto RETURN_LF;

        for(i=0;i<2;i++)
            {
            if(i==0)
                dumCode = code / 0x10;
            else
                dumCode = code % 0x10;
            if(dumCode <= 0x09) 
                binCode = dumCode + 0x30;
            else
                binCode = dumCode + 0x37;
            AKankPtr=(binCode-0x20) * 32 + (ULONG)AKankFont;
            myFontImage->Width = myFont_WidthHalf;
            CopyMemQuick((ULONG *)AKankPtr,(ULONG *)KanjiImageData,32);
            myFontImage->ImageData   = KanjiImageData;
            DrawImage(Win->RPort,myFontImage,cursolX,cursolY);
            cursolX += dxHalf;
            }
        myFontImage->Width = myFont_Width;
        }
    return 0;
        
RETURN_LF:
    return LF;
}



VOID scrollPageUp(void)
{
if(lineNum < lineNumMax - 2 * lineOfPage)
    lineNum += lineOfPage;
else
    lineNum = lineNumMax - lineOfPage;
doNewSize();
doDrawKanjiText();
}


VOID scrollPageDown(void)
{
if(lineNum > lineOfPage)
    lineNum -= lineOfPage;
else if(lineNum <= lineOfPage)
    lineNum = 0;
doNewSize();
doDrawKanjiText();
}



VOID scrollUp(void)
{
if((lineOfPage <= lineNumMax) && (lineNum < lineNumMax - lineOfPage)) 
    {
    ScrollRaster(Win->RPort, 0, myFont_Height, 0, 0, Win->GZZWidth, 
        Win->GZZHeight);
    if(lineNum < lineNumMax)
        {
        lineNum += lineOfPage;
        doDrawKanjiLine(ROLLUP);
        lineNum -= lineOfPage;
        lineNum++;
        doNewSize();
        }    
    }
}



VOID scrollDown(void)
{
if((lineOfPage <= lineNumMax) && (lineNum > 0)) 
    {
    ScrollRaster(Win->RPort, 0, -myFont_Height, 0, 0, Win->GZZWidth, 
        Win->GZZHeight);
    lineNum--;
    doDrawKanjiLine(ROLLDOWN);
    doNewSize();
    }
}



VOID doDrawKanjiLine(UBYTE drawLine)
{
UWORD *KanjiImageData;
ULONG l;
struct  Image *myFontImage = NULL;
BYTE returnFlug = 0;
int countLF = 0;

KanjiImageData=AllocMem(32, MEMF_CHIP | MEMF_CLEAR);

myFontImage=(struct Image *)AllocMem(sizeof(struct Image), 
    MEMF_ANY | MEMF_CLEAR);    
myFontImage->LeftEdge    = myFont_Left;
myFontImage->TopEdge     = myFont_Top;
myFontImage->Width       = myFont_Width;
myFontImage->Height      = myFont_Height;
myFontImage->Depth       = myFont_Depth;
myFontImage->PlanePick   = 0x1;  /* use first bit-plane      */  
myFontImage->NextImage   = NULL;

SetAPen(Win->RPort,Win->RPort->BgPen);

if(drawLine == ROLLDOWN)
    {
    cursolX = myFont_Left;
    cursolY = myFont_Top ;
    }
else if(drawLine == ROLLUP)
    {
    cursolX = myFont_Left;
    cursolY = myFont_Top + (lineOfPage - 1) * myFont_Height;
    }
         
if(linePos < 0 || lineOfPage>=lineNumMax) 
    {
    linePos = 0;
    lineNum = 0; /* lineNum is head line of text file.*/
    }
    
/* search head line */ 
textPtr=(UBYTE *)TextBuffer;
if(binFlug == TRUE) 
    textPtr += (ULONG)(COLUMN * lineNum);
else
    for(l=0;
        (l < lineNum) && (textPtr < TextBuffer+lenTextBuffer);)
        {
        if((*(textPtr)==0x0a && *(textPtr-1)!=0x0d) 
            || (*(textPtr)==0x0a) || (*(textPtr)==0x0d))
            l++;
        textPtr++;
        }


/* draw character */
if(drawLine == ROLLDOWN)
    {
    while(returnFlug != LF)
        {
        if(textPtr < TextBuffer+lenTextBuffer)
            {
/*
            if(cursolX > Win->GZZWidth - myFont_Width)
                {
                cursolX = myFont_Left;
                cursolY +=myFont_Height;
                }            
*/
            returnFlug = drawCharaImages(KanjiImageData,myFontImage,BACKWARD);
            }
        else
            break;
        }
    }
else if(drawLine == ROLLUP)
    {
    while(countLF <= 2)
        {
        if(textPtr < TextBuffer+lenTextBuffer)
            {
/*
            if(cursolX > Win->GZZWidth - myFont_Width)
                {
                cursolX = myFont_Left;
                cursolY +=myFont_Height;
                }            
*/
            returnFlug = drawCharaImages(KanjiImageData,myFontImage,FOREWARD);
            if(returnFlug == LF) 
                countLF++;
            }
        else
            break;
        }        
    }



FreeMem(myFontImage,sizeof(struct Image));
FreeMem(KanjiImageData,32);
}

