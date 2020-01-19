# ntv
NTV is Nihongo (Japanese) Text Viewer for AmigaOS2.x-3.x.
NTV version 1.1 can only Shift-JIS Japanese character set which is widely used in Japan of the 1990s.

AmigaOS unsupports multi-byte character sets which include Japanese, Korean and Chinese.
So, I decided to display Japanese multi-byte characters as graphical images by sophisticated and powerful graphic library in AmigaOS.

TIPS
  <SCROLL>
    Left mouse button scrolls down 1 page.
    PgUp       key scrolls up   1 page.
    PgDn       key scrolls down 1 page.
    Up-arrow   key scrolls up   1 line.
    Down-arrow key scrolls down 1 line.

  <AKFontMenu>
    AKankFont submenu changes ank font.
    AKkanjiFont submenu changes kanji font.
    (But AK kanji font is only K14_KANJI_16* now.)

  <more>
    NTV can start from CLI,too.
    So, try to start from DirectoryOpus.
    You can read Japanese Shift-JIS text more easily!
        
SPECIAL THANKS
  <AKFONT>
    K14_KANJI_16A,K14_KANJI_16B,K14_ank_16 font file is from 
    AK-series Nihongo applications that are made by Masashi Tsuda.

  <INSTALLER>
    The first version of NTV installer was written by Joe Yamasaki.s

HISTORY
   ver0.1   930709 First code.
   ver0.2   930711 some bug fixed, readable MSDOS text.
   
   ver0.3   930711 some bug fixed, readable ProDOS,Mac OS text.
   
   ver0.4   930820 some bug.fixed, use K14.font,S:AKTERM.CFG              
   
   ver0.4a  931002 not display 0x0d for *Hisada-san.
   
   ver0.4b  931023 able to read a text by setting default tools in icon.
   
   ver0.50  931028 select(right mouse) button is page-up. add short-cut key for 'open' and 'quit'. 
   
   ver0.51  931219 fixed bug.
   
   ver0.6   940524 change menu text from Topaz to Screen Font. bug fixed. 
   
   ver0.7   940528 bug fixed for refresh window and more.
   
   ver0.8   940602 added AKFont Menu.
   
   ver0.90  940611 added line scroll.
   
   ver0.91  940611 CR-LF of MSDOS text can feed only ONE line.   
   
   ver1.0   941228 When S:AKTerm.CFG is not found, could run default-mode.
   
   ver1.1   951120 If under default-mode, K14 fonts are in FONTS: drawer.
   
   ------   200119 Commit to GitHub.

FUTURE
    NTV could display UTF-8 texts on AmigaOS3.1.x .

AUTHOR
    Shinji Miyoshi  shinji080@amiga.ne.jp  @miyoshiamigan
