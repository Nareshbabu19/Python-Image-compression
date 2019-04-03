//####################################################################################
//----------------------------Project: Image Compression-----------------------------#
//----------------------------Programmer : Suman Debnath-----------------------------#
//--------------------------Computer Science & Engineering---------------------------#
//-------------Academy Of Technology, West Bengal, India, Batch 2003-2007------------#
//-----------------------This Program Only Be Compiled In UNIX-----------------------# 
//-----------------[ input file should be a grey level bitmap file ]-----------------#
//-----------[ output format is *.cbi (Compressed Bitmap Image) format ]-------------#
//####################################################################################
//--------------------- This Code Is To Decompress CBI Files ------------------------#
//####################################################################################
//---------------- For Further Help Mail To : debnath.suman@yahoo.co.in -------------#
//----------------------- Friday, 27 October, 2006, 11:00:13 PM ---------------------# 
//####################################################################################


#include<stdio.h>
#include<math.h>
#include<stdlib.h>
//#include <inttypes.h>
#define BYTE char
#define MAXX 1400
#define MAXY 1400
#define MAXDATA MAXX*MAXY 
#define MAXBLOCKS ((MAXX/8)*(MAXY/8)) 

void bitmap_info(FILE*);
void CBI_info(FILE*);
void CBI_data(FILE*);
void check_CBI();
void fskip_char(FILE*,int);

void setDCT();
void decode();
void createBlock();

void decompress();
void deMxMul();

double ROUND(double);
void createFile(char*,char*); 


struct CBIFILEHEADER
{
//  members           //size//purpose
char ch1;             //1   // set to C
char ch2;             //1   // set to B
char ch3;             //1   // set to I
int lengthOfCode;     //4   // length of the enhanced run-length code
int partitionValue;   //4   // value of program generated partitionning value that counts the no. of zeros in the code
int OffSet;           //4   // size of bitmap offset
int numberOfBlocks;   //4   // number of program generated [8x8] blocks 
int cbiHeaderSize;    //4   // value=28, size of CBIFILEHEADER structure, in bytes 
int cbiFileSize;      //4   // size of CBI file
char ch4;             //1   // set to F
};


struct BITMAPFILEHEADER
{
//  members          //start  size//purpose
struct           
{
 char char1;
 char char2;
}bfType;             //1      2  //set to 'BM'
int bfSize;       	 //3      4  //size of file in bytes 
int bfReserved1;  	 //7      2  //always set to 0
int bfReserved2;  	 //9      2  //always set to 0
int bfOffBits;    	 //11     4  //usually value=54, offset from the beginning of the file to the bitmap data
};

struct BITMAPINFOHEADER
{
//  members              //start  size//purpose
int biSize;              //15     4   //value=40, size of BITMAPINFOHEADER structure, in bytes
int biWidth;             //19     4   //width of the image in pixels
int biHeight;            //23     4   //height of the image in pixels
int biPlanes;            //27     2   //number of planes of the terget device, must be set to 0
int biBitCount;          //29     2   //number of bits per pixel
int biCompression;       //31     4   //type of compression, set to 0 for no compression
int biSizeImage;         //35     4   //size of image data in bytes, (***if no compression, then value=0***)  
int biXPelsPerMeter;     //39     4   //horizontal pixels/meter on the designated terget device, usually set to 0
int biYPelsPerMeter;     //43     4   //vertical pixels/meter on the designated terget device, usually set to 0
int biClrUsed;           //47     4   //no of colors used in bitmap,if set to 0 then no. of colors is calculated using biBitCount member
int biClrImportant;      //51     4   //no of colors that are 'important' for the bitmap, if set to 0 then all are important
};

struct RGBQUAD
{
int rgbBlue;      	 //the blue part of the color
int rgbGreen;     	 //the green part of the color
int rgbRed;       	 //th red part of the color
int rgbReserved; 	 //must always set to 0
};
//---------------------------
struct CBIFILEHEADER   cbifh;
struct BITMAPFILEHEADER bmfh;
struct BITMAPINFOHEADER bmih;
struct RGBQUAD   aColors[24];
char       CODE[MAXDATA]={0};
short       Image[MAXX][MAXY];
double   readyCODE[MAXDATA]={0};
double aBitmapBits[MAXDATA]={0};
double M[8][8]={0.0},D[8][8]={0.0},T[8][8]={0.0},_T[8][8]={0.0},N[MAXBLOCKS][8][8]={0.0},temp[8][8]={0.0},BLOCK[MAXBLOCKS][8][8]; // 1400/8=175
double C[MAXBLOCKS][8][8]={0.0},DECODE[MAXDATA]={0},R[8][8]={0};
int X,Y,lengthOfData=0;


int Q50[8][8]={16,11,10,16,24,40,51,61,
               12,12,14,19,26,58,60,55,
	       14,13,16,24,40,57,69,56,
	       14,17,22,29,51,87,80,62,
	       18,22,37,56,68,109,103,77,
	       24,35,55,64,81,104,113,92,
	       49,64,78,87,103,121,120,101,
	       72,92,95,98,112,100,103,99};
	       
int Q10[8][8]={80,60,50,80,120,200,255,255,
               55,60,70,95,130,255,255,255,
	       70,65,80,120,200,255,255,255,
	       70,85,110,145,255,255,255,255,
	       90,110,185,255,255,255,255,255,
	       120,175,255,255,255,255,255,255,
	       245,255,255,255,255,255,255,255,
	       255,255,255,255,255,255,255,255};
	       
int Q90[8][8]={3,2,2,3,5,8,10,12,
               2,2,3,4,5,12,12,11,
	       3,3,3,5,8,11,14,11,
	       3,3,4,6,10,17,16,12,
	       4,4,7,11,14,22,21,15,
	       5,7,11,13,16,12,23,18,
	       10,13,16,17,21,24,24,21,
	       14,18,19,20,22,20,20,20};              	       
	             //0     1     2     3     4     5     6    7 
int define[64][2]={{0,0},{0,1},{1,0},{2,0},{1,1},{0,2},{0,3},{1,2},
		   {2,1},{3,0},{4,0},{3,1},{2,2},{1,3},{0,4},{0,5},
		   {1,4},{2,3},{3,2},{4,1},{5,0},{6,0},{5,1},{4,2},
		   {3,3},{2,4},{1,5},{0,6},{0,7},{1,6},{2,5},{3,4},
		   {4,3},{5,2},{6,1},{7,0},{7,1},{6,2},{5,3},{4,4},
		   {3,5},{2,6},{1,7},{2,7},{3,6},{4,5},{5,4},{6,3},
		   {7,2},{7,3},{6,4},{5,5},{4,6},{3,7},{4,7},{5,6},
		   {6,5},{7,4},{7,5},{6,6},{5,7},{6,7},{7,6},{7,7}};	       
//---------------------------

int main()
{
FILE *cbi;
int  i,j,n,dataLength;
char source[120]="\0",ch;
char dest[120]="\0";

//getting source file
printf("Enter The Source CBI File to Decompress :- ");
fflush(stdin);
i=0;
while(ch!='\n')
{
 fflush(stdin);
 ch=getchar();
 source[i++]=ch;
}
fflush(stdin);
i--;
source[i]='\0';

//getting destination file
printf("Enter The Destination Bitmap File to Create :- ");
fflush(stdin);
i=0;
ch=0;
while(ch!='\n')
{
 fflush(stdin);
 ch=getchar();
 dest[i++]=ch;
}
fflush(stdin);
i--;
dest[i]='\0';


printf("Set Source CBI File :%s\n",source);
printf("Set Destination Bitmap File :%s\n",dest);


if((cbi=fopen(source,"rb"))!=NULL)
{
 CBI_info(cbi);
 bitmap_info(cbi);
 printf("\nCollecting CBI Data :\n");
 CBI_data(cbi);
 printf("\nDONE !\n");
 printf("\nSetting DCT Matrix & Starting Decoding :\n");
 setDCT();
 decode();
 printf("\nDONE !\n");
 printf("\nCreating Blocks :\n");
 createBlock();
 printf("\nDONE !\n");
 
 printf("\nStarting Decompressing :\n");
 //decompressing data
 for(n=0;n<cbifh.numberOfBlocks;n++)
 {
  decompress(n);
 }
 printf("\nDONE !\n");
 printf("\nRetrieving Original Data :\n");
 //retrieving original data
 dataLength=0;
 for(n=0;n<cbifh.numberOfBlocks;n++)
  for(i=0;i<8;i++)
    for(j=0;j<8;j++)
    {
      aBitmapBits[dataLength++]=N[n][i][j];
    } 
 printf("\nDONE !\n");  
 printf("\nReconstructing Bitmap File :\n");  
 //reconstructing *.bmp file
 createFile(source,dest);   
 printf("\nDONE !\n");
}
else
 {
  printf("File Openning Error");
  fclose(cbi);
  exit(0);
 } 

//showing original data
for(n=0;n<cbifh.numberOfBlocks*64;n++)
 {
  readyCODE[n]=aBitmapBits[n];
 } 
 
fclose(cbi);
printf("\n\nThe Output Bitmap File Is [%0.2f times] Decompressed\n\n",(float)bmfh.bfSize/(float)cbifh.cbiFileSize);

return 0;
}

//--------------------------------
//----Collect CBI information-----
//--------------------------------
void CBI_info(FILE *fp)
{
cbifh.ch1=fgetc(fp);
cbifh.ch2=fgetc(fp);
cbifh.ch3=fgetc(fp);
cbifh.lengthOfCode=getw(fp);
cbifh.partitionValue=getw(fp);
cbifh.OffSet=getw(fp);
cbifh.numberOfBlocks=getw(fp);
cbifh.cbiHeaderSize=getw(fp);
cbifh.cbiFileSize=getw(fp);
cbifh.ch4=fgetc(fp);

check_CBI();
printf("\n########### Reading CBI File :  #############\n");
printf("File Type 		: %c%c%c\n",cbifh.ch1,cbifh.ch2,cbifh.ch3);
printf("Length of CODE		: %d\n",cbifh.lengthOfCode);
printf("Partition Value		: %d\n",cbifh.partitionValue);
printf("Size of OffSet of Bitmap: %d Bytes\n",cbifh.OffSet);
printf("Number of Blocks	: %d\n",cbifh.numberOfBlocks);
printf("Size of CBIHEADER       : %d Bytes\n",cbifh.cbiHeaderSize);
printf("Size of CBI File        : %d Bytes\n",cbifh.cbiFileSize);
printf("End Character		: %c\n",cbifh.ch4);
printf("\n########### END OF CBI INFORMATION ###########\n");

}

//-------------------------------------------------
//--Checking whether the input file is CBI or not--
//-------------------------------------------------
void check_CBI()
{
 
if(cbifh.ch1!='C' || cbifh.ch2!='B' || cbifh.ch3!='I' || cbifh.ch4!='F' || cbifh.cbiHeaderSize!=28)
 {
  printf("\nThe Input File Is Not A Compatible CBI File!\nAborting.......\n");
  exit(0);
 }
}

//--------------------------------
//---Collect bitmap information---
//--------------------------------
void bitmap_info(FILE *fp)
{
 bmfh.bfType.char1=fgetc(fp); 
 bmfh.bfType.char2=fgetc(fp); 
 bmfh.bfSize=getw(fp);
 bmfh.bfReserved1=fgetc(fp);
 fskip_char(fp,1);
 bmfh.bfReserved2=fgetc(fp);
 fskip_char(fp,1);
 bmfh.bfOffBits=getw(fp);
 
 bmih.biSize=getw(fp);
 bmih.biWidth=getw(fp);
 bmih.biHeight=getw(fp);
 bmih.biPlanes=fgetc(fp);
 fskip_char(fp,1);
 bmih.biBitCount=fgetc(fp);
 fskip_char(fp,1);
 bmih.biCompression=getw(fp);
 bmih.biSizeImage=getw(fp);
 bmih.biXPelsPerMeter=getw(fp);
 bmih.biYPelsPerMeter=getw(fp);
 bmih.biClrUsed=getw(fp);
 bmih.biClrImportant=getw(fp);

printf("\n########### BITMAP information of CBI file #############\n");
printf("File Type 				: %c%c\n",bmfh.bfType.char1,bmfh.bfType.char2);
printf("File Size			        : %d Bytes\n",bmfh.bfSize);
printf("Reserved1 				: %d\n",bmfh.bfReserved1);
printf("Reserved2 				: %d\n",bmfh.bfReserved2);
printf("OffBits 				: %d\n",bmfh.bfOffBits);


printf("Size of BITMAPINFOHEADER 		: %d\n",bmih.biSize);
printf("Resolution 				: %d X %d\n",bmih.biWidth,bmih.biHeight);
printf("No of planes of the terget device 	: %d\n",bmih.biPlanes);
printf("No of bits per pixel 			: %d\n",bmih.biBitCount);
printf("Compression Type 			: %d\n",bmih.biCompression);
printf("Size of image data       		: %d Bytes\n",bmih.biSizeImage);
printf("Horizontal pixel/meter on terget device : %d\n",bmih.biXPelsPerMeter);
printf("Vertical pixel/meter on terget device   : %d\n",bmih.biYPelsPerMeter);
printf("No. of colors used                      : %d\n",bmih.biClrUsed);
printf("No. of important colors                 : %d\n",bmih.biClrImportant);
printf("############## END OF BITMAP INFORMATION ###############\n\n");
}
//--------------------------------
//-------Moves file pointer-------
//--------------------------------
void fskip_char(FILE *fl,int byte)
{
int i;

for(i=0;i<byte;i++)
 fgetc(fl);
}

//--------------------------------
//------Collecting CBI data-------
//--------------------------------
void CBI_data(FILE *fs)
{
 char ch;
 int k=0,i,j,tempX,tempY;
 
 rewind(fs);
 fskip_char(fs,cbifh.cbiHeaderSize+cbifh.OffSet);
 
 while(k<cbifh.lengthOfCode)
  {
    CODE[k++]=fgetc(fs); 
  }
  
}

//--------------------------------
//---Set Matrix T & T' for DCT----
//--------------------------------
void setDCT()
{
int i,j;

//DCT matrix
for(i=0;i<8;i++)
 for(j=0;j<8;j++)
  {
    if(i==0)
     T[i][j]=1.0/sqrt(8.0);
    else
     T[i][j]=sqrt(2.0/8.0)*cos(((2.0*j+1.0)*i*3.14)/(2.0*8.0));
  }

//Transpose of T, _T
for(i=0;i<8;i++)
 for(j=0;j<8;j++)
  _T[i][j]=T[j][i];
}

//--------------------------------
//------Decode the CBI code-------
//--------------------------------
void decode()
{
int i=0,j,k,zeroLength=0;
int r;

for(k=0;k<cbifh.lengthOfCode;k++)
{
 if(CODE[k]<=cbifh.partitionValue)
 {
   DECODE[i++]=CODE[k];
 } 
 else
  {
    zeroLength=CODE[k]-cbifh.partitionValue;
    i=i+zeroLength;
  } 
}
}

//---------------------------------------------
//------Create run length encoded Blocks-------
//---------------------------------------------
void createBlock()
{
int i,j,k=0,n;

for(n=0;n<cbifh.numberOfBlocks;n++)
{
 for(i=0;i<64;i++)
  {
   C[n][define[i][0]][define[i][1]]=DECODE[k++];
  }
} 
      
}

//--------------------------------
//------Decompress the image------
//--------------------------------
void decompress(int n)
{
int i,j,k;
 
 //R=Q * C
   for(i=0;i<8;i++)
     for(j=0;j<8;j++)
       R[i][j]=(double)((Q50[i][j]))*C[n][i][j];

  //N=T'RT
  deMxMul(n);  
}

//------------------------------------
//--Decompress Matrix Multiplication--
//------------------------------------
void deMxMul(int n)
{
  double c[8][8];
  int i,j,k;
   
  //N=T'RT
  for(i=0;i<8;i++)
   {
     for(j=0;j<8;j++)
      {
        c[i][j]=0.0;
        for(k=0;k<8;k++)
         c[i][j]=c[i][j] + _T[i][k]*R[k][j];
      }
   }        
   
   for(i=0;i<8;i++)
   {
     for(j=0;j<8;j++)
      {
        temp[i][j]=0.0;
        for(k=0;k<8;k++)
         temp[i][j]=temp[i][j] + (c[i][k]*T[k][j]);
      }
   }        
  
  //Decompressed BLOCK   
  for(i=0;i<8;i++)
    for(j=0;j<8;j++)
     {
      N[n][i][j]=(temp[i][j]) + 128; 
      if(N[n][i][j]<0)
       N[n][i][j]=0;   
     } 
}  

//------------------------------------
//---------Rounding function----------
//------------------------------------
double ROUND(double x)
{
 double tmp;
 if(x<0.0)
  {
   tmp=ceil(x)-0.5;
   
   if(x>tmp)
    return(ceil(x));
   else
    return(floor(x));
  }
else 
 if(x>0.0)  
  {
   tmp=floor(x)+0.5;
   
    if(x<tmp)
     return(floor(x));
    else
     return(ceil(x)); 
   }
  else
   return(abs(0.0));      
}


//------------------------------------
//--------Creating *.bmp File---------
//------------------------------------
void createFile(char *src,char *dst)
{
FILE *fs,*fd;
int i,j;

fs=fopen(src,"rb");
fd=fopen(dst,"wb");

fskip_char(fs,cbifh.cbiHeaderSize);

//writing header
for(i=0;i<cbifh.OffSet;i++)
 fputc(fgetc(fs),fd);

//writing data
if(bmih.biSizeImage==0)
 bmih.biSizeImage=bmfh.bfSize-bmfh.bfOffBits;

for(j=0;j<bmih.biSizeImage;j++) 
 {
  fputc(aBitmapBits[j],fd);
 }

fclose(fs);
fclose(fd);
}

