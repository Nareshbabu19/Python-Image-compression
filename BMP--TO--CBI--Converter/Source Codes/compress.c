//####################################################################################
//----------------------------Project: Image Compression-----------------------------#
//----------------------------Programmer : Suman Debnath-----------------------------#
//--------------------------Computer Science & Engineering---------------------------#
//-------------Academy Of Technology, West Bengal, India, Batch 2003-2007------------#
//-----------------------This Program Only Be Compiled In UNIX-----------------------# 
//-----------------[ input file should be a grey level bitmap file ]-----------------#
//-----------[ output format is *.cbi (Compressed Bitmap Image) format ]-------------#
//####################################################################################
//-------------------- This Code Is To Compress Bitmap Files ------------------------#
//####################################################################################
//---------------- For Further Help Mail To : debnath.suman@yahoo.co.in -------------#
//----------------------- Friday, 27 October, 2006, 11:00:13 PM ---------------------# 
//####################################################################################


#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include <inttypes.h>
#define BYTE char
#define MAXX 1400
#define MAXY 1400
#define MAXDATA MAXX*MAXY 
#define MAXBLOCKS ((MAXX/8)*(MAXY/8)) 

void bitmap_info(FILE*);
void bitmap_data(FILE*);
void check_BMP();

void compress(int);
void encode();
void copy_block();
void comMxMul();

void fskip_char(FILE*,int);
double ROUND(double);

void createFile(char*,char*);


struct BITMAPFILEHEADER
{
//  members       //start==size//purpose
struct           
{
 char char1;
 char char2;
}bfType;                 //1==2//set to 'BM'
int bfSize;       	 //3==4//size of file in bytes 
int bfReserved1;  	 //7=2//always set to 0
int bfReserved2;  	 //9==2//always set to 0
int bfOffBits;    	 //11==4//usually value=54, offset from the beginning of the file to the bitmap data
};

struct BITMAPINFOHEADER
{
//  members              //start==size//purpose
int biSize;              //15==4//value=40, size of BITMAPINFOHEADER structure, in bytes
int biWidth;             //19==4//width of the image in pixels
int biHeight;            //23==4//height of the image in pixels
int biPlanes;            //27==2//number of planes of the terget device, must be set to 0
int biBitCount;          //29==2//number of bits per pixel
int biCompression;       //31==4//type of compression, set to 0 for no compression
int biSizeImage;         //35==4//size of image data in bytes, (***if no compression, then value=0***)  
int biXPelsPerMeter;     //39==4//horizontal pixels/meter on the designated terget device, usually set to 0
int biYPelsPerMeter;     //43==4//vertical pixels/meter on the designated terget device, usually set to 0
int biClrUsed;           //47==4//no of colors used in bitmap,if set to 0 then no. of colors is calculated using biBitCount member
int biClrImportant;      //51==4//no of colors that are 'important' for the bitmap, if set to 0 then all are important
};

struct RGBQUAD
{
int rgbBlue;      	 //the blue part of the color
int rgbGreen;     	 //the green part of the color
int rgbRed;       	 //th red part of the color
int rgbReserved; 	 //must always set to 0
};
//---------------------------
struct BITMAPFILEHEADER bmfh;
struct BITMAPINFOHEADER bmih;
struct RGBQUAD   aColors[24];
short    aBitmapBits[MAXDATA]={0};
short    decomBitmapBits[MAXDATA]={0};
char     readyCODE[MAXDATA]={0};
double M[8][8]={0.0},D[8][8]={0.0},T[8][8]={0.0},_T[8][8]={0.0},N[MAXBLOCKS][8][8]={0.0},temp[8][8]={0.0},BLOCK[MAXBLOCKS][8][8]; // 1400/8=175
float C[MAXBLOCKS][8][8]={0.0},R[8][8]={0.0};
int X,Y,numberOfBlocks,CODE[MAXDATA]={0},lengthOfData=0,lengthOfCode=0;
int partitionValue=0,cbiFileSize;

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
int test[8][8]={0,1,5,6,14,15,27,28,
                2,4,7,13,16,26,29,42,
		3,8,12,17,25,30,41,43,
		9,11,18,24,31,40,44,53,
		10,19,23,32,39,45,52,54,
		20,22,33,38,46,51,55,60,
		21,34,37,47,50,56,59,61,
		35,36,48,49,57,58,62,63};	
		     
//---------------------------

int main()
{
FILE *bmp;
char source[120]="\0",ch;
char dest[120]="\0";
int i;

//getting source file
printf("Enter The Source Grey Level Bitmap File to Compress :- ");
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
printf("Enter The Destination CBI File to Create :- ");
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


printf("Set Source Bitmap File :%s\n",source);
printf("Set Destination CBI File :%s\n",dest);


if((bmp=fopen(source,"rb"))!=NULL)
{
bitmap_info(bmp);
check_BMP();

printf("\n########### Reading Bitmap File : %s #############\n",source);
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
printf("Size of image data       		: %d\n",bmih.biSizeImage);
printf("Horizontal pixel/meter on terget device : %d\n",bmih.biXPelsPerMeter);
printf("Vertical pixel/meter on terget device   : %d\n",bmih.biYPelsPerMeter);
printf("No. of colors used                      : %d\n",bmih.biClrUsed);
printf("No. of important colors                 : %d\n",bmih.biClrImportant);
printf("############## END OF BITMAP INFORMATION ###############\n\n");

//processing starts from here
bitmap_data(bmp);
}
else
 printf("File Openning Error");

fclose(bmp);

//creating *.cbi output file
createFile(source,dest);
  

printf("\nData Length :%d\n",lengthOfData);
printf("No of blocks :%d\n",numberOfBlocks);          
printf("Length of CODE :%d\n",lengthOfCode);
printf("The partitionValue :%d\n",partitionValue);  
printf("\n\nOutput CBI File Is [%0.2f %] Compressed\n\n",(1.0-(float)(cbiFileSize)/(float)bmfh.bfSize)*100.0); 

return 0;
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
}

//-------------------------------------------------
//--Checking whether the input file is BMP or not--
//-------------------------------------------------
void check_BMP()
{
 if(bmfh.bfType.char1!='B' || bmfh.bfType.char2!='M' || bmih.biSize!=40)
 {
  printf("\nThe Input File Is Not A Compatible Bitmap File!\nAborting.......\n");
  exit(0);
 }
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
//-----Collecting bitmap data-----
//--------------------------------
void bitmap_data(FILE *fs)
{
 short ch;
 int k=0,i,j,tempX,tempY;
 
 printf("\nCollecting Bitmap Data :\n");
 rewind(fs);
 fskip_char(fs,bmfh.bfOffBits);
 while(ch!=EOF)
  {
   ch=fgetc(fs);
   if(ch!=EOF)
    aBitmapBits[k++]=ch; 
  }
  lengthOfData=k;
  printf("\nDONE !\n");    
  
   //subtracting each pixel by 128
   for(k=0;k<lengthOfData;k++)
     aBitmapBits[k]= aBitmapBits[k]-128;  
  
  //calculating number of BLOCK
  numberOfBlocks=ceil(lengthOfData/64.0);
      
  copy_block();
}

//--------------------------------
//--------Copy 8X8 blocks---------
//--------------------------------
void copy_block()
{
int i=0,j=0,k=0,n=0;

 printf("\nCreating Blocks :\n");
 //creating BLOCKS form arranged data
 for(k=0;k<numberOfBlocks;k++)
 {  
    for(i=0;i<8;i++)
    {
     for(j=0;j<8;j++)
      BLOCK[k][i][j]=aBitmapBits[n++];
    }        
 }        
 printf("\nDONE !\n"); 
  //************** COMPRESSING ******************** 
  printf("\nStarting Discrete Cosine Transform & Quantization :\n");
  //Changing each BLOCK by D = TMT' 
  for(k=0;k<numberOfBlocks;k++)
   {
    compress(k);
   }
   printf("\nDONE !\n");
  //calculating partition value
   for(k=0;k<numberOfBlocks;k++)
    for(i=0;i<8;i++)
     for(j=0;j<8;j++)
       partitionValue=(partitionValue>C[k][i][j])?(partitionValue):(C[k][i][j]);
  
   printf("\nStarting Encoding :\n");    
  //Encode all BLOCK  
   {
    encode();
   } 
   printf("\nEncoding Completed Successfully !\n");
}


//--------------------------------
//-------Compress the image-------
//--------------------------------
void compress(int k)
{
int i,j;

//set M[i][j]
for(i=0;i<8;i++)
 for(j=0;j<8;j++)
  M[i][j]=BLOCK[k][i][j]; 
  
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

//D=TMT'  
  comMxMul();

//D[][] is DCT matrix   
 for(i=0;i<8;i++)
   for(j=0;j<8;j++)
     D[i][j]=temp[i][j]; 
       
//Quantization
for(i=0;i<8;i++)
 for(j=0;j<8;j++)
 { 
  C[k][i][j]= ROUND(D[i][j]/Q50[i][j]);  
 }  

//***** to forcefully compress *****
//setting 0's to the lower right corner of each BLOCK , to convert the color image into Grey Scale
for(i=2;i<8;i++)
 for(j=2;j<8;j++)
 { 
  C[k][i][j]= 0.0;  
 }  
//**** you can disable this portion if you need ****

}


//----------------------------------
//--Compress Matrix Multiplication--
//----------------------------------
void comMxMul()
{
  double c[8][8];
  int i,j,k;
   
  //D=TMT'
  for(i=0;i<8;i++)
   {
     for(j=0;j<8;j++)
      {
        c[i][j]=0.0;
        for(k=0;k<8;k++)
         c[i][j]=c[i][j] + T[i][k]*M[k][j];
      }
   }        
   
   for(i=0;i<8;i++)
   {
     for(j=0;j<8;j++)
      {
        temp[i][j]=0.0;
        for(k=0;k<8;k++)
         temp[i][j]=temp[i][j] + c[i][k]*_T[k][j];
      }
   }        
    
  for(i=0;i<8;i++)
    for(j=0;j<8;j++)
      D[i][j]=temp[i][j];
}  


//---------------------------------
//-----Encode quantized BLOCK------
//---------------------------------
void encode()
{
int i,j,k,DATA,partVal;
lengthOfCode=0;

printf("\nCreating Enhanced Run Length Code :\n");
for(k=0;k<numberOfBlocks;k++)
{
 //Enhanced run length coding
 partVal=partitionValue;
 for(i=0;i<64;)
 {  
    if((int)C[k][define[i][0]][define[i][1]]!=0)
     {
      CODE[lengthOfCode++]=(int)C[k][define[i][0]][define[i][1]]; 
      partVal=partitionValue;
      if(++i>=64)
       break;
     }
    else
    {
     while((int)C[k][define[i][0]][define[i][1]]==0)
      {
       CODE[lengthOfCode]=++partVal;  
       if(++i>=64)
        break;
      }
      lengthOfCode++;
    }
 } 
}
printf("\nDONE !\n");
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
   return(x);      
}


//------------------------------------
//--------Creating *.cbi File---------
//------------------------------------
void createFile(char *src,char *dst)
{
FILE *fs,*fd;
int i,j,cbiHeaderSize=28;


printf("\nCreating CBI file :\n");
fs=fopen(src,"rb");
fd=fopen(dst,"wb");

cbiFileSize=cbiHeaderSize+bmfh.bfOffBits+lengthOfCode;
//entering header of *.cbi format
fputc('C',fd);
fputc('B',fd);
fputc('I',fd);
putw(lengthOfCode,fd);
putw(partitionValue,fd);
putw(bmfh.bfOffBits,fd);
putw(numberOfBlocks,fd);
putw(cbiHeaderSize,fd);
putw(cbiFileSize,fd);
fputc('F',fd);

//copying header of *.bmp -> *.cbi
for(i=0;i<bmfh.bfOffBits;i++)
 fputc(fgetc(fs),fd);
 
//entering the CODE
for(i=0;i<lengthOfCode;i++)
{
 readyCODE[i]=CODE[i];
 fputc(readyCODE[i],fd); 
} 

fclose(fs);
fclose(fd);
printf("\nDONE !\n");
}


