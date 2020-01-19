# ntv
NTV is Nihongo (Japanese) Text Viewer for AmigaOS2.x-3.x.<br>
NTV version 1.1 can only Shift-JIS Japanese character set which is widely used in Japan of the 1990s.<br>

AmigaOS unsupports multi-byte character sets which include Japanese, Korean and Chinese.<br>
So, I decided to display Japanese multi-byte characters as graphical images by sophisticated and powerful graphic library in AmigaOS.<br>
<br>
1.Tips<br>
<table>
SCROLL<br>
    Left mouse button scrolls down 1 page.<br>
    PgUp       key scrolls up   1 page.<br>
    PgDn       key scrolls down 1 page.<br>
    Up-arrow   key scrolls up   1 line.<br>
    Down-arrow key scrolls down 1 line.<br>
</table>
<br>
<table>
AKFontMenu<br>
    AKankFont submenu changes ank font.<br>
    AKkanjiFont submenu changes kanji font.<br>
</table>
<br>
<table>
Misc.<br>
    NTV can start from CLI,too. So, try to start from DirectoryOpus. You can read Japanese Shift-JIS text more easily.<br>
</table>
<br>
<br>
2. Thanks<br>
<table>
AKFONT<br>
    K14_KANJI_16A,K14_KANJI_16B,K14_ank_16 font file is from AK-series Nihongo applications that are made by Masashi Tsuda.<br>
</tabel>
<br>
<table>
INSTALLER<br>
    The first version of NTV installer was written by Joe Yamasaki.<br>
</table>
<br>
<br>
3. History<br>
<table>
   ver0.1   930709 First code.<br>
   ver0.2   930711 some bug fixed, readable MSDOS text.<br>
   ver0.3   930711 some bug fixed, readable ProDOS,Mac OS text.<br>
   ver0.4   930820 some bug.fixed, use K14.font,S:AKTERM.CFG.<br>    
   ver0.4a  931002 not display 0x0d for Hisada-san.<br>
   ver0.4b  931023 able to read a text by setting default tools in icon.<br>
   ver0.5   931028 select(right mouse) button is page-up, add short-cut key for 'open' and 'quit'.<br> 
   ver0.51  931219 fixed bug.<br>
   ver0.6   940524 change menu text from Topaz to Screen Font, bug fixed.<br> 
   ver0.7   940528 bug fixed for refresh window and more.<br>
   ver0.8   940602 added AKFont Menu.<br>
   ver0.9   940611 added line scroll.<br>
   ver0.91  940611 CR-LF of MSDOS text can feed only one line.<br>
   ver1.0   941228 When S:AKTerm.CFG is not found, could run default-mode.<br>
   ver1.1   951120 If under default-mode, K14 fonts are in FONTS: drawer.<br>
   ------   200119 Commit to GitHub.<br>
</table>
<br>
4. FUTURE<br>
    NTV could display UTF-8 texts on AmigaOS3.1.x .<br>
<br>
5. AUTHOR<br>
    Shinji Miyoshi  shinji080@amiga.ne.jp  Twitter @miyoshiamigan<br>
<br>
