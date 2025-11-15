
/*   ihex.c */
/* Z80 gyaku asenbura hoja 2.01  UNIX gokan OS you */
/* Z80 dis assembler hoja 2.01 for UNIX compatible OS */
/* (C) Ooba Masaki 1998-4-27 */
/* compile
   UNIX gcc
      gcc -o hoja hoja.c ihex.c

   MS-DOS DJGPP(MS-DOS gcc)
      gcc -DGO32 hoja.c ihex.c

   MS-DOS Microsoft-C ver6.0
      cl -AC -DMSDOSREAL hoja.c ihex.c /link /ST:50000
*/

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE
#ifdef MSDOSREAL
#define MSDOSSS
#define OS "Windows 32bit"
#define huge
#define halloc malloc
#define hfree free
#endif

#include <stdio.h>
#include <malloc.h>

#define HEXLINESIZE 1000


#ifdef MSDOSREAL
extern unsigned char huge *mem;
#else
extern unsigned char *mem;
#endif

unsigned short *kawari;
int Kat;	/* kawari count */
unsigned Hexorg;	/* hex file org */
extern unsigned bc;
extern unsigned Katct;	/* Kat count */
extern long fsize;	/* file size */
extern int Alpf;	/* 0:kanji mode  1:roma ji mode */

#ifdef TANTAITEST
int H4err;
#else
int H4err;
#endif

/* kani hexfile check zenbu wa sirabenai */
/* return 1; tabun hex file */
/* return 0; tabun binary file */
int ihexck(bbbb)
char *bbbb;
{
  int i;
  if(bbbb[0]!=':')
    return 0;
  for(i=1;i<11;++i){
    if(ishex(bbbb[i]))
      return 0;
  }
  return 1;
}

/* hex segment sort */
bsort()
{
  int i,j;
  unsigned short tmp;
  unsigned short *tti,*ttj;
  for(j=0;j<Kat-1;++j){
    for(i=j+1;i<Kat;++i){
      tti = kawari+(i*2);
      ttj = kawari+(j*2);
      if(*ttj > *tti){
        tmp = *tti;
        *tti = *ttj;
        *ttj = tmp;
        ++tti;
        ++ttj;
        tmp = *tti;
        *tti = *ttj;
        *ttj = tmp;
      }
    }
  }
}

/* return hexfile code current segment end+1 */
unsigned long segend()
{
  return (unsigned long) *(kawari+(Katct*2)+1)+1;
}


#ifdef TANTAITEST
testkt()
{
  int i;
  for(i=0;i<Kat;++i){
  printf("st[%d]=%x   en[%d]=%x \n",i,*(kawari+(i*2)),i,
   *(kawari+(i*2)+1));
  }
}
#else
/* hexfile code segment end check */
int kaow()
{
  if(bc > *(kawari+(Katct*2)+1)){
    ++Katct;
    if(Katct >=Kat){
      return -1;	/* owari file end */
    }
    bc = (unsigned)*(kawari+(Katct*2));
    return 1;
  }
  return 0;
}


/* hexfile code hani check */
int codehani(adb)
unsigned short adb;
{
  int i;
  unsigned short as;
  unsigned short ae;
  for(i=0;i<Kat;++i){
    as = *(kawari+(i*2)); 
    ae = *(kawari+(i*2)+1);
    if(as <= adb && ae >=adb){
      return i+1;
    }
  }
  return 0;	/* code no hani dewa nai*/
}

/* hexfile code hani end syuusei */
kt_size(adb)
unsigned short adb;
{
  int i;
  unsigned short as;
  unsigned short mad;
  if(Kat<2){
    return;
  }
  mad = *(kawari+1);
  for(i=0;i<Kat;++i){
    as = *(kawari+(i*2)); 
    if(as >adb){
      fsize = (unsigned long)mad+1;
    }
    mad = *(kawari+(i*2)+1);
  }
}

/* hexfile code hani start syuusei */
kt_st(adb)
unsigned short adb;
{
  int i;
  unsigned short as;
  unsigned short mad;
  if(Kat<2){
    return;
  }
  for(i=0;i<Kat;++i){
    as = *(kawari+(i*2)); 
    if(as >adb){
      Hexorg = as;
    }
  }
}

#endif

/* 16sinsuu moji? */
int ishex(abc)
char abc;
{
  if((abc >='0' && abc <='9') || (abc >= 'A' && abc <='F')) 
    return 0;
  else
    return 1;
}

int hex1toi(abc)
char abc;
{

  if(abc >='0' && abc <='9'){
    abc -= '0';
  }
  else
  if(abc >='A' && abc <='F'){
    abc -= ('A'-10);
  }
  else
    return -1;
  return (int)abc;
}

/* 4moji 16sinsuu -> int */
unsigned long hex4tou(abc)
char *abc;
{
  unsigned aaa1,aaa2,aaa3,aaa4;
  unsigned long gk;

  aaa1 = *abc;
  aaa2 = *(abc+1);
  aaa3 = *(abc+2);
  aaa4 = *(abc+3);
  if(ishex(aaa1) || ishex(aaa2) || ishex(aaa3) || ishex(aaa4)){
    H4err = -1;  
    return 0;
  }
  else
    H4err = 0;
  aaa1 = hex1toi(aaa1);
  aaa2 = hex1toi(aaa2);
  aaa3 = hex1toi(aaa3);
  aaa4 = hex1toi(aaa4);
  
  gk = (unsigned long) aaa4;
  gk += (unsigned long) ((aaa3)*16);
  gk += (unsigned long) ((aaa2)*256);
  gk += (unsigned long) ((aaa1)*4096);
  return gk;   
}


/* 2moji 16sinsuu -> int */
int hex2toi(abc)
char *abc;
{
  int aaa1,aaa2;

  aaa1 = *abc;
  aaa2 = *(abc+1);
  if(ishex(aaa1) || ishex(aaa2)){
    H4err = -1;
    return -1;
  }
  else
    H4err = 0;
  aaa1 = hex1toi(aaa1);
  aaa2 = hex1toi(aaa2);
  return (aaa1 * 16) + aaa2;   
}

/******************************/
/* check sum and  hex -> binary */
/* retun 0;  OK */
/* return -1; NG */
/******************************/
int sumcheck(line,rb,hexdata)
char *line;
int rb;
unsigned char *hexdata;
{
  unsigned sum;
  int a;
  unsigned char b;
  int i;
  sum = 0;
  for(i=0;i<rb+5;++i,line+=2){
    if(ishex(*line) || ishex(*(line+1)))
      return -1;
    a = hex2toi(line);
    sum += a;
    if(i>3 && i < rb+4){
      *hexdata = a;
      ++hexdata;
    }
  }
  b = sum;
  return (int)b;
}


hexallocerr()
{
    if(Alpf)
    	fprintf(stderr,"cannot alloc memory(hex file)\n");
    else{
/*KANJI*/      fprintf(stderr,"ヘキサファイル処理用のメモリが確保できません。\n");
    }
}

#ifdef TANTAITEST
int hex(argc,inpname)
int argc;
char *inpname;
#endif
int hex(inpname)
char *inpname;
{
  FILE *fp;
  int i;
  char *hexline;
  unsigned char *hexdata;
  int err;
  int rb;	/* record byte */
  unsigned long ad;	/* address */
  int rt;	/* record type */
  unsigned long segment;	/* hex file segment */
  unsigned long mad;	/* mae no segment+adress */
  

  err = 0;
  segment = (long)0;	/* hexfile segment */
  Kat = 0;
  mad = (long)0xffffe;
  Hexorg = 0;
  fsize = (long)0;

#ifdef MSDOSREAL
  if(NULL == (fp=fopen(inpname,"rt"))){
#else
#ifdef GO32
  if(NULL == (fp=fopen(inpname,"rt"))){
#else
  if(NULL == (fp=fopen(inpname,"r"))){
#endif
#endif

    if(Alpf){
      fprintf(stderr,"file cannot open[%s]\n",inpname);
    }
    else{
/*KANJI*/      fprintf(stderr,"ファイルが開けません。[%s]\n",inpname);
    }
    return -1;
  }
#ifdef MSDOSREAL
  if(NULL == (mem = halloc((long)65536,1))){
#else
  if(NULL == (mem = malloc(65536))){
#endif
    if(Alpf){
      fprintf(stderr,"cannot alloc memory(hex file 64Kbyte)\n");
    }
    else{
/*KANJI*/      fprintf(stderr,"ヘキサファイル処理用のメモリ64Kbyteが確保できません。\n");
    }
    fclose(fp);
    return -1;
  }
  if(NULL == (hexline = malloc(HEXLINESIZE))){
    hexallocerr();
#ifdef MSDOSREAL
    hfree(mem);
#else
    free(mem);
#endif
    fclose(fp);
    return -1;
  }
  if(NULL == (hexdata = malloc(HEXLINESIZE/2))){
    hexallocerr();
#ifdef MSDOSREAL
    hfree(mem);
#else
    free(mem);
#endif
    free(hexline);
    fclose(fp);
    return -1;
  }
  if(NULL == (kawari = malloc(500))){
    hexallocerr();

#ifdef MSDOSREAL
    hfree(mem);
#else
    free(hexline);
#endif
    free(hexdata);
    fclose(fp);
    return -1;
  }
  while(1){
    if(NULL == (fgets(hexline,HEXLINESIZE,fp))){
      break;
    } 
    if(hexline[0]!=':'){
      err = 1;
      break;
    }
    if(ishex(hexline[1]) || ishex(hexline[2])){
      err = 1;
      break;
    }
    rb = hex2toi(&hexline[1]); 
    if(H4err){
      err = 1;
      break;
    }
    H4err = 0;
    if(sumcheck(&hexline[1],rb,hexdata)){
      err = 2;
      break;
    }
    if(H4err){
      err = 1;
      break;
    }
    ad = hex4tou(&hexline[3]);	/* address */
    if(H4err){
      err = 1;
      break;
    }
    rt = hex2toi(&hexline[7]);	/* record type */
    if(H4err){
      err = 1;
      break;
    }
    if(rt <0 || rt>5){
      err = 1;
      break;
    }
    if(rt > 2){
      err = 4;
      break;
    }
    if(rt == 2){	/* segment data */
      if(rb!=2){
        err = 1;
        break;
      }
      segment = hex4tou(&hexline[9]) *16;
      if(H4err){
        err = 1;
        break;
      }
    }
    else
    if(!rt){	/* data record */
      if(rb){
        if(ad + segment +rb-1 > 0xffff){
          err = 3;
          break;
        }
        if(mad != (ad + segment)){ /* banci ga kawatta */
          *(kawari+(Kat*2)) = ad + segment;
          if(Kat){
            *(kawari+((Kat*2)-1)) = mad - 1;
          }
          ++Kat;
          if(Kat >=500){
            if(Alpf){
              fprintf(stderr,"hex file area > 500\n");
              fprintf(stderr,"500 made sika taiou shite imasen.");
            }
            else{
/*KANJI*/     fprintf(stderr,"このヘキサファイルは、あまりにもバラバラなコード配置で、\n");
/*KANJI*/     fprintf(stderr,"領域の数が500を越えています。\n");
/*KANJI*/     fprintf(stderr,"500までしか対応していないので処理できません。\n");
            }
            fclose(fp);
            free(hexline);
            free(hexdata);

#ifdef MSDOSREAL
            hfree(mem);
#else
            free(mem);
#endif
            return -1;
          }
          /* mad = ad+segment+rb; */
        }
        for(i=0;i<rb;++i){
          mem[ad+segment+i]=hexdata[i];
        }
        mad = ad+segment+rb;	/* 1998-3-22  bug syuusei */
      }
    }
    if(rt == 1){		/* end of file record */
      if(Kat){
        *(kawari+(Kat*2)-1) = mad -1;
        if(Kat > 1)
       	  bsort();	/* hex segment sort */
        Hexorg = *kawari;
        fsize = (unsigned long)(*(kawari+((Kat-1)*2)+1)+1);
      }
      break;
    }
  }
  fclose(fp);
  free(hexline);
  free(hexdata);
  if(err == 1){
    if(Alpf)
      fprintf(stderr,"intel hex file syntax error\n");
    else{
/*KANJI*/      fprintf(stderr,"インテルヘキサファイル文法エラー\n");
    }
  }
  else
  if(err == 2){
    if(Alpf)
      fprintf(stderr,"intel hex file sum check error \n");
    else{
/*KANJI*/      fprintf(stderr,"インテルヘキサファイル チェックサムエラー\n");
    }
  }
  else
  if(err == 3){
    if(Alpf)
      fprintf(stderr,"extended hex file address 64k over\n");
    else{
/*KANJI*/      fprintf(stderr,"拡張インテルヘキサファイル アドレス64k越えた\n");
    }
  }
  else
  if(err == 4){
    if(Alpf)
      fprintf(stderr,"intel hex file record type 3-5 taiou site imasen.\n");
    else{
/*KANJI*/      fprintf(stderr,"インテルヘキサ レコードタイプ3-5 未対応です。\n");
    }
  }
  if(err){
#ifdef MSDOSREAL
    hfree(mem);
#else
    free(mem);
#endif
  }
  return err;
}

#ifdef TANTAITEST
int main(argc,argv)
int argc;
char **argv;
{
  int err;
  if(argc < 2){
    printf("ihex filename\n");
    return 1;
  }
  err = hex(argc,argv[1]);
  if(err)
    return 1;
  else{
    testkt();
  }
  return 0;
}
#endif 


