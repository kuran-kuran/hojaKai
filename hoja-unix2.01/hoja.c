 

/*   hoja.c */
/* Z80 gyaku asenbura hoja 2.01  UNIX gokan OS you */
/* Z80 dis assembler hoja 2.01 for UNIX compatible OS */
/* (C) Ooba Masaki 1998-4-27 */
/* compile
   UNIX cc
      cc -o hoja hoja.c ihex.c

   UNIX gcc
      gcc -o hoja hoja.c ihex.c

   MS-DOS DJGPP(MS-DOS gcc)
      gcc -DGO32 hoja.c ihex.c

   MS-DOS Microsoft-C ver6.0
      cl -AC -DMSDOSREAL hoja.c ihex.c /link /ST:50000
*/
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>
#include <fcntl.h>
#ifdef MSDOSREAL
#include <io.h>
#else
#include <unistd.h>
#include <io.h>
#endif

#ifdef GO32
#define MSDOSSS
#define OS "MS-DOS go32(80386)"
#endif
#ifdef MSDOSREAL
#ifdef _WIN32
#define OS "Windows 64bit"
#define huge
#define halloc malloc
#define hfree free
#else
#define MSDOSSS
#define OS "MS-DOS real mode(8086)"
#endif
#endif

#ifndef GO32
#ifndef MSDOSREAL
#define OS "UNIX"
#endif
#endif

#define  VERSION "2.01kai"
#define  NENGOU  "1992-1998"


#define  ERROR -1

#define  READSIZE 1024
#ifdef MSDOSREAL
unsigned LOSI = 65535;	/* location counter hairetu suu */
#else
unsigned LOSI = 32765;	/* location counter hairetu suu */
#endif

/* Sift JIS kanji dai1 byte? user teigi nozoku */
#define iskanji(c) ((c)>=0x81 && (c)<=0x9f)

/* Sift JIS kanji dai2 byte? */
#define iskanji2(c) ((c)>=0x40 && (c)<=0xfc && (c)!=0x7f)

unsigned kanhai[200];	/* kanji banci hairetu */
int Eucf;	/* Shift jis hankaku kana -> EUC */
int Kanji;	/* Shift jis kanji mojiretu check */
int kanc;	/* kanji count1 */
int kt;		/* kanji count2 */
long fsize;	/* file size */
int orgflag;	/* ORG sitei flag */
unsigned char sm[4];	/* 8 sinsuu henkan hairetu */
int m80;	/* Microsoft MACRO80 assembler flag */
int bload;	/* MSX bload file flag */
unsigned bc;	/* dis assemble address counter */
unsigned bc2;	/* location counter */
unsigned i2i,orgs,endad,start;
unsigned char jrz,cxi,buf[READSIZE],yb,yc2,yc3;
int swf;
int fd;	/* tei suijun file handle */
short	yc;
long lc,tel,tmax;
unsigned lasize;
unsigned short *labt,*labm,*labc,*labc2;
int doubleQuotation;
int decimalD;

#ifdef MSDOSREAL
unsigned short huge *locc;
unsigned char huge *mem;
#else
unsigned short *locc;
unsigned char *mem;
#endif
int adenflag,startflag,jf,jjf;
unsigned char data[8],dak,mkf,c3f;
int lf;		/* 0:lavel ari  1:label nasi */
int intel;	/* 0:Z80 1:8080 */
unsigned kanaf,deff,nagaf,notef,stjf;
int dataf;	/* dump option flag */
int dump_kana;	/* dump  0:ascii   1:ascii+kana */
long Saisyo;	/* FILE read banci */
int Saisf;	/* -S sitei flag */
extern int Kat;	/* hexfile code bunretu suu */
unsigned Katct;	/* Kat count */
int Katsk;	/* Katct syokichi */
int Hexmode;	/* 0:binary mode  1:hex file mode */
/* int Symmode;*/	/* 0:no symbol 1:symbol mode */


#ifdef ROMA_ONLY
#undef ROMAJI
#define ROMAJI
#endif

#ifdef ROMAJI
int Alpf = 1;	/* 0:kanji mode  1:roma ji mode */
#else
int Alpf = 0;	/* 0:kanji mode  1:roma ji mode */
#endif
extern unsigned Hexorg;	/* hex file org */
extern unsigned short *kawari;

unsigned long segend();
unsigned xtou();
void zcb();
void jptes();
void qusort();
int ihexck();
int codehani();	/* hex file address hani check */

void eprintf(str)
char *str;
{
  fprintf(stderr,str);
  fprintf(stderr,"\n%c",7);
  exit(1);
}

void eprintf2(str,str2)
char *str,*str2;
{
  fprintf(stderr,str,str2);
  fprintf(stderr,"\n%c",7);
  exit(1);
}

void bigerr()
{
  if(Alpf)
    eprintf("file size big. please use -S,-e option.");
  else{
/*KANJI*/    eprintf("ファイルが大きすぎる。-S,-e オプションを使ってください。");
  }
}

dataput()
{
  unsigned char dd;
  int i;
  putchar('\t');
  for(i=0;i<dak;++i)
    printf("%02X ",data[i]);
    if(dak<3) putchar('\t');
    putchar('\t');
    for(i=0;i<dak;++i){
      dd=data[i];
      if(dd>31 && dd<127)
        putchar(dd);
      else
      if(( dd>160  && dd<224 && dump_kana)){
        if(Eucf){
          /* hankaku 1byte kana -> hankaku 2byte EUC kana */
          printf("%c",0x8e);
        }
        putchar(dd);
      }
      else putchar('.');
  }
}

void rerere()
{
  printf("RET\t\t\t;%X",bc2+orgs);
  if(dataf){
    if(dump_kana)
      printf("\tC9\t\t%c",0xc9);
    else
      printf("\tC9\t\t.");
  }
   printf("\n");
   lllr();
}

long tansaku(x,v,n)
unsigned x,n;
unsigned short *v;
{
  /* 2bun tansaku */
  long low,high,mid;

  if(!n) return (long)-1;
  low=0;
  high=n-1;
  while(low<=high){
    mid=(low+high)/2;
    if(x<v[mid])
      high=mid-1;
      else
      if(x>v[mid])
        low=mid+1;
      else
      return mid;
  }
  return (long)-1;
}       

void swapp(iiii,j)
long iiii,j;
{
  /* koukan */
   unsigned temp99,temp299;

  temp99=labt[iiii];
  temp299=labc2[iiii];
  labt[iiii]=labt[j];
  labc2[iiii]=labc2[j];
  labt[j]=temp99;
  labc2[j]=temp299;
}

void qusort(left,right)
long left,right;
{
  /* quick sort */

  long iiii,last;

  if(left >= right) return;

  swapp(left,(left+right)/2);
  last=left;
  for(iiii=left+1;iiii<=right;iiii++)
    if(labt[iiii]<labt[left])
      swapp(++last,iiii);
  swapp(left,last);
  qusort(left,last-1);
  qusort(last+1,right);
}

/* SJIS kanji -> EUC kanji */
void sjis2euc(p1,p2)
unsigned char *p1,*p2;
{
  unsigned char hc1,hc2;
  int aj,rsf,csf;

  hc1 = *p1;
  aj = (*p2) < 159;
  rsf = 176;
  if(*p1 <160){
    rsf = 112;
  }
  if(aj){
    if(*p2 > 127)
      *p2 -= 32;
    else{
      *p2 -= 31;
    }
  }
  else{
   *p2 -= 126;
  }

  *p1 = ((((hc1-rsf)<<1)-aj) | 0x80);
  *p2 |= 0x80;
}

dbor(){
  deff ? printf("DB\t") : printf("DEFB\t");
}

dwor(){
      deff ? printf("DW\t") : printf("DEFW\t");
}

int is1moji(ccc)
unsigned char ccc;
{
  if(!kanaf){ if(ccc>31 && ccc<127) return 0; else return -1;}
  else
  if((ccc>31 && ccc<127) || (ccc>160 && ccc<224))
  return 0; else return -1;
}
int short iscont(ccc)
unsigned char ccc;
{
  /* kontorooru moji? */
  if(ccc==9 || ccc==0x0d || ccc==0x0a || ccc==0x27/* \' */
    || ccc==0x22 || !ccc) return 0; else return -1;
}
int short ismoji(ccc)
unsigned char ccc;
{
  /* moji? */
  if(!is1moji(ccc) || !iscont(ccc)) return 0;  else return -1;
}


int iskana(ccc)
unsigned char ccc;
{
  /* kana? */
  if(ccc>165 && ccc<224 || ccc==' ') return 0; else return -1;
}

int iseigo(ccc)
unsigned char ccc;
{
  /* eiji? */
  if((ccc>64 && ccc<91) || (ccc>96 && ccc<123)) return 0; else return -1;
}

unsigned r16(aaag,bbbg)
unsigned aaag; 
unsigned bbbg;
{
  /* aaa=r16(0x12,0x34); ->  aaa==0x1234 */

  return aaag*256+bbbg;
}

he16()
{
  unsigned tppp;
  tppp=r16((unsigned)yc,(unsigned)yc2);
  printf("0%XH",tppp);
  if(jf){
    if(jjf){
      if(tppp>0xff) swf=2;
    }
    else
    if(tppp>0xfff) swf=2;
  }
}  

/* LD label taiou 1996-9-1 */
he16_2()
{
  unsigned tppp;
  if(bc2==labm[tel] && tel < lc){
    if(jjf==1) swf=2;
    printf("Z%04d",labc[tel++]);
  }
  else{
    tppp=r16((unsigned)yc,(unsigned)yc2);
    printf("0%XH",tppp);
    if(jf){
      if(jjf){
        if(tppp>0xff) swf=2;
      }
      else
      if(tppp>0xfff) swf=2;
    }
  }
}

jwj()
{
  unsigned jrstemp;
  if(!stjf){
    if(decimalD) {
      printf("$%c%dD",jrz,yc);
    } else {
      printf("$%c%d ",jrz,yc);
    }
    if(jjf==1 && yc>9) swf=2;
    else
    if(!jjf && yc>99) swf=2;
  }
  else{
    yc=(unsigned short)mem[bc-1];
    if(yc>127) yc-=256;
    jrstemp=orgs+bc+yc;
    printf("0%XH",jrstemp);
    if(jjf==1 && jrstemp>0xff) swf=2;
    else
    if(!jjf && jrstemp>0xfff) swf=2;
  }
}
jpcasub(dochi)
int dochi;
{
  int sf;
  sf = 0;
  if(bc2==labm[tel] && tel < lc){
    if(jjf==1) swf=2;
      printf("Z%04d",labc[tel++]);  sf=1; 
  }
  if(!sf){
    (dochi) ? jwj() : he16();
  }
}


void jcky()
{
  swjk();
  putchar(',');
  switch(sm[1]){
    case 0:
    case 2:
    case 4:
    case 5: jjf=1;
  }
  jf=1;
  jpcasub(0);
}

setumei() 
{
  /* setumei hyouji */
if(!Alpf){
/*KANJI*/ /* printf("使い方: hoja [-[oesSl] <16進数値>] [-y シンボリックファイル] [-mudhkKCHnNigbAN] 読込ファイル\n"); */
/*KANJI*/ printf("使い方: hoja [-[oesSl] <16進数値>] [-mudhkKCHnNigbAN] 読込ファイル\n");
/*KANJI*/ printf("   xxxは16進数。 ORGデフォルトは100H\n");
/*KANJI*/ printf("     -m  Microsoft M80用のリスト出力。\n");
/*KANJI*/ printf("     -oxxx  ORG出力の値指定。 例えば -o8B34\n");
/*KANJI*/ printf("     -sxxx  開始番地。例えば -s20A なら 20A番地から\n");
/*KANJI*/ printf("     -exxx  終了番地。例えば -e5CA なら 5CA番地まで\n");
/*KANJI*/ printf("            (-s と -e はターゲットのメモリ番地を指定します。)\n");
/*KANJI*/ printf("            (注意:メモリを読むわけではなく、ファイルから読む)\n");
/*KANJI*/ printf("     -Sxxx  ファイルの先頭から指定した値バイト無視。0～\n");
/*KANJI*/ printf("     -u  JR命令を絶対表記にする。例えば \"JR 230H\"\n"); 
/*KANJI*/ printf("     -d  ダンプ表示付加、(半角カタカナ無し、コメント部分)\n");
/*KANJI*/ printf("     -h  ダンプ表示付加、(半角カタカナ含む、コメント部分)\n");
/*KANJI*/ printf("     -lxxx  ラベル処理配列の大きさの指定。0～FFFFH\n");
/*KANJI*/ printf("            0を指定するとラベルをつけません。\n");
/*KANJI*/ printf("            数値を小さくするとメモリを喰わず高速だがラベル減る。\n");
/*KANJI*/ printf("     -k  1バイト半角カタカナの文字列も判別しようとする。\n");
/*KANJI*/ printf("     -K  シフトJIS全角の文字列も判別しようとする。\n");
/*KANJI*/ printf("     -C  シフトJIS全角、1バイト半角カタカナの出力をＥＵＣに変換。\n");
/*KANJI*/ printf("     -n  文字列を判別しない。\n");
/*KANJI*/ printf("     -H  インテルHEXファイルを逆アセンブル。\n");
/*KANJI*/ /* printf("     -yファイル名       SID互換のシンボリックファイルを読み込む\n"); */
/*KANJI*/ printf("     -i  8080コードのみニーモニックに変換する。(ザイログニーモニック)\n");
/*KANJI*/ printf("     -g  DEFB疑似命令を DB に変える。\n");
/*KANJI*/ printf("     -b  MSXのbloadファイルを逆アセンブルする。\n");
/*KANJI*/ printf("     -q  文字列を\"で囲む\n");
/*KANJI*/ printf("     -p  10進数にDを付けない\n");
#ifdef ROMAJI
/*KANJI*/ printf("     -A  エラーや説明等のメッセージをローマ字で出力する。(デフォルト)\n");
/*KANJI*/ printf("     -N  エラーや説明等のメッセージを全角で出力する。\n");
#else
/*KANJI*/ printf("     -A  エラーや説明等のメッセージをローマ字で出力する。\n");
/*KANJI*/ printf("     -N  エラーや説明等のメッセージを全角で出力する。(デフォルト)\n");
#endif
/*KANJI*/ printf("\n     Z80逆アセンブラ hoja ver ");
}
else{
/* printf("usage: hoja [-[oesSl] <hexnum>] [-y symbolicfile] [-mudhkKCHnigbAN] readfile\n"); */
printf("usage: hoja [-[oesSl] <hexnum>] [-mudhkKCHnigbAN] readfile\n");
printf("   xxx hexnum. ORG default 100H.\n");
printf("     -m  Microsoft M80 list.\n");
printf("     -oxxx  ORG address(hex). for example -o8B34\n");
printf("     -sxxx  start address(hex).  for example -s20A\n");
printf("     -exxx  end address(hex).  for example -e5ca\n");
printf("     -Sxxx  fairu sentou kara shitei shita atai(hex)baito musi 0-\n");
printf("     -u  for example \"JR $+15\" -> \"JR 230H\"\n"); 
printf("     -d  ascii code dump\n");
printf("     -h  ascii code and KATAKANA dump\n");
printf("     -lxxx  raberu hairetu no ookisa wo shitei.0-FFFFH\n");
printf("            0 wo shitei suru to raberu wo tukemasen.\n");
printf("            suuchi wo chiisaku suru to memori wo kuwazu hayai ga\n");
printf("            raberu no kazu wa herimasu.\n");
printf("     -k  1byteKATAKANA no mojiretu mo hanbetu.\n");
printf("     -K  shiftJIS no mojiretu mo hanbentu.\n");
printf("     -C  shiftJIS and 1byteKATAKANA output -> EUC\n");
printf("     -n  mojiretu wo hanbetu shinai.\n");
printf("     -H  intel HEX file dis assemble\n");
/* printf("     -y filename    SID simbolic file read.\n"); */
printf("     -i  8080 code only dis assemble\n");
printf("     -g  DEFB -> DB\n");
printf("     -b  MSX bload file dis assemble\n");
printf("     -q  Enclose the string in double quotes\n");
printf("     -p  Do not append 'D' to decimal numbers\n");
#ifdef ROMAJI
printf("     -A  ROMA JI message(default)\n");
#ifndef ROMA_ONLY
printf("     -N  ZENKAKU MOJI message\n");
#endif
#else
printf("     -A  ROMA JI message\n");
printf("     -N  ZENKAKU MOJI message(default)\n");
#endif
printf("\n     Z80 dis assembler hoja ver ");
}
printf(VERSION);
printf(" for %s",OS);
printf(" (C) Ooba Masaki ");
printf(NENGOU);
printf(" Modified by kuran_kuran 2025 ");
printf("\n");
exit(1);
}

void haihai(aaa)
char *aaa;
{
  if(Alpf)
    eprintf2("file cannot open[%s]",aaa);
  else{
/*KANJI*/    eprintf2("ファイルを開けません。[%s]",aaa);
  }
}

lllr()
{
  /* raberu (hidarigawa) hyouji */

  int ab;
  long ssz;
  bc2=bc;
  yb=mem[bc];
  if(fsize<=bc)
    owari(0);
  if(Hexmode){
    ab = kaow();
    if(ab == -1)
      owari(0);
    else
    if(ab == 1){
      hexsegow(0);
      yb = mem[bc];
    }
  }
  bc2=bc;
/*
  if(Symmode){
  }
*/
  if((ssz=tansaku(bc,labt,lc))!=(long)ERROR){
    printf("Z%04u:",labc2[ssz]);
  }
  ++bc;
}




void smch()
{
  sm[0]-='0';
  sm[1]-='0';
  sm[2]-='0';
}


/* Z80 kakucyou code CB */
void zcb()
{
  yc2=mem[bc];
  endcheck(3);
  sprintf(sm,"%03o",yc2);
  smch();
  switch(sm[0]){
    case 1: printf("BIT");
            birese();
            return;
    case 2: printf("RES");
            birese();
            return;
    case 3: printf("SET");
            birese();
            return;
  }
  if(sm[0]==0){
    switch(sm[1]){
      case 0: printf("RLC"); break;
      case 1: printf("RRC"); break;
      case 2: printf("RL"); break;
      case 3: printf("RR"); break;
      case 4: printf("SLA"); break;
      case 5: printf("SRA"); break;
      case 7: printf("SRL"); break;
      default: dxxx(); printf("\n\t;*SLL"); /* SLL MITEIGI MEIREI  2/4 */
    }
    putchar('\t');
    swsm(sm[2]); /* 2/4 */
    if(sm[1]==6)putchar('\t'); /* 2/4 */
  }
  else
  dxxx();
}

void aokage()
{
  if(decimalD) {
    printf("(I%c%c%dD)",cxi,jrz,yc);
  } else {
    printf("(I%c%c%d) ",cxi,jrz,yc);
  }
}


/* label taiou 1996-9-1 */
hifuro_2()
{
  if(bc2==labm[tel] && tel < lc ){
    printf("(Z%04d)",labc[tel++]);
  }
  else{
    printf("(0%XH)",r16((unsigned)yc,(unsigned)yc2));
  }
}

void yomi(long *tbc)
{
  int rsi;
  rsi = read(fd,buf,READSIZE);
  if(1>rsi){
    if(Alpf)
      eprintf("file read error");
    else{
/*KANJI*/      eprintf("ファイル読み込みエラー");
    }
  }
  memcpy(&mem[*tbc],buf,rsi);
  *tbc+=rsi;
}

void fugou()
{
  if(yc>127) {yc-=256;}
  if(yc>=0) jrz='+';
  else{
    jrz='-';
    yc=(-yc);
  }
}


unsigned xtou(str)
char *str;
{
  /* 16sinsuu mojiretu -> unsigned */
  unsigned s;
  char c;
  s=0;
  while(*str !=0){
    if(('a'-1)< *str && *str < ('f'+1))
      c= (*str & 0x9f)+9;
    else if(('A'-1)< *str && *str <('F'+1))
      c= (*str & 0xbf)+9;
    else if(('0'-1)< *str && *str <('9'+1))
      c=*str & 0x0f;
    else 
      c=0;
    s=s*16+c;
    ++str;
  }
  return s;
}


long xtol(str)
char *str;
{
  /* 16sinsuu mojiretu -> long */
  long s;
  char c;
  s=0;
  while(*str !=0){
    if(('a'-1)< *str && *str < ('f'+1))
      c= (*str & 0x9f)+9;
    else if(('A'-1)< *str && *str <('F'+1))
      c= (*str & 0xbf)+9;
    else if(('0'-1)< *str && *str <('9'+1))
      c=*str & 0x0f;
    else 
      c=0;
    s=s*(long)16+(long)c;
    ++str;
  }
  return s;
}

void fdo(inpname)
char *inpname;
{
#ifdef MSDOSREAL
  if((fd=open(inpname,O_RDONLY | O_BINARY))==ERROR)
#else
#ifdef GO32
  if((fd=open(inpname,O_RDONLY | O_BINARY))==ERROR)
#else
  if((fd=open(inpname,O_RDONLY))==ERROR)
#endif
#endif
    haihai(inpname);
}

/* SHIFT JIS KANJI ka? */
int kanz(ct,ct2)
unsigned char ct,ct2;
{
  if(iskanji(ct)&& iskanji2(ct2)){
    return 1;
  }
  return 0;
}


/* mojiretu kensa */
int moji_check(jdu)
int *jdu;
{
  int mina;
  int at;
  unsigned char ct;
  long sege;
  int tppp;
  int jidesu;
  int dame;
  int i;

  jidesu=tppp=0;
  kt = kanc = 0;
  sege = Hexmode ? segend():fsize;
  while(1){
    at = 0;
    if(!ismoji(mem[bc+tppp])){
    }
    else
    if(!Kanji){
      break;
    }
    else
    if(iskanji(mem[bc+tppp])){
      if(bc+tppp+2<=sege){
        if(iskanji2(mem[bc+tppp+1])){
/*
          kanhai[kanc] = bc+tppp;
          ++kanc;
          ++tppp;
*/
          at=1;
        }
        else
          break;
      }
      else{
        break;
      }
    }
    else{
      break;
    }
            
    if(!jidesu){
      if(Kanji){
        for(dame=i=0;i<5 && bc+tppp+i+1<sege;++i){	/* 1996 */
          ct = mem[bc+tppp+i];
          if((kanaf && !iskana(ct)) || !iseigo(ct) ||
            (bc+tppp+i+1<sege && kanz(ct,mem[bc+tppp+i+1]))){
       
            if(iskanji(ct)){
              if(iskanji2(mem[bc+tppp+i+1])){
                kanhai[kanc] = bc+tppp;
                ++kanc;
                ++i;
              }
            }
          }
          else
            dame = 1;
        }
        if(!dame)
          jidesu = 1;
      }
      else{
        if(kanaf){
          for(dame=i=0;i<5 && bc+tppp+i+1<sege;++i){	/* 1996 */
            if(iskana(mem[bc+tppp+i])){dame=1; break;}
          }
          if(!dame) jidesu=1;
        }
        if(!jidesu){
          for(dame=i=0;i<5 && bc+tppp+i<sege;++i){	/* 1996 */
            if(iseigo(mem[bc+tppp+i])){dame=1;break;}
          }
          if(!dame) jidesu=1;
        }
      }
    }
    mina=0;
    while(!mem[bc+tppp+mina++]){
      if(bc+tppp+mina+1>sege)		/* 1996 */
        break;
        if(mina>8)break;
    }
    if(mina>8)break;
    if(bc+tppp+1>sege)
      break;
    ++tppp;
    tppp+=at;
  } 
  *jdu = jidesu;
  return tppp;
}

/*******************************************/
/* PASS 2,3 dis assemble */
/*******************************************/
mroop()
{
  unsigned char kc1,kc2;
  long sege;
  int tppp;
  int jidesu;
  int i;

  if(Hexmode)
    bc = Hexorg;
  else
    bc=0;
  Katct = Katsk;
  if(!lf) lavtes();	/* PASS 2 */

  /* PASS 3 */

  if(lc && !lf)
    qusort((long)0,(long)lc-(long)1);
  sege = Hexmode ? segend():fsize;

/* debug   
printf("i2i=%d\tlc=%d\n",i2i,lc); 
for(i=0;i<16;++i){
  printf("locc[%u]=%X\t",i,locc[i]+orgs);
  if(!(i%5))
    printf("\n");
}
*/
/*
for(i=0;i<30;++i){
  printf("labm[%u]=%X\t",i,labm[i]+orgs);
  if(!(i%10))
    printf("\n");
}
for(i=0;i<30;++i){
  printf("labt[%u]=%X\t",i,labt[i]+orgs);
  if(!(i%10))
    printf("\n");
}
*/
  if(Hexmode)
    bc = Hexorg;
  else
    bc=0;
  tel=0;
  Katct = Katsk;

  while(1){

    bc2=bc;
    jf=jjf=kt=kanc=0;
    swf = 0;
    lllr();


   if(!notef){
    if(yb==0xc9 /* RET */ || c3f /* JP */){
      if(!c3f){
        putchar('\t');
        rerere();
      }

        --bc;
      tppp = moji_check(&jidesu);
        ++bc;
          if(tppp>8 && bc+tppp-1<=sege && jidesu){
        
#define KETASUU 45


            putchar('\t');
            
              --bc;
              dbor();
              if(!is1moji(mem[bc])){
                /* hankaku 1byte kana-> hankaku EUC 2 byte kana */
                if(Eucf && !iskana(mem[bc])){
                  if (doubleQuotation) {
                    printf("\"%c%c", 0x8e, mem[bc++]);
                  } else {
                    printf("\'%c%c", 0x8e, mem[bc++]);
                  }
                }
                else
                {
                  if (doubleQuotation) {
                    printf("\"%c", mem[bc++]);
                  } else {
                    printf("\'%c", mem[bc++]);
                  }
                }
                mkf=1;
              }
              else{
                if(kt<kanc && bc == kanhai[kt]){		/* 1996 */
                  kc1 = mem[bc];
                  kc2 = mem[bc+1];
                  if(Eucf){
                    sjis2euc(&kc1,&kc2);	/* SJIS->EUC */
                  }
                  if (doubleQuotation) {
                    printf("\"%c%c", kc1, kc2);/* 1996 SJIS,EUC kanji put */
                  } else {
                    printf("\'%c%c", kc1, kc2);/* 1996 SJIS,EUC kanji put */
                  }
                  bc += 2;
                  mkf = 1;
                  ++kt;
                  --tppp;

                }
                else{
                  printf("0%XH,",mem[bc++]);
        	  mkf=0;	/* 2/2 {}*/
                }
              }
            i=0;
            nagaf=0;
            --tppp;
            while(tppp--){
              if(i>KETASUU && !(kt<kanc && kanhai[kt]==bc)){
                i=0;
                if(mkf) {
                  if (doubleQuotation) {
                    printf("\"");
                  } else {
                    printf("\'");
                  }
                  mkf=0;
                }
                if(!nagaf) printf("\t;%X",bc2+orgs);
                printf("\n\t");
                dbor();
                nagaf=1;
              }
              if(!iscont(mem[bc])){
        	if(mkf){
              if (doubleQuotation) {
        	    printf("\",");
              } else {
        	    printf("\',");
              }
                  ++i;
        	  mkf=0;
        	}
        	printf("0%XH",mem[bc]);
                i+=4;
                if(tppp && i<KETASUU) { putchar(','); ++i;}
        	else i=KETASUU+10; /* i>=KETASUU */ /* 2/2 */
              }
              else{
           	if(!mkf){
              if (doubleQuotation) {
        	    putchar('\"');
              } else {
        	    putchar('\'');
              }
                ++i;
        	  mkf=1;
                }
                if(!Eucf)
                  putchar(mem[bc]);
                if(Eucf){
                  if(iskanji(mem[bc]) && iskanji2(mem[bc+1])){
                    kc1 = mem[bc];
                    kc2 = mem[bc+1];
                    sjis2euc(&kc1,&kc2);	/* SJIS->EUC */
                    printf("%c%c",kc1,kc2);/* 1996 SJIS,EUC kanji put */
                    ++bc;
                    ++i;
                    --tppp;
                  }
                  else{
                    if(!iskana(mem[bc])){
                      printf("%c",0x8e);
                    }
                    putchar(mem[bc]);
                  }
                }
                if(kt<kanc && bc == kanhai[kt]){		/* 1996 */
                  ++kt;
                }
                ++i;
        	if(!tppp) {putchar('\''); ++i;}
              }
              ++bc;
            }
            if(!nagaf) printf("\t;%X\n",bc2+orgs);
            else printf("\n");
            lllr();
          }
    }
   }

    *data=yb;
    dak=1;
    putchar('\t');
    c3f=0;
   
    sprintf(sm,"%03o",yb);
    smch();
    if(sm[0]==1){
      if(yb==0x76) printf("HALT\t");
      else{
        printf("LD	");
        swsm(sm[1]);
        putchar(',');
        swsm(sm[2]);
      }
    /* putchar('\n'); */
      
    }

    else
    if(sm[0]==2){
      switch(sm[1]){
        case 0: printf("ADD	A,"); break;
        case 1: printf("ADC	A,"); break;
        case 2: printf("SUB	");   break;
        case 3: printf("SBC	A,"); break;
        case 4: printf("AND	");   break;
        case 5: printf("XOR	");   break;
        case 6: printf("OR	");   break;
        case 7: printf("CP	");
      }
      swsm(sm[2]);
    }

    else
    if(sm[0]==3){
      if(sm[2]==2){
        matome1();
        printf("JP	");
        jcky();
      }
      else
      if(sm[2]==4){
        matome1();
        printf("CALL	");
        jcky();
      }

      else
      if(sm[2]==0){
        printf("RET	");
        swjk();
      }
      else
      if(sm[2]==7){
        printf("RST	");
        switch(sm[1]){
          case 0: printf("00H"); break;
          case 1: printf("08H"); break;
          case 2: printf("10H"); break;
          case 3: printf("18H"); break;
          case 4: printf("20H"); break;
          case 5: printf("28H"); break;
          case 6: printf("30H"); break;
          case 7: printf("38H");
        }
      }
      else
      if(sm[2]==1){
        switch(sm[1]){
          case 1: printf("RET\t");      break;
          case 0: printf("POP	BC");   break;
          case 2: printf("POP	DE");   break;
          case 3: if(!intel) printf("EXX\t");
                  else{
                    dbor();
                    printf("0D9H");
                  }
 	          break;
          case 4: printf("POP	HL");   break;
          case 5: printf("JP	(HL)"); break;
          case 6: printf("POP	AF");   break;
           case 7: printf("LD	SP,HL");
        }
      }
      else
      if(sm[2]==6){
        yc=(unsigned short)mem[bc];
        endcheck(1);
        switch(sm[1]){
          case 0: printf("ADD	A,"); break;
          case 1: printf("ADC	A,"); break;
          case 2: printf("SUB	");   break;
          case 3: printf("SBC	A,"); break;
          case 4: printf("AND	");   break;
          case 5: printf("XOR	");   break;
          case 6: printf("OR	");   break;
          case 7: printf("CP	");
        }
        printf("0%XH",yc);
      }
      else
      if(sm[2]==3){
        switch(sm[1]){
          case 0: matome1();


                  printf("JP	");
        	  jpcasub(0);
        	  swf=0;
                  /* JP wa hikui banci ni tonde ikumonowa mojiretu nasi 
                     8byte wo koete takai banci ni tonde iku to
                     mojiretu no kensa wo suru */
        	  if(bc+orgs+8<r16((unsigned)yc,(unsigned)yc2))
                     c3f=1;
                  break;

          case 1: if(!intel) zcb();
                  else{
                    dbor();
                    printf("0CBH");
                  }
        	  break;
          case 2:  yc=(unsigned short)mem[bc];
                  endcheck(1);
                  printf("OUT	(0%XH),A",yc);
        	  if(yc>0xf) swf=2;
                  break;
          case 3:  yc=(unsigned short)mem[bc];
                  endcheck(1);
                  printf("IN	A,(0%XH)",yc);
        	  if(yc>0xf) swf=2;
        	  break;
          case 4: printf("EX	(SP),HL");        break;
          case 5: printf("EX	DE,HL");          break;
          case 6: printf("DI\t");                 break;
          case 7: printf("EI\t");
        }
        /* putchar('\n'); */  
      }

      else
      if(sm[2]==5){
        switch(sm[1]){
          case 1: matome1();
        	  printf("CALL	");
        	  jpcasub(0);
        	  swf=0;
        	  break;
          case 0: printf("PUSH	BC"); break;
          case 2: printf("PUSH	DE"); break;
          case 4: printf("PUSH	HL"); break;
          case 6: printf("PUSH	AF"); break;

          case 5: if(!intel) zed();
                  else{
                    dbor();
                    printf("0EDH");
                  }
                   break;
          default: if(!intel) zddfd();
                   else{
                     dbor();
                     printf("0%XH",yb);
                   }
                   break;
        }
        /* putchar('\n'); */
      }
    }
    else 
    if(sm[0]==0){              /*sm[0]==0*/
      if(sm[2]==6){
        yc=(unsigned short)mem[bc];
        endcheck(1);
        printf("LD	");
        swsm(sm[1]);
        if(sm[1]==6) swf=2;
        putchar(',');
        printf("0%XH",yc);
      }
      else
      if(sm[2]==4){
        printf("INC	");
        swsm(sm[1]);
      }
      else
      if(sm[2]==5){
        printf("DEC	");
        swsm(sm[1]);
      }
      else
      if(sm[2]==3){  /* inc 16 | dec 16 */
        !(sm[1]%2) ? printf("INC	") : printf("DEC	");
        swpr();
      }
      else
      if(sm[2]==2){ /* ld (banci) */
        if(sm[1]>3) matome1();
        printf("LD	");
        switch(sm[1]){
          case 0: printf("(BC),A"); break;
          case 1: printf("A,(BC)"); break;
          case 2: printf("(DE),A"); break;
          case 3: printf("A,(DE)"); break;
          case 4: hifuro_2(); 
          	  printf(",HL");	/* 0x22 LD (nn),HL */
        	  swf=2;
        	  break;
          case 5: printf("HL,");	/* 0x2a LD HL,(nn) */
        	  hifuro_2();
        	  swf=2;
        	  break;
          case 6: hifuro_2();		/* 0x32 LD (nn),A */
        	  printf(",A");
        	  if(0xf< r16((unsigned)yc,(unsigned)yc2))
                    swf=2;

        	  break;
          case 7: printf("A,");		/* 0x3a LD A,(nn) */
        	  hifuro_2();
        	  if(0xf< r16((unsigned)yc,(unsigned)yc2))  swf=2;

        }
        /* putchar('\n'); */
      }        
      else
      if(sm[2]==1){
        if(!(sm[1]%2)){	/* LD HL,nn|LD BC,nn|LD DE,nn|LD SP,nn */
          matome1();
          printf("LD\t");
          jf=jjf=1;
          swpr();
          putchar(',');
          he16_2();	/* label taiou 1996-9-1 */
        }
        else {
          printf("ADD	HL,");
          swpr();
        }                    
        /* putchar('\n'); */  
      }
      else
        if(sm[2]==7){
        switch(sm[1]){
          case 0: printf("RLCA"); break;
          case 1: printf("RRCA"); break;
          case 2: printf("RLA");  break;
          case 3: printf("RRA");  break;
          case 4: printf("DAA");  break;
          case 5: printf("CPL");  break;
          case 6: printf("SCF");  break;
          case 7: printf("CCF");
        }      
        printf("\t");
      }
      else
      if(!yb) printf("NOP\t");
      else
      mrsu();
        
    }  
    else
    fprintf(stderr,"BUG?");
    if(!swf) printf("\t");
    printf("\t;%X",bc2+orgs); 
    if(dataf){
      dataput();
    }
    printf("\n");
 }
}


mrsu()
{
    if(sm[2]==0){
      if(!intel){
        if(sm[1]==1){
          printf("EX	AF,AF\'");
        }
        else{
        yc=yc2=(unsigned short)mem[bc];		/* 93/1/22 yc2 tuketasi */
        endcheck(1);
        yc+=2;
        fugou();

        if(jrz!='-' || yc<127){		/* 93/1/22 */
        switch(sm[1]){
          /* case 1: printf("EXAF,AF\'");   break; */
          case 2: printf("DJNZ\t");
                  jjf=3;
                  break;
          case 3: printf("JR\t");
                  jjf=3;
                  break;
          case 4: printf("JR	NZ,");
                  jjf=1;
                  break;
          case 5: printf("JR	Z,");
                  break;
          case 6: printf("JR	NC,");
                  jjf=1;
                  break;
          case 7:
                  printf("JR	C,");

        }
        jpcasub(1);
      }
      else{	/* 93/1/22 */
          dbor();
          printf("0%xH,0%xH",yb,yc2);	/* 93/1/22 */
      }
      }
    }
    else{
     dbor();
     printf("0%XH",yb);
   }
  }
}

option(argc,argv,symname)
short argc;
char **argv;
char **symname;
{
  int i;

  doubleQuotation = 0;
  decimalD = 1;

#ifndef GETOPT
  int tugia;
  int lof;	/* arg option flag */
#endif

#ifdef GETOPT
  int c;


  /* getopt() use */
  int digit_optind = 0;

  while (1){
     int this_option_optind = optind ? optind : 1;
     c = getopt(argc, argv, "o:mbonl:e:s:S:y:KCHAdhikug");
     if (c == -1)
       break;
     switch (c){
#else
  /* getopt() not use */
  for(i=1;i<argc-1;++i){
    tugia = 0;
    lof = 0;
    if(argv[i][tugia]=='-'){
     while(1){
       switch (argv[i][tugia+1]){
#endif
         case 'g':
           deff=1;
           break;
  
         case 'm':
           m80=1;
           break;

#if 0
         case 'y':
           Symmode = 1;
#ifdef GETOPT
           *symname = optarg;
#else
           if(!argv[i][tugia+2]){
             ++i;
             if(i>=argc-1){
               if(Alpf){
                 eprintf("-y symbolicfile");
               }
               else{
/*KANJI*/        eprintf("-y オプションの後ろはシンボリックファイルの指定が必要です。");
               }
             }
             else{
               if(argv[i][0]!='-')
                 *symname = argv[i];
               else{
                 if(Alpf){
                   eprintf("-y symbolicfile");
                 }
                 else{
/*KANJI*/          fprintf(stderr,"もし、-で始まるシンボリックファイルなら、./-filenameとして下さい。\n");
/*KANJI*/          eprintf("-y オプションの後ろはシンボリックファイルの指定が必要です。");
                 }
               }
             }
           }
           else{
             orgs=xtou(&argv[i][tugia+2]);
           }
           lof = 1;	/* arg option flag */
#endif
           break;
#endif
  
         case 'b': 
           if(Hexmode || orgflag || startflag){
             if(Alpf)
               eprintf("-b wa -H,-o,-s to douji ni tukaemasen.(nihongo)");
             else{
/*KANJI*/      eprintf("-b オプションは -H,-o,-sオプションと併用できません。");
             }
           }
           bload=1;
           break;
         case 'o': 
           if(Hexmode || bload){
             if(Alpf)
               eprintf("-o wa -H,-b to douji ni tukaemasen.(nihongo)");
             else{
/*KANJI*/      eprintf("-o オプションは -H,-bオプションと併用できません。");
             }
           }

#ifdef GETOPT
           orgs=xtou(optarg);
#else
           if(!argv[i][tugia+2]){
             ++i;
             if(i>=argc-1){
               orgs=0;
             }
             else{
               if(argv[i][0]!='-')
                 orgs=xtou(&argv[i][0]);
               else{
                 orgs = 0;
                 --i;
               }
             }
           }
           else{
             orgs=xtou(&argv[i][tugia+2]);
           }
           lof = 1;	/* arg option flag */
#endif
           orgflag=1;
           break;
         case 'n':
           if(kanaf || Kanji){
             if(Alpf)
               eprintf("-n wa -k,-K to douji ni tukaemasen.(nihongo)");
             else{
/*KANJI*/      eprintf("-n オプションは -k,-Kオプションと併用できません。");
             }
           }
           notef=1;
           break;
         case 'l':
           
#ifdef GETOPT
           LOSI=xtou(optarg);
#else
           if(!argv[i][tugia+2]){
             ++i;
             if(i>=argc-1){
               LOSI=0;
             }
             else{
               if(argv[i][0]!='-')
                 LOSI=xtou(&argv[i][0]);
               else{
                 LOSI = 0;
                 --i;
               }
             }
           }
           else{
             LOSI=xtou(&argv[i][tugia+2]);
           }
           lof = 1;	/* arg option flag */
#endif
           if(!LOSI)
             lf=1;
           break;
         case 'e':
#ifdef GETOPT
           endad=xtou(optarg);
#else
           if(!argv[i][tugia+2]){
             ++i;
             if(i>=argc-1){
               endad=0;
             }
             else{
               if(argv[i][0]!='-')
                 endad=xtou(&argv[i][0]);
               else{
                 endad = 0;
                 --i;
               }
             }
           }
           else{
             endad=xtou(&argv[i][tugia+2]);
           }
           lof = 1;	/* arg option flag */
#endif
           adenflag=1;
           break;
         case 'S':
           if(Hexmode){
             if(Alpf)
               eprintf("-S wa -H to douji ni tukaemasen.(nihongo)");
             else{
/*KANJI*/      eprintf("-S オプションは -H オプションと併用できません。");
             }
           }
#ifdef GETOPT
           Saisyo = xtol(optarg);
#else
           if(!argv[i][tugia+2]){
             ++i;
             if(i>=argc-1){
               Saisyo=0;
             }
             else{
               if(argv[i][0]!='-')
                 Saisyo=xtol(&argv[i][0]);
               else{
                 Saisyo = 0;
                 --i;
               }
             }
           }
           else{
             Saisyo = xtol(&argv[i][tugia+2]);
           }
           lof = 1;	/* arg option flag */
#endif
           if(Saisyo < 0){
             if(Alpf)
               eprintf("-S no sitei ga mainasu.(nihongo)");
             else{
/*KANJI*/      eprintf("-S オプションの指定値がマイナス値になっています。");
             }
           }
           Saisf = 1;
           break;
         case 'H':
           if(orgflag || bload || Saisyo){
             if(Alpf)
               eprintf("-H wa -o,-b,-S to douji ni tukaemasen.(nihongo)");
             else{
/*KANJI*/      eprintf("-H オプションは -o,-b,-S オプションと併用できません。");
             }
           }
           Hexmode = 1;
           break;
         case 'K':
           if(notef){
             if(Alpf)
               eprintf("-n wa -k,-K to douji ni tukaemasen.(nihongo)");
             else{
/*KANJI*/      eprintf("-n オプションは -k,-Kオプションと併用できません。");
             }
           }
           Kanji = 1;
           break;
         case 'C':
           Eucf = 1;
           break;
         case 'd': 
           dataf=2;
           if(dump_kana){
             if(Alpf)
               eprintf("-d to  -h wa douji ni tukaemasen(nihongo)");
             else{
/*KANJI*/      eprintf("-d と -h は 併用できません。");
             }
           }
           dump_kana = 0;
           break;
         case 'h':
           if(dataf == 2){
             if(Alpf)
               eprintf("-d to  -h wa douji ni tukaemasen(nihongo)");
             else{
/*KANJI*/      eprintf("-d と -h は 併用できません。");
             }
           }
           dataf=1;
           dump_kana = 1;
           break;
         case 'i':
           intel=1;
           break;
         case 'A':
           Alpf = 1;	/* roma ji mode */
           break;
         case 'k': 
           if(notef){
             if(Alpf)
               eprintf("-n wa -k,-K to douji ni tukaemasen.(nihongo)");
             else{
/*KANJI*/      eprintf("-n オプションは -k,-Kオプションと併用できません。");
             }
           }
           kanaf=1;
           break;
         case 's':
           if(bload){
             operr1();
           }
#ifdef GETOPT
           start=xtou(optarg);
#else
           if(!argv[i][tugia+2]){
             ++i;
             if(i>=argc-1){
               start=0;
             }
             else{
               if(argv[i][0]!='-')
                 start=xtou(&argv[i][0]);
               else{
                 start = 0;
                 --i;
               }
             }
           }
           else{
             start=xtou(&argv[i][tugia+2]);
           }
           lof = 1;
#endif
           startflag=1;
           break;
         case 'u':
           stjf=1;
           break;
         case 'N':
#ifndef ROMA_ONLY
           Alpf = 0;
#endif
           break;
  
         case 'q':
           doubleQuotation = 1;
           break;

         case 'p':
           decimalD = 0;
           break;

         default:
#ifdef GETOPT
           if(Alpf){
             fprintf (stderr,"Invalid option '-%c'",c);
             exit(1);
           }
           else{
/*KANJI*/    fprintf(stderr,"存在しないオプションです。'-%c'",c);
             exit(1);
           }
#else
           if(Alpf){
             fprintf(stderr,"Invalid option '-%c'",argv[i][tugia+2]);
             exit(1);
           }
           else{
/*KANJI*/    fprintf(stderr,"存在しないオプションです。'-%c'",argv[i][tugia+2]);
             exit(1);
           }
#endif
      }
#ifndef GETOPT
       if(lof){
         lof = 0;
         break;
       }
       else
       if(iseigo(argv[1][tugia+2]))
         break;
       else
         ++tugia;
     }
    }
    else{
      if(Alpf){
        eprintf2("Invalid option '%.5s'",&argv[i][0]);
      }
      else{
/*KANJI*/        eprintf2("存在しないオプションです。'%.5s'",&argv[i][0]);
      }
    }
#endif
  }
}

operr1()
{
  if(Alpf)
   eprintf("-b to -s wa douji ni tukaemasen(nihongo)");
  else{
/*KANJI*/   eprintf("-b と -s は 併用できません。");
  }
}

operr2()
{
  if(Alpf)
   eprintf("-e option error");
  else{
/*KANJI*/   eprintf("-e オプションエラー");
  }
}







swpr()
{
  switch(sm[1]){
    case 0:
    case 1:  printf("BC"); break;
    case 2:
    case 3:  printf("DE"); break;
    case 4:
    case 5:  printf("HL"); break;
    default: printf("SP");
  }
}

/* dd2 */




birese()
{
    printf("\t%c,",sm[1]+'0');
    swsm(sm[2]);
}

dxxx()
{
  dbor();
  printf("0%XH,0%XH",yb,yc2);
  swf=2;
}


zed()
{
  yb=mem[bc];
  endcheck(4);
  sprintf(sm,"%03o",yb);
  smch();
  if(sm[0]==1){
    if(sm[2]==0){  
      if(sm[1]==6){
        eddbk();
        printf("\n\t;*IN\tF,(C)\t"); /* MITEIGI 2/4 */ 
      }
      else{
        printf("IN	");
        swsm(sm[1]);
        printf(",(C)");
      }
    }
    else
    if(sm[2]==1){
      if(sm[1]==6) eddbk();
      else{
        printf("OUT	(C),");
        swsm(sm[1]);
      }
    }
    else
    if(sm[2]==2){
      !(sm[1]%2) ? printf("SBC	HL,") : printf("ADC	HL,");
      swpr();
    }
    else
    if(sm[2]==7){
      switch(sm[1]){
        case 5: printf("RLD\t"); break;
        case 4: printf("RRD\t"); break;
        case 2: printf("LD	A,I"); break;
        case 3: printf("LD	A,R"); break;
        case 0: printf("LD	I,A"); break;
        case 1: printf("LD	R,A"); break;
        default:  eddbk();  
      }
    }
    else
    if(sm[2]==5){
      switch(sm[1]){
        case 1: printf("RETI\t"); break;
        case 0: printf("RETN\t"); break;
        default:  eddbk();
      }
    }
    else
    if(sm[2]==3){
      swf=2;
      if(sm[1]<4 || sm[1]>5){
        matometa();
        printf("LD	");
      }
      else eddbk();
      switch(sm[1]){
  
        case 1: printf("BC,");
                hifuro_2();
                break;
        case 3: printf("DE,");
                hifuro_2();
                break;
        case 7: printf("SP,");
                hifuro_2();
                break;
        case 0: hifuro_2();
                printf(",BC");
                break;
        case 2: hifuro_2();
                printf(",DE");
                break;
        case 6: hifuro_2();
                printf(",SP");
      }
    }
                  

    else
    if(sm[2]==6){
      switch(sm[1]){
        case 0: printf("IM	0"); break;
        case 2: printf("IM	1"); break;
        case 3: printf("IM	2"); break;
        default:  eddbk();
      }
    }
    else
    if(yb==0x44) printf("NEG\t");
    else eddbk();
  }
  else
  if(sm[0]==2){
    if(sm[2]==0){
      switch(sm[1]){
        case 5: printf("LDD\t");  break;
        case 7: printf("LDDR\t"); break;
        case 4: printf("LDI\t");  break;
        case 6: printf("LDIR\t"); break;
        default:  eddbk();
      }
    }
    else
    if(sm[2]==1){
      switch(sm[1]){
        case 5: printf("CPD\t");  break;
        case 7: printf("CPDR\t");  break;
        case 4: printf("CPI\t");  break;
        case 6: printf("CPIR\t"); break;
        default:  eddbk();
      }
    }
    else
    if(sm[2]==2){
      switch(sm[1]){
        case 5: printf("IND\t");  break;
        case 7: printf("INDR\t"); break;
        case 4: printf("INI\t");  break;
        case 6: printf("INIR\t"); break;
        default:  eddbk();
      }
    }
    else
    if(sm[2]==3){
      switch(sm[1]){
        case 5: printf("OUTD\t"); break;
        case 7: printf("OTDR\t"); break;
        case 4: printf("OUTI\t"); break;
        case 6: printf("OTIR\t"); break;
        default:  eddbk();
      }
    }
    else eddbk();	/* 93/1/22 */

  }
  else eddbk();
}

void addddky() /* 2/5 */
{
      switch(sm[1]){
        case 0: printf("ADD	A,"); break;
        case 1: printf("ADC	A,"); break;
        case 2: printf("SUB	");   break;
        case 3: printf("SBC	A,"); break;
        case 4: printf("AND	");   break;
        case 5: printf("XOR	");   break;
        case 6: printf("OR	");   break;
        case 7: printf("CP	");
      }
}

owari(hikisu)
short hikisu;
{
  unsigned sailo;
  if(hikisu)
    dbor();

  sailo = orgs+bc2;
  switch(hikisu){
    case 1: printf("0%XH\t\t;%X",yb,sailo);
            break;
    case 2: printf("0%XH,0%XH\t",yb,yc2);
            if(yb < 0x10 && yc2 < 0x10){
              printf("\t");
            }
            printf(";%X",sailo);
            break;
    case 3: printf("0CBH\t\t;%X",sailo);
            break;
    case 4: printf("0EBH\t\t;%X",sailo);
            break;
    case 5: printf("0EBH,0%XH\t;%X",yb,sailo);
            break;
    case 6: printf("0EBH,0%XH,0%XH\t;%X",yb,yc2,sailo);
            break;
    case 7: printf("0%XH,0%XH,0%XH\t;%X",yb,yc2,yc,sailo);
  }
  if(dataf && hikisu > 0 && hikisu < 8){
    dataput();
    printf("\n");
  }

  printf("\tEND\n");
  free(labt);
  free(labm);
  free(labc);
  free(labc2);
#ifdef MSDOSREAL
  hfree(locc);
  hfree(mem);
#else
  free(locc);
  free(mem);
#endif
  if(Hexmode)
    free(kawari);
  exit(0);
}

/* hexfile segment owari */
hexsegow(hikisu)
short hikisu;
{
  unsigned sailo;
  if(hikisu)
    dbor();

  sailo = orgs+bc2;
  switch(hikisu){
    case 1: printf("0%XH\t\t;%X",yb,sailo);
            break;
    case 2: printf("0%XH,0%XH\t",yb,yc2);
            if(yb < 0x10 && yc2 < 0x10){
              printf("\t");
            }
            printf(";%X",sailo);
            break;
    case 3: printf("0CBH\t\t;%X",sailo);
            break;
    case 4: printf("0EBH\t\t;%X",sailo);
            break;
    case 5: printf("0EBH,0%XH\t;%X",yb,sailo);
            break;
    case 6: printf("0EBH,0%XH,0%XH\t;%X",yb,yc2,sailo);
            break;
    case 7: printf("0%XH,0%XH,0%XH\t;%X",yb,yc2,yc,sailo);
  }
  if(dataf && hikisu > 0 && hikisu < 8){
    dataput();
    printf("\n");
  }

  printf("\n\tORG\t0%XH\n\n",bc);
}

/* MITEIGI INC IXH 2/5 */
void miteinde(anpon,tansuke)
char *anpon; /* IN(C) or DE(C) */
char tansuke; /* H or L */
{
  dxxx();
  printf("\n\t;*%sC\tI%c%c\t",anpon,cxi,tansuke);
}

zddfd()
{
  char chl;

  cxi=(sm[1]==3) ? 'X':'Y';
  yc2=mem[bc];
  endcheck(1);
  sprintf(sm,"%03o",yc2);
  smch();
  if(sm[0]==2){
    if(sm[2]==6){

      yc=(unsigned short)mem[bc];
      endcheck(2);
      addddky();
      fugou();
      aokage();
      if(sm[1]<4) swf=2;
      else
      if(yc>9) swf=2;
    }
    else{ /* MITEIGI  ADC A,IXH nado 2/5 */
      if(sm[2]==4 || sm[2]==5){
        dxxx();
        printf("\n\t;*");
        addddky();
        printf("I%c%c\t",cxi,(sm[2]==4) ? 'H':'L'); 
      }
      else
      dxxx();
    }
  }
  else
  if(sm[0]==1){
    if(yc2==0x76) {                  /* \o166 */
      dxxx();
    }
    else
    if(sm[2]==6){
        yc=(unsigned short)mem[bc];
        endcheck(2);
        printf("LD	");
        swsm(sm[1]);
        fugou();
        putchar(',');
        aokage();
        swf=2;
    }
    else
    if(sm[1]==6){
      yc=(unsigned short)mem[bc];
      endcheck(2);
      fugou();
      printf("LD	");
      aokage();
      putchar(',');		/* 93/1/22 BUG FIX*/
      swsm(sm[2]);
      swf=2;
    }
    else{
      dxxx();
      if(sm[2]!=6){
        if(sm[1]==4 || sm[1]==5){
          chl=(sm[1]==4) ? 'H' : 'L';
          printf("\n\t;*LD\tI%c%c,",cxi,chl);
          mitld();
        }
        else
        if(sm[1]!=6){
          if(sm[2]==4 || sm[2]==5){
            chl=(sm[2]==4) ? 'H' : 'L';
            printf("\n\t;*LD\t");
            swsm(sm[1]);
            printf(",I%c%c\t",cxi,chl);
          }
        }
      }
    }
  }
  else
  if(sm[0]==0){
    if(sm[2]==1){
      switch(sm[1]){
        case 4: mamama();
                if(bc2==labm[tel] && tel < lc ){
                  printf("LD\tI%c,Z%04d",cxi,labc[tel++]);
                }
                else{
                  printf("LD\tI%c,0%X%02XH",cxi,yc3,yc);
                }
                swf=2;
                break;
        case 1: printf("ADD\tI%c,BC",cxi); break;
        case 3: printf("ADD\tI%c,DE",cxi); break;
        case 5: printf("ADD\tI%c,I%c",cxi,cxi); break;
        case 7: printf("ADD\tI%c,SP",cxi); break;
        default: dxxx();
      }
    }                
    else
    switch(yc2){
      case 0x36:    /* \o066 */
        mamama();
        fugou();
        printf("LD	");
        aokage();
        printf(",0%XH",yc3);
        swf=2;
        break;
      case 0x2b: printf("DEC	I%c",cxi); break;  /* \o053 */
      case 0x23: printf("INC	I%c",cxi); break;  /* \o043 */
      case 0x34:   /* \o064 */
        yc=(unsigned short)mem[bc];
        endcheck(2);
        fugou();
        printf("INC	");
        aokage();
        swf=2;
        break;
      case 0x2a:  /* \o052 */
        mamama();
        if(bc2==labm[tel] && tel < lc ){
          printf("LD\tI%c,(Z%04d)",cxi,labc[tel++]);
        }
        else{
          printf("LD\tI%c,(0%X%02XH)",cxi,yc3,yc);
        }
        swf=2;
        break;
      case 0x22:  /* \o042 */
        mamama();
        if(bc2==labm[tel] && tel < lc ){
          printf("LD\t(Z%04d),I%c",labc[tel++],cxi);
        }
        else{
          printf("LD\t(0%X%02XH),I%c",yc3,yc,cxi);
        }
        swf=2;
        break;
      case 0x35:  /* \o065 */
        yc=(unsigned short)mem[bc];
        endcheck(2);
        fugou();
        printf("DEC	");
        aokage();
        swf=2;
        break;

/* 2/4 MITEIGI LD IXH,n */
      case 0x26:
      case 0x2e:
        yc=(unsigned short)mem[bc];
        endcheck(2);
        chl=(yc2==0x26) ? 'H':'L';
        dbor();
        printf("0%XH,0%XH,0%xH",yb,yc2,yc);
        printf("\n\t;*LD\tI%c%c,0%2xH",cxi,chl,yc);
        swf=2; /* ? */
        break;

/* 2/5 MITEIGI INC IXH */
      case 0x24:
        miteinde("IN",'H');
        break;
      case 0x2c:
        miteinde("IN",'L');
        break;
      case 0x25:
        miteinde("DE",'H');
        break;
      case 0x2d:
        miteinde("DE",'L');
        break;
      default: dxxx();
    }
  }
  else{
  switch(yc2){
      case 0xcb:  /* \o313 */
        ddfdcb();
        break;
      case 0xe5:  /* \o345 */
        printf("PUSH	");
        icple();
        break;
      case 0xe1:  /* \o341 */
        printf("POP	");
        icple();
        break;
      case 0xf9:  /* \o371 */
        printf("LD	SP,");
        icple();
        break;
      case 0xe3:  /* \o343 */
                  printf("EX	(SP),");
        icple();
        break;
      case 0xe9:  /* \o351 */
                  printf("JP	(I%c)",cxi);
        break;
  
      case 0xcd:  /* \o315 SLL (IY+dd) 2/4 */
        if(yb==0xfd) slliy();
        else
        dxxx();
        break;
      default:
        dxxx();
    }
  }
}

sllkyo()
{
    dbor(); 
    printf("0%XH,0%XH,",yb,yc2);
    printf("0%XH,0%XH\n\t",*(data+2),yc3);
    printf(";*SLL\t");
    aokage();
    if(yc>9)swf=2;
}
/* MITEIGI SLL (IY+dd) 2/4 */
slliy()
{
  mamama();

  if(yc3==0x36){
    fugou();
    sllkyo();
  }
  else
  dbfcbk();
}

icple()
{
  printf("I%c",cxi);
}

dbfcbk()
{
  dbor(); 
  printf("0%XH,0%XH\t;%X\n\t",yb,yc2,bc2);	/* 93/1/22 BUG FIX */
  bc2+=2;
  dbor();
  printf("0%XH,0%XH",yc,yc3);
}

ddfdcb()
{
  mamama();
  sprintf(sm,"%03o",yc3);/* octm(sm,yc3); */
  smch();
  if(sm[2]!=6)
    dbfcbk();
  else
  {
    fugou();
    if(sm[0]==0){
      if(sm[1]==6){ /* 2/4 MITEIGI SLL IX+dd */
        if(yb==0xdd){
          sllkyo();
        }
        else
        dbfcbk();
      }
      else{
        switch(sm[1]){
          case 0: printf("RLC"); break;
          case 1: printf("RRC"); break;
          case 2: printf("RL");  break;
          case 3: printf("RR");  break;
          case 4: printf("SLA"); break;
          case 5: printf("SRA"); break;
          case 7: printf("SRL");
        }
        putchar('\t');
        aokage();

        if(yc>9)swf=2;
      }
    }
    else
    switch(sm[0]){
      case 1:
        printf("BIT");
        iteset();
        break;
      case 2:
        printf("RES");
        iteset();
        break;
      case 3:
        printf("SET");
        iteset();
        break;
      default:
        dbfcbk();
    }    
  }
}

iteset()
{
  printf("\t%c,",sm[1]+'0');
  aokage();
  swf=2;
}



swsm(hiks)
char hiks;
{
  switch(hiks){
    case 7: putchar('A'); break;
    case 0: putchar('B'); break;
    case 1: putchar('C'); break;
    case 2: putchar('D'); break;
    case 3: putchar('E'); break;
    case 4: putchar('H'); break;
    case 5: putchar('L'); break;
    case 6: printf("(HL)");
  }
}

swjk()
{
  switch(sm[1]){
    case 0: printf("NZ"); break;
    case 1: putchar('Z');   break;
    case 2: printf("NC"); break;
    case 3: putchar('C');   break;
    case 4: printf("PO"); break;
    case 5: printf("PE"); break;
    case 6: putchar('P');   break;
    case 7: putchar('M');
  }
}

endcheck(hikisu)
int  hikisu;
{
  int ab;
  if(bc>=fsize){
    owari(hikisu);
  }
  ab = 0;
  if(Hexmode){
    ab = kaow();
    if(ab == -1)
      owari(hikisu);
    else
    if(ab == 1)
      hexsegow(hikisu);
  }
  data[dak]=mem[bc];
  ++dak;
  ++bc;
}

matometa()
{
  yc2=mem[bc];
  endcheck(5);
  yc=(unsigned short)mem[bc];
  endcheck(6);
}

matome1()
{
  yc2=mem[bc];
  endcheck(1);
  yc=(unsigned short)mem[bc];
  endcheck(2);
}
lmato()
{
  yc2=mem[bc];
  lendc();
  yc=(unsigned short)mem[bc];
  lendc();
}

latan(unsigned byc,unsigned tmae)
{
  int mktf;
  int lni;
  unsigned tobc;
  if(tansaku(byc,locc,i2i)!=ERROR){
    lni=0;
    mktf = 0;
    while(lni<lc && lni<lasize){
      if(labt[lni]==byc){
        tobc=labc[lni];
        mktf = 1;
        break;
      }
      ++lni;
    }

    if(!mktf){
      labc2[lc]=labc[lc]=tmax++;
    }
    else
      labc2[lc]=labc[lc]=tobc;
    labt[lc]=byc;   
    labm[lc]=tmae; /* tobu mae */
    ++lc;
  }
}

void jptes(unsigned ltbc,unsigned ms)
{
  int jpmf;
  int mktf;
  unsigned byc;
  unsigned tk;
  int zkak;
  long sege;

  sprintf(sm,"%03o",ms);  /* 8sinsuu ni henkan site sm[] ni ireru */
  smch();
  sege = Hexmode ? segend():fsize;
    
  jpmf = zkak = 0;
  if(sm[0]==3){
    if(ms==0xc3 || ms==0xcd || sm[2]==4 || sm[2]==2){
      /* if(JP || CALL) */
      jpmf = 1;
    }
    else
    if(!intel && ltbc+3<sege){
      if(ms==0xed){
        ++ltbc;
        tk = mem[ltbc];
        if(tk==0x4b || tk==0x5b || tk==0x7b ||
          tk==0x43 || tk==0x73 || tk==0x53){
          /* LD BC,(nn)  LD DE,(nn)  LD SP,(nn)
             LD (nn),BC  LD (nn),SP  LD(nn),DE */
          jpmf = zkak = 1;
        }
      }
      else
      if(ms==0xdd || ms==0xfd){
        ++ltbc;
        tk = mem[ltbc];
        if(tk==0x2a || tk==0x22 || tk==0x21){
          /* LD IX,(nn)  LD (nn),IX  LD IX,nn */
          /* LD IY,(nn)  LD (nn),IY  LD IY,nn */
          jpmf = zkak = 1;
        }
      }
    }
  }
  else{
    if(!sm[0]){
      switch(ms){
        case 0x3a:	/* LD A,(nn) */
        case 0x32:	/* LD (nn),A */
        case 0x21:	/* LD HL,nn */
        case 0x01:	/* LD BC,nn */
        case 0x11:	/* LD DE,nn */
        case 0x31:	/* LD SP,nn */
        case 0x2a:	/* LD HL,(nn) */
        case 0x22:	/* LD (nn),HL */
           jpmf = 1;
      }
    }
  }
  if(jpmf){
      ltbc+=2;
      if(ltbc>=sege)
        return;
      byc=r16((unsigned)mem[ltbc],(unsigned)mem[ltbc-1]);
      byc-=orgs;
      latan(byc,ltbc-2-zkak);
  }
}
/*******************************************/
/* PASS 2 JP,JR,LD jump,load address check */
/*******************************************/
lavtes()
{
  /* jump meirei code no tobisaki ga location counter ka douka sirabe  */
  /* soude areba hairetu ni ireru */

  unsigned ms;
  unsigned ltbc;
  long sege;
  int i;

  sege = Hexmode ? segend():fsize;

  for(i=0;i<i2i;++i){
    ltbc = locc[i];
    ms=mem[ltbc];
    if(!intel){
      switch(ms){
        case 0x18:	/* JR */
        case 0x20:	/* JR NZ, */
        case 0x28:	/* JR Z, */
        case 0x30:	/* JR NC, */
        case 0x38:	/* JR C, */
        case 0x10:	/* DJNZ */
          ++ltbc;
          if(ltbc>=sege)
            break;
          yc=(unsigned short)mem[ltbc];
          if(yc>127)
            yc-=(256-1);
          else 
            ++yc;
          latan(ltbc+yc,ltbc-1);

          break;
        default:
          jptes(ltbc,ms);
      }
    }
    else jptes(ltbc,ms);
    if(lc>=lasize)
      break;
  }
}

locpr()
{
/*
    if(Symmode){
      if(symcheck()){
        Sycc[Syi++]bc;
      }
      else
        locc[i2i++]=bc;
    }
    else
*/
      locc[i2i++]=bc;
}


/*********************************/
/* PASS 1 location counter check */
/*********************************/
lmroop()
{
  /* location counter check */

  long sege;
  int tppp;
  int jidesu;

  sege = Hexmode ? segend():fsize;
  i2i=0;
  /* Sycc = 0; */
    
  Katct = Katsk;

  c3f = 0;
  while(1){
    kt = kanc = 0;
    locpr();

    yb=mem[bc];
    lendc();

    if(!notef){
      if(yb==0xc9 /* RET */ || c3f /* JP */){
        if(!c3f){
          locpr();
          yb=mem[bc];
          lendc();
        }
          --bc;
        tppp = moji_check(&jidesu);
          ++bc;
        if(tppp>8 && tppp+bc-1<=sege && jidesu){
/*        if(mem[bc-1]==0xc9) locc[i2i++]=bc;*/
        bc+=(tppp);
        --bc;
        locpr();
        yb=mem[bc];
        lendc();
      }
    }
   }
   c3f=0;
   if(yb==0xc3)
     if(bc+orgs+8<r16((unsigned)mem[bc+1],(unsigned)mem[bc])) 
       c3f=1;

    sprintf(sm,"%03o",yb);  /* 8sinsuu ni henkan site sm[] ni ireru */
    smch();


    if(sm[0]==3){
      if(sm[2]==6 || yb==0xdb || yb==0xd3 || yb==0xcb)
        lendc();
      else
      if(sm[2]==4 || sm[2]==2 || yb==0xcd || yb==0xc3){
        lendc();
        lendc();
      }
      else
      switch(yb){
        case 0xed: if(!intel) lzed();
                   break;
        case 0xfd:
        case 0xdd: if(!intel) lzddfd();
      }
    }
    else
    if(!sm[0]){
      if( (!sm[2] && sm[1]>1 && !intel) || sm[2]==6) 
        lendc();
      else
      switch(yb){
        case 0x3a:
        case 0x32:
        case 0x01:
        case 0x11:
        case 0x21:
        case 0x2a:
        case 0x31:
        case 0x22: lendc();
        	   lendc();
      }
    }
  }
}    

lzed()
{
  yb=mem[bc];
  lendc();
  sprintf(sm,"%03o",yb);
  smch();
  if(sm[0]==1 && sm[2]==3 && sm[1]!=4 && sm[1]!=5)
    lmato();
}

lzddfd()
{
  yc2=mem[bc];
  lendc();
  sprintf(sm,"%03o",yc2);
  smch();
  if(sm[0]==2 && sm[2]==6){
      lendc();
  }
  else
  if(sm[0]==1){
    if(yc2!=0x76)                  /* \o166 */
    if(sm[2]==6 ||sm[1]==6){
      lendc();
    }
  }
  else
  if(sm[0]==0){
    if(sm[2]==1){
      switch(sm[1]){
        case 4:
                lendc();
                lendc(); 
                break;
      }
    }                
    else
    switch(yc2){
      case 0x36:    /* \o066 */
      case 0x2a:
      case 0x22:
        lendc();
        lendc();
        break;
      case 0x2b:
      case 0x23:
      case 0x34:
      case 0x35:
      case 0x26:	/* ;*LD	IXH,n *LD IYH,n 1996-8-29 */
      case 0x2e:	/* ;*LD	IXL,n *LD IYL,n 1996-8-29 */
        lendc();
        break;
    }
  }
  else{
    if(yc2==0xcb){
      lendc();lendc();
    }
    else
    if(yc2==0xcd && yb ==0xfd){	/* \o315 SLL (IY+dd) 1996-8-29 */
      lendc();lendc();
    }
  }
}


lendc()
{
  int ab;
  if(bc>=fsize || i2i>LOSI){
    /* lomax=locc[i2i-1]; */
    --i2i;
    mroop();
  }
  if(Hexmode){
    ab = kaow();
    if(ab == -1){
      --i2i;
      mroop();
    }
    if(ab!=1)
      ++bc;
  }
  else
    ++bc;
}

eddbk()
{
  dbor();
  printf("0EDH,0%XH",yb);
  swf=2;
}

mamama()
{
  yc=(unsigned short)mem[bc];
  endcheck(2);
  yc3=mem[bc];
  endcheck(7);
}



mitld()
{
  if(sm[2]!=4 && sm[2]!=5)
    swsm(sm[2]);
  else
  switch(sm[2]){
    case 4: printf("I%cH",cxi); break;
    case 5: printf("I%cL",cxi);
  }
  putchar('\t');
}

allocerr()
{
  if(Alpf)
    eprintf("cannot alloc memory. -l,-s,-S,-e option wo tamesite.");
  else{
/*KANJI*/    eprintf("メモリ領域確保できない。-l,-s,-S,-eオプションを試して下さい");
  }
}

#ifdef MSDOSSS
hojarchk(char *s)
{
  int len;

  len=strlen(s);
  if(!strcmp("HOJAR.EXE",s+(len-9))){
    Alpf = 1;	/* roma ji mode */
  }
}
#endif

int main(argc,argv)
short argc;
char **argv;
{
  long tbc;
#ifdef MSDOSREAL
  unsigned lasi2 = 0;
#else
  long lasi2 = (long)0;
#endif
  unsigned bstart; /* MSX-BASIC bload FILE run start banci*/
  unsigned bsaigo; /* MSX-BASIC bload FILE saigo no banci */
  int q,kct;
  long mfsize;	/* hontou no file size */
  unsigned tppp;
  char *symname;

  tppp = 0;
  tbc = (long)0;
  q = 0;
  Katsk = 0;	/* Katct syokichi */
  Hexmode = /* Symmode = */ 0;

#ifdef MSDOSSS
  hojarchk(argv[0]);
#else
  if(!strcmp("hojar",argv[0])){ /* romaji mode */
    Alpf = 1;	/* roma ji mode */
  }
#endif
  if(argc<2)
    setumei();
  if(argv[argc-1][0]=='-'){
    if(argv[argc-1][1]=='A' || argv[argc-1][1]=='N'){
      if(argv[argc-1][1]=='A'){
        Alpf = 1;
      }
      else{
        Alpf = 0;
      }
    }
    setumei();
  }
  
  orgs=0x100;	/* default org banci 0x100 */
  if(argc>1)
    option(argc,argv,&symname);

  if(Hexmode){
    if(hex(argv[argc-1]))
      exit(1);
    orgs=0;
  }
/*
  if(Symmode){
    if(-1 == symbol(symname)){
      exit(1);
    }
  }
*/
#ifdef MSDOSREAL
  if((long)LOSI*(long)(sizeof(short))+(long)2 > (long)65535){
    LOSI = 32765;
  }
#endif
  lasize= LOSI / 2;	/* saidai label kazu kari kettei */
  
  if(!Hexmode){
    fdo(argv[argc-1]);
    mfsize = fsize = lseek(fd,(long)0,2);	/* file size */

    if(Saisf){
      /* file yomikomi kaisi sitei ari */
      if(fsize <= Saisyo){
        if(Alpf)
          eprintf("-S no sitei ga fairu saizu yori ookii.(nihongo)");
        else{
/*KANJI*/          eprintf("-S の指定がファイルサイズより大きい");
        }
      }
      fsize -= Saisyo;
    }
  
  
    if(startflag){
      if(start<orgs){
        if(Alpf)
          eprintf("-s no sitei ga okashii. ORG yori chiisai(nihongo)");
        else{
/*KANJI*/          eprintf("-s の指定がおかしい。(ORG値より小さい)");
        }
      }
      tppp=start-orgs;
      fsize-=(long)tppp;
      orgs=start;
    }
    if((long)tppp+Saisyo >= mfsize){
      if(Alpf)
        eprintf("file read start address > file size");
      else{
/*KANJI*/        eprintf("ファイル読み込み開始位置がファイルサイズを越えています。");
      }
    }
    if(lseek(fd,(long)tppp+Saisyo,0)== (long)-1){
      if(Alpf)
        eprintf("disk seek error");
      else{
/*KANJI*/        eprintf("ディスクシークエラー");
      }
    }
    
    if(1>read(fd,buf,128)){
      if(Alpf)
        eprintf("file read error");
      else{
/*KANJI*/        eprintf("ファイル読み込みエラー");
      }
    }
    if(!tppp && !Saisyo){
      if(ihexck(buf)){
        printf("\n\t;Warning: intel HEX file?\n");
        printf("\t;HEX file wa \"-H\" option ga hituyou\n");
      }
    }
    if(bload && (buf[0]!=0xfe || fsize<8)){
      if(Alpf)
        eprintf("Not MSX bload file. -b option.");
      else{
/*KANJI*/        eprintf("MSXのbloadファイルでは無いのに -b オプションが指定されている");
      }
    }
  
    if(bload){
      orgs=r16((unsigned)buf[2],(unsigned)buf[1]);
      bsaigo=r16((unsigned)buf[4],(unsigned)buf[3]);
      bstart=r16((unsigned)buf[6],(unsigned)buf[5]);
      fsize-=7;
      q=7; 
    }
  }

  if(adenflag){
    if(Hexmode){
      if(Hexorg>endad){
        if(Alpf)
          eprintf("-e no sitei ga hex file ORG yori chiisai.");
        else{
/*KANJI*/          eprintf("-e の指定値がヘキサファイルORGより小さい。");
        }
      }
      if(fsize >endad+1){
      	if(!codehani((unsigned short)endad)){
          /* saisyuu banchi wa hex file ni nai */
          kt_size((unsigned short)(endad));
        }
        else{
          fsize = endad+1;
        }
      }
    }
    else
    if(orgflag || startflag || bload){
      if(orgs>endad || fsize+orgs<endad){
        operr2();
      }
      fsize=endad-orgs+1;
    }
    else
    if(endad< 0x100 || fsize+orgs<endad){
        operr2();
    }
    else
      fsize=endad- 0x100 +1;
  }
#ifdef MSDOSREAL
  if(fsize > (long)(65535-READSIZE-130)) bigerr();
#else
  if(fsize > (long)65535) bigerr();
#endif
  if(startflag && Hexmode){
    if(start >= fsize){
        if(Alpf)
          eprintf("-s no sitei ga hex file ORG yori chiisai.");
        else{
/*KANJI*/          eprintf("-s の開始番地指定値が終了番地より大きい。");
        }
    } 
    if(start > Hexorg){
      	kct = codehani((unsigned short)start);
        if(!kct){
          /* saisyuu banchi wa hex file ni nai */
          kt_st((unsigned short)start);
      	  kct = codehani((unsigned short)Hexorg);
          start = Hexorg;
        }
        else{
          Hexorg = start;
        }
        Katsk = kct -1;
    }
  }

  if(fsize < LOSI){
    LOSI = (unsigned)fsize;
    lasize = LOSI / (unsigned)2;
  }
#ifdef MSDOSREAL
  if(!Hexmode){
    if((mem=(unsigned char huge *)halloc((long)(fsize+(READSIZE*2)),1))==NULL){
      allocerr();
    }
  }
  lasi2=(lasize*(sizeof(short)))+sizeof(short);
#else
  if(!Hexmode){
    if((mem=(unsigned char *)malloc(fsize+(READSIZE*2)))==NULL){
      allocerr();
    }
  }
  lasi2=(long)(lasize*(sizeof(short)))+sizeof(short);
#endif

  if((labt=(unsigned short *)malloc(lasi2))==NULL ||
    (labm=(unsigned short *)malloc(lasi2))==NULL || 
    (labc=(unsigned short *)malloc(lasi2))==NULL ||
    (labc2=(unsigned short *)malloc(lasi2))==NULL ||
#ifdef MSDOSREAL
    (locc=(unsigned short huge *)halloc(LOSI*(sizeof(short))+2,1))==NULL){
#else
    (locc=(unsigned short *)malloc(LOSI*(sizeof(short))+2))==NULL){
#endif
    free(labt);
    free(labm);
    free(labc);
    free(labc2);
#ifdef MSDOSREAL
    hfree(locc);
    hfree(mem);
#else
    free(locc);
    free(mem);
#endif
    if(Hexmode)
      free(kawari);
    allocerr();
  }

  if(!Hexmode){
    memcpy(mem,&buf[q],128-q);
    tbc=128-q;
    while(1){
      if(tbc>=fsize)
        break;
      yomi(&tbc);
    }
    close(fd);
  }

  if(Alpf)
    fprintf(stderr,"\tZ80 dis assembler hoja  ver");
  else{
/*KANJI*/    fprintf(stderr,"\tZ80逆アセンブラ hoja  ver");
  }
  fprintf(stderr,VERSION);
  fprintf(stderr,"   for %s",OS);
  fprintf(stderr,"\n\t\t\t(C) Ooba Masaki  ");
  fprintf(stderr,NENGOU);
  fprintf(stderr,"\n");
  printf("\n\n");	

  if(m80)
    printf("\t.Z80\n\tASEG\n");
  if(bload){
    printf("\tORG	0%XH\n\t",orgs-7);
    dbor(); 
    printf("0FEH		;bload file sentou hedda\n\t");
    dwor();
    printf("0%xH		;load sentou banci\n\t",orgs);
    dwor();
    printf("0%xH		;load syuturyoku banci\n\t",bsaigo);
    dwor();
    printf("0%xH		;run banchi\n",bstart);
  }
  else{
    if(Hexmode){
      printf("\tORG\t0%XH\n",Hexorg);
    }
    else
    if(orgflag || startflag)
      printf("\tORG	0%XH\n",orgs); 
    else{
      printf("\tORG	100H\n");
    }
  }

  if(Hexmode)
    bc = Hexorg;
  else
    bc=0;

  !lf ? lmroop():mroop();
  free(labt);
  free(labm);
  free(labc);
  free(labc2);
#ifdef MSDOSREAL
  hfree(locc);
  hfree(mem);
#else
  free(locc);
  free(mem);
#endif
  if(Hexmode)
    free(kawari);
  return 0;
}

