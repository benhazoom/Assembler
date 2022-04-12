/*Symbols Table*/
/*This table manages the symbols around the code, data counter and instruction counter */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#define MAX_FILE_NAME_SIZE 100
#define LINEMAXSIZE 81 /*including '\0'*/
#define DATA_IMAGE_SIZE 80/*for data image, as said in page 33 of maman 14 instruction.*/
#define CODE_IMAGE_SIZE 80/*for code image, as said in page 44 of maman 14 instruction - we allowed to assume arrays for code image*/

#define CODE 61
#define DATA 401

typedef struct CodeWords
{
    int WordIC;
    int type;/*1 for R,2 for I,3 for J*/
    union Word
    {
         struct R
        {
            unsigned int notInUse:5;
            unsigned int funct:5;
            unsigned int rd:5;
            unsigned int rt:5;
            unsigned int rs:5;
            unsigned int opcode:6;
        }R;
         struct I
        {
            unsigned int immed:16;
            unsigned int rt:5;
            unsigned int rs:5;
            unsigned int opcode:6;
        }I;
         struct J
        {
            unsigned int address:25;
            unsigned int reg:1;
            unsigned int opcode:6;
        }J;
    }Word;
}CodeWords;

typedef struct symbol
{
    char name[31];
    int value;
    int attribute;/*CODE for code,DATA for data*/
    int Entry;   /*0 for not entry, 1 for entry*/
    int External;/*0 for not external, 1 for external*/
    struct symbol *link;
}symbol;
/*A method for displaying symbol table*/
void display(symbol start)
{
	symbol *ptr;
	ptr = &start;
	if (ptr == NULL)
		printf("\nLinklist is empty.\n");
	else
	{
            printf("Symbol\t\tValue\t\tAttribute\tisEntry\t\tisExternal\n");
ptr = ptr->link;
ptr = ptr->link;
		while(ptr != NULL)
        {
            printf("\n%s\t\t%d\t\t%d\t\t%d\t\t%d\n",ptr->name,ptr->value,ptr->attribute,ptr->Entry,ptr->External);
			ptr = ptr->link;
		}
	}
}
typedef struct dataWords
{
    int WordDC;
    int type;/*1 for ascii,2 for db,3 for dh,4 for dw*/
    union dataType
    {
         struct asciiChar
        {
            unsigned int value:8;
        }asciiChar;
         struct db
        {
            unsigned int value:8;
        }db;
         struct dh
        {
            unsigned int value:16;
        }dh;
         struct dw
        {
            unsigned int value:32;
        }dw;
    }dataType;
}dataWords;
/*Global Veriables*/
    int DC,DCF;/*data counter*/
    int items;
    int IC,ICF;/*points on the next available space in memory*/
    symbol start;
    symbol nextt;
    int Error;
     CodeWords CodeImage [CODE_IMAGE_SIZE];
     dataWords DataImage [DATA_IMAGE_SIZE];
    int codeImageCounter;
    int dataImageCounter;
    int dataImageCounter=0;
    int bitsLine[32];
    int k;

    FILE *InputFilePointer;
    FILE *obFilePointer;
    FILE *entFilePointer;
    FILE *extFilePointer;
    char fileName[MAX_FILE_NAME_SIZE];
    char assemblyFileName[MAX_FILE_NAME_SIZE];
    int extOpened;
    int entOpened;

/*A method for checking if a symbol is R type*/
int isR(char symbolName[])
{
if(
   strcmp(symbolName,"add")==0||
   strcmp(symbolName,"sub")==0||
   strcmp(symbolName,"and")==0||
   strcmp(symbolName,"or")==0 ||
   strcmp(symbolName,"nor")==0||
   strcmp(symbolName,"move")==0||
   strcmp(symbolName,"mvhi")==0||
   strcmp(symbolName,"mvlo")==0
   ) return 1;else return 0;
}
/*A method for checking if a symbol is I type*/
int isI(char symbolName[])
{
if(
   strcmp(symbolName,"addi")==0||
   strcmp(symbolName,"subi")==0||
   strcmp(symbolName,"andi")==0||
   strcmp(symbolName,"ori")==0||
   strcmp(symbolName,"nori")==0||
   strcmp(symbolName,"bne")==0||
   strcmp(symbolName,"beq")==0||
   strcmp(symbolName,"blt")==0||
   strcmp(symbolName,"bgt")==0||
   strcmp(symbolName,"lb")==0||
   strcmp(symbolName,"sb")==0||
   strcmp(symbolName,"lw")==0||
   strcmp(symbolName,"sw")==0||
   strcmp(symbolName,"lh")==0||
   strcmp(symbolName,"sh")==0
   ) return 1;else return 0;
}
/*A method for checking if a symbol is J type*/
int isJ(char symbolName[])
{
if(
   strcmp(symbolName,"jmp")==0||
   strcmp(symbolName,"la")==0||
   strcmp(symbolName,"call")==0||
   strcmp(symbolName,"stop")==0
   ) return 1;else return 0;
}
/*A method for checking if a letter is a low case letter for maman 14 project*/
int IsLowcase(char next)
{
if(next>=97&&next<=122) return 1;else return 0;
}
/*A method for checking if a letter is a high case letter*/
int IsHighcase(char next)
{
if(next>=65&&next<=90)  return 1;else return 0;
}
/*A method for for checking if a letter is a number*/
int IsNumber(char next)
{
if(next>=48&&next<=57 ) return 1;else return 0;
}
int charToDecimal (char a)
{
    return a-48;
}
/*A method for checking if data contant is valid*/
int contantCheck(char contant[],int lineNumber,char dataInstruction[])
{
    int i,z,commaCounter=0,mul=1,value=0;
    for (i=0;contant[i]!='\0';i++)
        {
            if((!(IsNumber(contant[i])))&&(contant[i]!='-')&&(contant[i]!='+')&&(contant[i]!=','))
            {
                printf("ERROR - non valid character in data contant  - line number %d (the character is '%c')\n",lineNumber,contant[i]);
                Error=1;/*Error with commas*/
            }
            if((contant[i]==',')&&(IsNumber(contant[i-1])==0))/*if a character before comma is not a number so the comma doesn't devide between numbers */
            {
                printf("ERROR - a number need to be between commas - line number %d (the character before comma is '%c')\n",lineNumber,contant[i-1]);
                Error=1;
            }
            if (contant[i]=='-') mul =-1;
            else if (contant[i]=='+') mul =1;
            else if((commaCounter!=-1)&&(contant[i]==',')) commaCounter++;
            else if (IsNumber(contant[i]))
            {
                for (z=0;(contant[i]!='\0')&&(contant[i]!=',')&&(contant[i]!='\n');z++)
                {
                    value=value*10;
                    value=value+charToDecimal(contant[i]);
                    i++;
                }
                if (strcmp(dataInstruction,"db")==0)
                {
                    DataImage[dataImageCounter].WordDC=DC;
                    DataImage[dataImageCounter].type=2;
                    DataImage[dataImageCounter].dataType.db.value=(value*mul);
                    items=items+1;;
                    dataImageCounter++;
                }
                if (strcmp(dataInstruction,"dh")==0)
                {
                    DataImage[dataImageCounter].WordDC=DC;
                    DataImage[dataImageCounter].type=3;
                    DataImage[dataImageCounter].dataType.dh.value=(value*mul);
                    items=items+2;
                    dataImageCounter++;
                }
                if (strcmp(dataInstruction,"dw")==0)
                {
                    DataImage[dataImageCounter].WordDC=DC;
                    DataImage[dataImageCounter].type=4;
                    DataImage[dataImageCounter].dataType.dw.value=(value*mul);
                    items=items+4;
                    dataImageCounter++;
                }
                value=0;
            }
        }


        if (contant[i-1]==',')
            {
                printf("ERROR - last character in data contant is ',' - line number %d (data contant is - %s)\n",lineNumber,contant);
                Error=1;
                commaCounter=-1;
            }
        return commaCounter;/*if no errors were found and data contant is valid returns number of commas else returnes -1*/

}
/*A method for finding and returning an adress(value) of a lable*/
int AdressOfLable(char lableName[],int linenumber)
{
    symbol *ptr;
	ptr=&start;
	while (ptr != NULL)
	{
		if (strcmp(lableName,ptr->name)==0)
		{
			return ptr->value;
		}
		else
		{
			ptr = ptr->link;
		}
    }
    return -1;

}
/*A method for processing an R command*/
void Rcommand(int funct,int opcode,char contant[],int lineNumber)
{
    int i,signCounter;
    int number [2];
    CodeImage[codeImageCounter].Word.R.funct=funct;
    CodeImage[codeImageCounter].Word.R.opcode=opcode;
    CodeImage[codeImageCounter].WordIC=IC;
    signCounter=0,Error=0;

if (opcode==0) /*3 registers*/
{
    for(i=0;((contant[i]!='\0')&&(signCounter<3));i++)
    {
         while ((contant[i]==' ')||(contant[i]=='\t'))/*skips blanks*/
            {
                i++;
            }

        if(contant[i]=='$')
        {
            i++;
                if (!(IsNumber(contant[i])))/*there isnt a number after $*/
                {
                printf("ERROR - not a valid number after '$' sign in R command - line number %d (command contant is - %s)\n",lineNumber,contant);
                Error=1;
                }
                else /*valid register*/
                {
                    number[0]=contant[i];

                    if(IsNumber(contant[i+1]))/*two digits reg*/
                    {
                        number[1]=contant[i+1];
                        if (((charToDecimal(number[1])+charToDecimal(number[0])*10))>31)
                        {
                        printf("ERROR - register number is not valid (higher than 31) - line number %d (command contant is - %s)\n",lineNumber,contant);
                        Error=1;
                        }
                        else
                        {
                            if(signCounter==0)/*rs*/
                            {
                                CodeImage[codeImageCounter].Word.R.rs=(charToDecimal(number[1])+(charToDecimal(number[0])*10));
                            }
                            if(signCounter==1)/*rt*/
                            {
                                CodeImage[codeImageCounter].Word.R.rt=(charToDecimal(number[1])+(charToDecimal(number[0])*10));
                            }
                            if(signCounter==2)/*rd*/
                            {
                                CodeImage[codeImageCounter].Word.R.rd=(charToDecimal(number[1])+(charToDecimal(number[0])*10));
                            }
                        }
                        signCounter++;
                        i=i+2;

                        while ((contant[i]==' ')||(contant[i]=='\t'))/*skips blanks*/
                        {
                            i++;
                        }
                        if((contant[i]!=',')&&(signCounter!=2))
                        {
                            printf("ERROR - R command contant is invalid - line number %d (command contant is - %s)\n",lineNumber,contant);
                            Error=1;
                        }
                    }


                    else/*single digit register*/
                    {
                        if(signCounter==0)/*rs*/
                        {
                            CodeImage[codeImageCounter].Word.R.rs=charToDecimal(number[0]);
                        }
                        if(signCounter==1)/*rt*/
                        {
                            CodeImage[codeImageCounter].Word.R.rt=charToDecimal(number[0]);
                        }
                        if(signCounter==2)/*rd*/
                        {
                            CodeImage[codeImageCounter].Word.R.rd=charToDecimal(number[0]);
                        }
                        signCounter++;
                    }
                }/*End of valid reg*/
number[0]=0; number[1]=0;/*reseting register value for next register*/
        }/*End of current '$/*/
    }/*End of for loop*/
    if(signCounter<3)
    {
        printf("ERROR - not enough registers - line number %d (command contant is - %s)\n",lineNumber,contant);
        Error=1;
    }

}/*End of opcode == 0*/

if (opcode==1) /*2 registers*/
{
    for(i=0;((contant[i]!='\0')&&(signCounter<2));i++)
    {
         while ((contant[i]==' ')||(contant[i]=='\t'))/*skips blanks*/
            {
                i++;
            }

        if(contant[i]=='$')
        {
            i++;
                if (!(IsNumber(contant[i])))/*there isnt a number after $*/
                {
                printf("ERROR - not a number after '$' sign in R command - line number %d (command contant is - %s)\n",lineNumber,contant);
                Error=1;
                }
                else /*valid register*/
                {
                    number[0]=contant[i];

                    if(IsNumber(contant[i+1]))/*two digits reg*/
                    {
                        number[1]=contant[i+1];
                        if (((charToDecimal(number[1])+charToDecimal(number[0])*10))>31)
                        {
                        printf("ERROR - register number is not valid (higher than 31) - line number %d (register number is - %d)\n",lineNumber,(charToDecimal(number[1])+charToDecimal(number[0])*10));
                        Error=1;
                        signCounter++;
                        }
                        else
                        {
                            if(signCounter==0)/*rs*/
                            {
                                CodeImage[codeImageCounter].Word.R.rs=(charToDecimal(number[1])+(charToDecimal(number[0])*10));
                            }
                            if(signCounter==1)/*rd*/
                            {
                                CodeImage[codeImageCounter].Word.R.rd=(charToDecimal(number[1])+(charToDecimal(number[0])*10));
                            }
                        }
                        signCounter++;
                        i=i+2;
                        while ((contant[i]==' ')||(contant[i]=='\t'))/*skips blanks*/
                        {
                            i++;
                        }
                        if((contant[i]!=',')&&(signCounter!=1))
                        {
                            printf("ERROR - R command contant is invalid - line number %d (command contant is - %s)\n",lineNumber,contant);
                            Error=1;
                        }
                    }
                    else/*single digit register*/
                    {
                        if(signCounter==0)/*rs*/
                        {
                            CodeImage[codeImageCounter].Word.R.rs=charToDecimal(number[0]);
                        }
                        if(signCounter==1)/*rd*/
                        {
                            CodeImage[codeImageCounter].Word.R.rd=charToDecimal(number[0]);
                        }
                        signCounter++;
                    }
                }/*End of valid reg*/
number[0]=0; number[1]=0;/*reseting register value for next register*/
        }/*End of current '$/*/
    }/*End of for loop*/
}/*End of opcode==1*/
CodeImage[codeImageCounter].type=1;/*R type*/
codeImageCounter++;
}/*End of R commands*/
/*A method for processing an I command*/
void Icommand(int opcode,char contant[],int lineNumber)
{
/*VERIABLES*/
int i,z,signCounter,toAdress;
int number[2];
char adressLable[32];
CodeImage[codeImageCounter].WordIC=IC;
CodeImage[codeImageCounter].Word.I.opcode=opcode;

i=0,signCounter=0,Error=0;
/*I ARITHMETHIC LOGIC COMMANDS*/
if((opcode>=10)&&(opcode<=14))
{
    for(i=0;((contant[i]!='\0')&&(signCounter<3));i++)
    {
    /*skips blanks*/
    while ((contant[i]==' ')||(contant[i]=='\t'))
    {
        i++;
    }
    if (contant[i]==',')
    {
        if (signCounter==0)
        {
            printf("ERROR - comma at the beginning of the contant - line number %d (command contant is - %s)\n",lineNumber,contant);
            Error=1;
        }
        i++;
        while ((contant[i]==' ')||(contant[i]=='\t'))
        {
            i++;
        }
    }
        if(contant[i]=='$')
        {
            i++;
            if (!(IsNumber(contant[i])))/*there isnt a valid (positive)number after $*/
            {
            printf("ERROR - not a valid number after '$' sign in I command - line number %d (command contant is - %s)\n",lineNumber,contant);
            Error=1;
            }
            else /*valid register*/
            {
                number[0]=contant[i];
                if(IsNumber(contant[i+1]))/*two digits reg*/
                {
                    number[1]=contant[i+1];
                    if (((charToDecimal(number[1])+charToDecimal(number[0])*10))>31)
                    {
                    printf("ERROR - register number is not valid (higher than 31) - line number %d (register number is - %d)\n",lineNumber,(charToDecimal(number[1])+charToDecimal(number[0])*10));
                    Error=1;
                    }
                    else
                    {
                        if(signCounter==0)/*rs*/
                        {
                            CodeImage[codeImageCounter].Word.I.rs=(charToDecimal(number[1])+(charToDecimal(number[0])*10));
                        }
                        if(signCounter==1)/*rt*/
                        {
                            CodeImage[codeImageCounter].Word.I.rt=(charToDecimal(number[1])+(charToDecimal(number[0])*10));
                        }
                    }
                    signCounter++;
                    i=i+2;

                    while ((contant[i]==' ')||(contant[i]=='\t'))/*skips blanks*/
                    {
                        i++;
                    }
                    if((contant[i]!=',')&&(signCounter!=3))
                    {
                        printf("ERROR - I command contant is invalid - line number %d (command contant is - %s)\n",lineNumber,contant);
                        Error=1;
                    }
                }

                else/*single digit register*/
                {
                    if(signCounter==0)/*rs*/
                    {
                        CodeImage[codeImageCounter].Word.I.rs=charToDecimal(number[0]);
                    }
                    if(signCounter==2)/*rt*/
                    {
                        CodeImage[codeImageCounter].Word.I.rt=charToDecimal(number[0]);
                    }
                    signCounter++;
                    i++;
                }
            }/*End of valid reg*/
            number[0]=0; number[1]=0;/*reseting register value for next register*/
        }/*End of current '$*/

        /*IMMED*/
        else/*not $*/
        {
            while ((contant[i]==' ')||(contant[i]=='\t'))
            {
                i++;
            }
            if(((IsNumber(contant[i])==0))&&(contant[i]!='-')&&(contant[i]!='+'))
            {
                printf("ERROR -immed is not a number  - line number %d (command contant is - %s)\n",lineNumber,contant);
                Error=1;
            }
            else
            {
                int number=0,mul,j=0;
                if((contant[i])=='-')
                {
                    mul = -1;
                    i++;
                }
                else if((contant[i])=='+')
                {
                    mul =  1;
                    i++;
                }
                else mul=1;/*no sign means positive number*/
                /*calculating the number*/
                while (IsNumber(contant[i+j]))/*counting digits*/
                {
                    j++;
                }
                for (;j>0;j--)
                {
                    number=number+charToDecimal(contant[i])*pow(10,j-1);
                    i++;
                }
                number=number*mul;
                CodeImage[codeImageCounter].Word.I.immed=number;
                signCounter++;
            }
        }
    }/*End of for loop*/
/*End of opcode 10-14*/
}
/*I CONDITIONAL BRANCHING COMMANDS*/
if((opcode>=15)&&(opcode<=18))
{

    for(i=0;((contant[i]!='\0')&&(signCounter<2));i++)
    {
         while ((contant[i]==' ')||(contant[i]=='\t'))/*skips blanks*/
            {
                i++;
            }

        if(contant[i]=='$')
        {
            i++;
                if (!(IsNumber(contant[i])))/*there isnt a number after $*/
                {
                printf("ERROR - not a valid number after '$' sign in I command - line number %d (command contant is - %s)\n",lineNumber,contant);
                Error=1;
                }
                else /*valid register*/
                {
                    number[0]=contant[i];

                    if(IsNumber(contant[i+1]))/*two digits reg*/
                    {
                        number[1]=contant[i+1];
                        if (((charToDecimal(number[1])+charToDecimal(number[0])*10))>31)
                        {
                        printf("ERROR - register number is not valid (higher than 31) - line number %d (register number is - %d)\n",lineNumber,(charToDecimal(number[1])+charToDecimal(number[0])*10));
                        Error=1;
                        }
                        else
                        {
                            if(signCounter==0)/*rs*/
                            {
                                CodeImage[codeImageCounter].Word.I.rs=(charToDecimal(number[1])+(charToDecimal(number[0])*10));
                            }
                            if(signCounter==1)/*rd*/
                            {
                                CodeImage[codeImageCounter].Word.I.rt=(charToDecimal(number[1])+(charToDecimal(number[0])*10));
                            }
                        }
                        signCounter++;
                        i=i+2;
                        while ((contant[i]==' ')||(contant[i]=='\t'))/*skips blanks*/
                        {
                            i++;
                        }
                        if((contant[i]!=',')&&(signCounter!=1))
                        {
                            printf("ERROR - I command contant is invalid - line number %d (command contant is - %s)\n",lineNumber,contant);
                            Error=1;
                        }
                    }
                    else/*single digit register*/
                    {
                        if(signCounter==0)/*rs*/
                        {
                            CodeImage[codeImageCounter].Word.I.rs=(charToDecimal(number[0]));
                        }
                        if(signCounter==1)/*rd*/
                        {
                            CodeImage[codeImageCounter].Word.I.rt=(charToDecimal(number[0]));
                        }
                        signCounter++;
                    }
                }/*End of valid reg*/
number[0]=0; number[1]=0;/*reseting register value for next register*/
        }/*End of current '$/*/
    }/*End of for loop for two operands*/
    /*ADRESS COMPARISON*/
    z=0;
    while ((contant[i]==' ')||(contant[i]=='\t'))/*skips blanks*/
    {
        i++;
    }
    for(;contant[i]!='\0'&&contant[i]!='\n';i++)
    {
      adressLable[z]=contant[i];
      z++;
    }
    toAdress=AdressOfLable(adressLable,lineNumber);
    CodeImage[codeImageCounter].Word.I.immed=toAdress-IC;
}/*End of opcode 15-18*/


/*I LOAD AND SAVE IN MEMORY COMMANDS*/
if((opcode>=19)&&(opcode<=24))
{
    for(i=0;((contant[i]!='\0')&&(signCounter<3));i++)
    {
        if (contant[i]==',') i++;
        /*skips blanks*/
        while ((contant[i]==' ')||(contant[i]=='\t'))
        {
            i++;
        }
        if(contant[i]=='$')
        {
            i++;
            if (!(IsNumber(contant[i])))/*there isn't a valid (positive)number after $*/
            {
            printf("ERROR - not a number after '$' sign in I command - line number %d (command contant is - %s)\n",lineNumber,contant);
            Error=1;
            }
            else if (signCounter==1)
            {
            printf("ERROR - a register has to be on the first or third spot of command sign in I command - line number %d (command contant is - %s)\n",lineNumber,contant);
            Error=1;
            }

            else /*valid register*/
            {
                number[0]=contant[i];
                if(IsNumber(contant[i+1]))/*two digits reg*/
                {
                    number[1]=contant[i+1];
                    if (((charToDecimal(number[1])+charToDecimal(number[0])*10))>31)
                    {
                    printf("ERROR - register number is not valid (higher than 31) - line number %d (register number is - %d)\n",lineNumber,(charToDecimal(number[1])+charToDecimal(number[0])*10));
                    Error=1;
                    }
                    else
                    {
                        if(signCounter==0)/*rs*/
                        {
                            CodeImage[codeImageCounter].Word.I.rs=(charToDecimal(number[1])+(charToDecimal(number[0])*10));
                        }
                        if(signCounter==2)/*rt*/
                        {
                            CodeImage[codeImageCounter].Word.I.rt=(charToDecimal(number[1])+(charToDecimal(number[0])*10));
                        }
                    }
                    signCounter++;
                    i=i+2;
                }
                else/*single digit register*/
                {
                    if(signCounter==0)/*rs*/
                    {
                        CodeImage[codeImageCounter].Word.I.rs=charToDecimal(number[0]);
                    }
                    if(signCounter==2)/*rt*/
                    {
                        CodeImage[codeImageCounter].Word.I.rt=charToDecimal(number[0]);
                    }
                    signCounter++;
                    i++;
                }
                while ((contant[i]==' ')||(contant[i]=='\t'))/*skips blanks*/
                {
                    i++;
                }
                if((contant[i]!='\n')&&(contant[i]!='\0'))/*only after first ,*/
                if((contant[i]!=',')&&(signCounter==1))/*not a comma after first register*/
                {
                        printf("ERROR - I command contant is invalid - line number %d (command contant is - %s)\n",lineNumber,contant);
                        Error=1;
                }

            }/*End of valid reg*/

            number[0]=0; number[1]=0;/*reseting register value for next register*/

        }/*End of current '$*/

        /*IMMED*/
        else/*not $*/
        {
            while ((contant[i]==' ')||(contant[i]=='\t'))
            {
                i++;
            }
            if(((IsNumber(contant[i])==0))&&(contant[i]!='-')&&(contant[i]!='+'))
            {
                printf("ERROR -immed is not a number  - line number %d (command contant is - %s)\n",lineNumber,contant);
                Error=1;
            }
            else
            {
                int number=0,mul,j=0;
                if((contant[i])=='-')
                {
                    mul = -1;
                    i++;
                }
                else if((contant[i])=='+')
                {
                    mul =  1;
                    i++;
                }
                else mul=1;/*no sign means positive number*/
                /*calculating the number*/
                while (IsNumber(contant[i+j]))/*counting digits*/
                {
                    j++;
                }
                for (;j>0;j--)
                {
                    number=number+charToDecimal(contant[i])*pow(10,j-1);
                    i++;
                }
                number=number*mul;
                CodeImage[codeImageCounter].Word.I.immed=number;
                signCounter++;
            }
        }
    }/*End of for loop*/
/*End of opcode 19-24*/
}
CodeImage[codeImageCounter].type=2;/*I type*/
codeImageCounter++;
}
/*A method for processing an J command*/
void Jcommand(int opcode,char contant[],int lineNumber)
{
int i=0,z=0;
char lable[32];
i=0,z=0;
CodeImage[codeImageCounter].WordIC=IC;
CodeImage[codeImageCounter].Word.J.opcode=opcode;
/*skips blanks*/
while ((contant[i]==' ')||(contant[i]=='\t'))
{
    i++;
}
if(((IsHighcase(contant[i])==0)&&(IsLowcase(contant[i])==0)&&(contant[i]!='$'))&&((opcode==30)))
{
    printf("ERROR - not a valid lable or register in jmp command - line number %d (command contant is - %s)\n",lineNumber,contant);
    Error=1;
}
if(((IsHighcase(contant[i])==0)&&(IsLowcase(contant[i])==0))&&((opcode==31)||(opcode==32)))
{
    printf("ERROR - not a valid lable in la or call command - line number %d (command contant is - %s)\n",lineNumber,contant);
    Error=1;
}

if (opcode==30)
{
    int number[2];
     if  (contant[i]=='$') /*register*/
     {
                CodeImage[codeImageCounter].Word.J.reg=1;
                i++;
                if (!(IsNumber(contant[i])))/*there isnt a number after $*/
                {
                printf("ERROR - not a number after '$' sign in R command - line number %d (command contant is - %s)\n",lineNumber,contant);
                Error=1;
                }
                else /*valid register*/
                {
                    number[0]=contant[i];
                    if(IsNumber(contant[i+1]))/*two digits reg*/
                    {
                        number[1]=contant[i+1];
                        if (((charToDecimal(number[1])+charToDecimal(number[0])*10))>31)
                        {
                        printf("ERROR - register number is not valid (higher than 31) - line number %d (register number is - %d)\n",lineNumber,(charToDecimal(number[1])+charToDecimal(number[0])*10));
                        Error=1;
                        }
                        else
                        {
                            CodeImage[codeImageCounter].Word.J.address=(charToDecimal(number[1])+(charToDecimal(number[0])*10));
                        }
                    }


                    else/*single digit register*/
                    {
                            CodeImage[codeImageCounter].Word.J.address=charToDecimal(number[0]);
                    }


                }/*End of valid reg*/
        }/*End of current '$/*/

     else /*lable*/
     {
        for (z=0;((contant[i]!='\0')&&(contant[i]!=' ')&&(contant[i]!='\n'));)/*finding lable*/
            {
               lable[z]=contant[i];
               i++;
               z++;
            }
        CodeImage[codeImageCounter].Word.J.address=AdressOfLable(lable,lineNumber);
        CodeImage[codeImageCounter].Word.J.reg=0;
        CodeImage[codeImageCounter].Word.J.opcode=opcode;
     }
}
if ((opcode==31)||(opcode==32))
{
    for (z=0;((contant[i]!='\0')&&(contant[i]!=' ')&&(contant[i]!='\n'));)/*finding lable*/
            {
               lable[z]=contant[i];
               i++;
               z++;
            }
    CodeImage[codeImageCounter].Word.J.address=AdressOfLable(lable,lineNumber);
    CodeImage[codeImageCounter].Word.J.reg=0;
    CodeImage[codeImageCounter].Word.J.opcode=opcode;
}
if(opcode==63)
{
    CodeImage[codeImageCounter].Word.J.address=0;
    CodeImage[codeImageCounter].Word.J.reg=0;
    CodeImage[codeImageCounter].Word.J.opcode=opcode;
}
CodeImage[codeImageCounter].type=3;/*J type*/
codeImageCounter++;
}
/*A method for that decides what is the command in current line and sends corresponding funct,opcode,command contant and line number for potential Error msgs */
void command(char commandName[],char Contant[],int lineNumber)
{
   /*R COMMANDS*/
   if (strcmp(commandName,"add")==0) Rcommand(1,0,Contant,lineNumber);
   if (strcmp(commandName,"sub")==0) Rcommand(2,0,Contant,lineNumber);
   if (strcmp(commandName,"and")==0) Rcommand(3,0,Contant,lineNumber);
   if (strcmp(commandName,"or")==0)  Rcommand(4,0,Contant,lineNumber);
   if (strcmp(commandName,"nor")==0) Rcommand(5,0,Contant,lineNumber);
   if (strcmp(commandName,"move")==0)Rcommand(1,1,Contant,lineNumber);
   if (strcmp(commandName,"mvhi")==0)Rcommand(2,1,Contant,lineNumber);
   if (strcmp(commandName,"mvlo")==0)Rcommand(3,1,Contant,lineNumber);
   /*I COMMANDS*/
   if (strcmp(commandName,"addi")==0)Icommand(10,Contant,lineNumber);
   if (strcmp(commandName,"subi")==0)Icommand(11,Contant,lineNumber);
   if (strcmp(commandName,"andi")==0)Icommand(12,Contant,lineNumber);
   if (strcmp(commandName,"ori")==0) Icommand(13,Contant,lineNumber);
   if (strcmp(commandName,"nori")==0)Icommand(14,Contant,lineNumber);
   if (strcmp(commandName,"bne")==0) Icommand(15,Contant,lineNumber);
   if (strcmp(commandName,"beq")==0) Icommand(16,Contant,lineNumber);
   if (strcmp(commandName,"blt")==0) Icommand(17,Contant,lineNumber);
   if (strcmp(commandName,"bgt")==0) Icommand(18,Contant,lineNumber);
   if (strcmp(commandName,"lb")==0)  Icommand(19,Contant,lineNumber);
   if (strcmp(commandName,"sb")==0)  Icommand(20,Contant,lineNumber);
   if (strcmp(commandName,"lw")==0)  Icommand(21,Contant,lineNumber);
   if (strcmp(commandName,"sw")==0)  Icommand(22,Contant,lineNumber);
   if (strcmp(commandName,"lh")==0)  Icommand(23,Contant,lineNumber);
   if ( strcmp(commandName,"sh")==0) Icommand(24,Contant,lineNumber);
   /*J COMMANDS*/
   if (strcmp(commandName,"jmp")==0) Jcommand(30,Contant,lineNumber);
   if (strcmp(commandName,"la")==0)  Jcommand(31,Contant,lineNumber);
   if (strcmp(commandName,"call")==0)Jcommand(32,Contant,lineNumber);
   if (strcmp(commandName,"stop")==0)Jcommand(63,Contant,lineNumber);

}




                                                                        /*SYMBOL TABLE METHODS*/




/*A method for inserting a symbol to a symbol table linked list*/
void insert_last(symbol start,char newName[],int newValue,int newAttribute,int isEntry,int isExternal)
{

	symbol *ptr;
	if (start.link == NULL)
	{
	    ptr = &start;
	    ptr->link = (struct symbol *)malloc(sizeof(struct symbol));
	    ptr=ptr->link;

	    /*node update*/
	    strcpy(ptr->name, newName);
		ptr->value = newValue;
		ptr->attribute=newAttribute;
		ptr->Entry=isEntry;
		ptr->External=isExternal;
		ptr->link = NULL;
		start.link=ptr;
    }

	else
	{
		ptr = &start;
		while (ptr->link != NULL)
        ptr = ptr->link;
		ptr->link = (struct symbol *)malloc(sizeof(struct symbol));
		ptr = ptr->link;
		/*last node update*/
        strcpy(ptr->name, newName);
		ptr->value = newValue;
		ptr->attribute=newAttribute;
		ptr->Entry=isEntry;
		ptr->External=isExternal;
		ptr->link = NULL;
		start.link=ptr;

		ptr->link = NULL;

	}
}

/*A method for making and filling "ent" file if needed*/
void EntryFile(symbol start)
{
    char entFileName[MAX_FILE_NAME_SIZE];
    symbol *ptr;
	ptr=&start;
	strcpy(entFileName,fileName);
	strcat(entFileName,".ent");
	while (ptr != NULL)
	{
		if ((ptr->Entry)==1)
		{
            if (entOpened==0)
            {
                        entFilePointer = fopen(entFileName,"w");
                        if (entFilePointer == NULL)
                        {
                            printf("Error in opening the entry output file\n");
                            exit(1);
                        }
                        entOpened=1;
            }
            fprintf(entFilePointer,"%s 0%d\n",ptr->name,ptr->value);
            ptr = ptr->link;
		}
		else
		{
			ptr = ptr->link;
		}
    }
    if (fclose(entFilePointer) == EOF)
    {
        printf("Error in closing the Input file\n");
    }
}
/*A method for searching a symbol table for certain lable.returns 1 if the lable exists in the table,0 if not*/
int search_Symbol_Table(char newName[])
{
	int found;
	symbol *ptr;
	ptr=&start;
	found=0;
	while ((ptr != NULL)&&(found!=1))
	{

		if (strcmp(newName,ptr->name)==0)
		{
			found = 1;
		}
		else
		{
			ptr = ptr->link;
		}
	}

	return found;
}

/*A method for checking if a certain lable is an entry.returns 1 if the lable is an entry ,0 if not */
int EntryOfLable(char lableName[])
{
    symbol *ptr;
	ptr=&start;
	while (ptr != NULL)
	{
		if (strcmp(lableName,ptr->name)==0)
		{
			return ptr->Entry;
		}
		else
		{
			ptr = ptr->link;
		}
    	}
    	return -1;
}
/*A method for setting a certain lable as an entry*/
void set_symbol_entry(char symbolName[])
{
    symbol *ptr;
	ptr=&start;
	while (ptr != NULL)
	{

		if (strcmp(symbolName,ptr->name)==0)
		{
			ptr->Entry=1;
			ptr=NULL;
		}
		else
		{
			ptr = ptr->link;
		}
    }


}
/*A method for setting a lable as code lable or data lable*/
void set_symbol_attribute(char symbolName[],int newAttribute)
{
    symbol *ptr;
	ptr=&start;
	while (ptr != NULL)
	{
		if (strcmp(symbolName,ptr->name)==0)
		{
			ptr->attribute=newAttribute;
            ptr=NULL;

		}
		else
		{
			ptr = ptr->link;
		}
    }
}
/*A method for setting a certain lable's adress(value) with a new adress*/
void set_symbol_value(char symbolName[],int newValue)
{
    symbol *ptr;
	ptr=&start;
	while (ptr != NULL)
	{
		if (strcmp(symbolName,ptr->name)==0)
		{
			ptr->value=newValue;
            ptr=NULL;
		}
		else
		{
			ptr = ptr->link;
		}
    }
}
/*A method for updating data words with a new DC*/
void DataImageUpdate(int value)
{
    int i=0;
    while (i<DATA_IMAGE_SIZE)
    {
        if (DataImage[dataImageCounter].WordDC==value)
        {
            DataImage[dataImageCounter].WordDC=value+ICF;
        }
        i++;
    }
}
/*A method for updeting a data lable with a new adress(value) corresponding with code image size */
/*also updetes its new adress in the code image for later processing*/
void SymbolTableUpdate()
{
    symbol *ptr;
	ptr=&start;
	while (ptr != NULL)
	{
		if (ptr->attribute==DATA)
		{
            DataImageUpdate(ptr->value);
			ptr->value=(ptr->value)+ICF;
			ptr = ptr->link;
		}
		else
		{
			ptr = ptr->link;
		}
    }
}

/*A method for checking if a certain lable is external .returns 1 if the lable is an external ,0 if not */
int isExternal(char lableName[],int linenumber)
{
    symbol *ptr;
	ptr=&start;
	while (ptr != NULL)
	{
		if (strcmp(lableName,ptr->name)==0)
		{
			return ptr->External;
		}
		else
		{
			ptr = ptr->link;
		}
    }
    return -1;
}


/*A method for parsing one line from the input file at a time*/
int LineParsing(char line[],int lineNumber)
{
char symbolName[32],dataInstruction[7],contant[78],commandName[5];
int z,i,isLable,emptysymbol,dataInstructionIsValid;
z=0,i=0,isLable=0,emptysymbol=1,dataInstructionIsValid=0;

memset(symbolName,0,32);
memset(dataInstruction,0,7);
memset(contant,0,78);
memset(commandName,0,5);
while ((line[i]==' ')||(line[i]=='\t')) /*ignoring all spaces or tabs from the beginning of the line*/
{
i=i+1;
}
while (IsNumber(line[i]))
{
printf("ERROR - no line can start with a number. line number - %d.\n",lineNumber);
Error=1;
i++;
}

/*blank line or comment line*/
if (line[i]=='\n') return 0;/*blank line*/
if (line[i]==';') return 0;/*if the first character in line who is not a white character is ; its a comment line thus skip the line*/


/*not a blank or comment santance*/
for (z=0;((line[i]!=':')&&(line[i]!='.')&&(line[i]!='\0')&&(line[i]!=' '));)/*inserting lable name/command into symbol name untill :/./space/end of line*/
{
   emptysymbol=0;/*not an empty symbol*/
   symbolName[z]=line[i];
   i++;
   z++;
}
       if((isR(symbolName))==1||(isI(symbolName))==1||(isJ(symbolName))==1)/*certain command is spotted no lable or data*/
           {
                for (z=0;(line[i]!='\0');)
                {
                    contant[z]=line[i];
                    i++;
                    z++;
                }
                command(symbolName,contant,lineNumber);
                IC=IC+4;
           }

if (line[i]==':')/*lable*/
{
    isLable=1;
    if (z>31)
    {
        printf("ERROR - lable langth cannot exceed 31 characters on line number %d (lable langth is %d)\n",lineNumber,z);
        Error=1;
    }
    if (!IsHighcase(symbolName[0])&&(!IsLowcase(symbolName[0])))
    {
        printf("ERROR - first character in a lable have to be big/small case alphabetic character - line number %d (first character is %c)\n",lineNumber,symbolName[0]);
        Error=1;
    }
    if ((symbolName[z-1]==' ')||(symbolName[z-1]=='\t'))
    {
        printf("ERROR - end of lable has to be connected with ':' - on line number %d \n",lineNumber);
        Error=1;
    }
    if (isI(symbolName)||isR(symbolName)||isJ(symbolName)||(strcmp(symbolName,"dh")==0)||(strcmp(symbolName,"dw")==0)||(strcmp(symbolName,"db")==0)||
        (strcmp(symbolName,"asciz")==0)||(strcmp(symbolName,"entry")==0)||(strcmp(symbolName,"extern")==0))
    {
        printf("ERROR - symbol name can not be a saved word in assembly - line number %d (symbol name is %s)\n",lineNumber,symbolName);
        Error=1;
    }
    i++;/*move from :*/
}/*End of lable*/

if ((line[i]=='.')&&(isLable==0)&&(emptysymbol==0))/*if there are characters before '.' which are not part of lable its an error*/
{
        printf("ERROR - there are characters before '.' which are not part of a lable - line number %d (characters are %s)\n",lineNumber,symbolName);
        Error=1;
}
/*after lable detaction skip all blanks*/
 while ((line[i]==' ')||(line[i]=='\t'))
 {
    i=i+1;
 }

/*DATA SANTENCE WITH OR WITHOUT A LABLE*/
if (line[i]=='.')/*data sentence*/
{
    i++;
    /*finding data instruction type*/
    for (z=0;((line[i]!=' ')&&(line[i]!='\0')&&(i<81)&&(z<7)&&(strcmp(dataInstruction,"dh")!=0)&&(strcmp(dataInstruction,"dw")!=0)&&(strcmp(dataInstruction,"db")!=0)&&(strcmp(dataInstruction,"asciz")!=0)
         &&(strcmp(dataInstruction,"entry")!=0)&&(strcmp(dataInstruction,"extern")!=0)     );)
        {
           dataInstruction[z]=line[i];
           i++;
           z++;

        if((strcmp(dataInstruction,"dh")==0)||(strcmp(dataInstruction,"dw")==0)||(strcmp(dataInstruction,"db")==0))
        {
            dataInstructionIsValid=1;
            for(z=0;(line[i]!='\0')&&(line[i]!='\n');)
            {

                while ((line[i]==' ')||(line[i]=='\t'))/*makes sure data contant is now with no space or tabs for later validation*/
                {
                    i=i+1;
                }
                if ((!(IsNumber(line[i])))&&(line[i]!='+')&&(line[i]!='-')&&(z==0))
                {
                     printf("ERROR - first character in a data contant must be a number - line number %d (first character is %c)\n",lineNumber,line[i]);
                     Error=1;
                }
                contant[z]=line[i];
                z++;
                i++;
            }
            contant[z-1]='\0';
            contantCheck(contant,lineNumber,dataInstruction);
        }



        else if((strcmp(dataInstruction,"asciz")==0))
        {
            dataInstructionIsValid=1;
                while ((line[i]!='"')&&(line[i]!='\n'))
                {
                    if ((line[i]==' ')||(line[i]=='\t'))/*skips blanks*/
                    {
                        i++;
                    }
                    else if ((line[i]!='"')&&(line[i]!='\n'))/*invalid character before '"'*/
                    {
                     printf("ERROR - invalid character before start of string - line number %d ( character is %c)\n",lineNumber,line[i]);
                     Error=1;
                        i++;
                    }
                }

                if(line[i]=='\0')
                {
                 printf("ERROR - no opening quotation mark in asciz command - line number %d\n",lineNumber);
                 Error=1;
                }
                if(line[i]=='"')
                {
                    i++;
                    /*reading the string*/
                    for(;(line[i]!='\0')&&(line[i]!='"');)/*copies string into data contant*/
                    {
                        DataImage[dataImageCounter].WordDC=DC;
                        DataImage[dataImageCounter].dataType.asciiChar.value=(int)(line[i]);
                        DataImage[dataImageCounter].type=1;/*ascii*/
                        items=items+1;
                        i++;
                        dataImageCounter++;
                    }
                    DataImage[dataImageCounter].WordDC=DC;
                    DataImage[dataImageCounter].dataType.asciiChar.value=0;/*\0*/
                    DataImage[dataImageCounter].type=1;
                    items=items+1;
                    dataImageCounter++;
                    if (line[i]=='\0')
                    {
                     printf("ERROR - no closing quotation mark in asciz command(or line is too long) - line number %d\n",lineNumber);
                     Error=1;
                    }
                }
        }


        else if((strcmp(dataInstruction,"entry")==0))
        {
                dataInstructionIsValid=1;
                while ((line[i]==' ')||(line[i]=='\t'))/*skips blanks*/
                {
                        i++;
                }
                for(z=0;(line[i]!='\0')&&(line[i]!='\n');)
                {
                    contant[z]=line[i];
                    i++;
                    z++;
                }
                contant[z-1]='\0';
                if((Error==0)&&(search_Symbol_Table(contant)==1))
                {
                    set_symbol_entry(contant);/*if lable exists in symbol table set it as entry*/
                }
        }

        else if((strcmp(dataInstruction,"extern")==0))
        {
            dataInstructionIsValid=1;
            while ((line[i]==' ')||(line[i]=='\t'))/*skips blanks*/
            {
                    i++;
            }
            for(z=0;(line[i]!=' ')&&(line[i]!='\n')&&(line[i]!='\t')&&(line[i]!='\0');)
            {
                contant[z]=line[i];
                i++;
                z++;
            }
            contant[z-1]='\0';
            if(Error==0)
            {
                insert_last(start,contant,0,0,0,1);
            }
        }
    }/*End of data instruction type loop*/

/*ADDING DATA SYMBOL TO SYMBOL TABLE*/
if (Error==0)
{


    if(dataInstructionIsValid==0)
    {
    printf("ERROR - data instruction is invalid - line number %d (data instruction is - .%s)\n",lineNumber,dataInstruction);
    Error=1;
    }
    else if (isLable==1)/*valid data instruction after lable - add to symbol table*/
    {
        if (search_Symbol_Table(symbolName)==1)
        {
            if (EntryOfLable(symbolName)==0)/*Exist in symbol table as data and not entry*/
            {
            printf("ERROR - lable already exists in symbol table - line number %d (lable name is - %s)\n",lineNumber,symbolName);
            Error=1;
            }
            else /*Exist in symbol table as entry - set as code aswell*/
            set_symbol_attribute(symbolName,DATA);
            set_symbol_value(symbolName,DC);
        }
        else
        insert_last(start,symbolName,DC,DATA,0,0);
    }
}
DC=DC+items;
items=0;
}/*End of data case*/

else /*checking if there is command after lable*/
{
         for (z=0;(line[i]!='\0');)
         {

            commandName[z]=line[i];
            i++;
            z++;
            if  ((strcmp(commandName,"add")==0||strcmp(commandName,"sub")==0||strcmp(commandName,"and")==0||strcmp(commandName,"or")==0||strcmp(commandName,"nor")==0)
                &&(line[i]=='i')) commandName[z]='i';
            if  ((isR(commandName))==1||(isI(commandName))==1||(isJ(commandName))==1)
                {
                i++;
                for (z=0;(line[i]!='\0');)
                {
                    contant[z]=line[i];
                    i++;
                    z++;
                }
                command(commandName,contant,lineNumber);/*no lable or data instruction deal with command end move to next line*/

                }
          }
/*ADDING CODE SYMBOL TO SYMBOL TABLE*/
if (Error==0)
{
    if (isLable==1)/*valid data instruction after lable - add to symbol table*/
    {
        if (search_Symbol_Table(symbolName)==1)
        {
            if (EntryOfLable(symbolName)==0)/*Exist in symbol table as code and not entry*/
            {
            printf("ERROR - lable already exists in symbol table - line number %d (lable name is - %s)\n",lineNumber,symbolName);
            Error=1;
            }
            else /*Exist in symbol table as entry - set as code aswell*/
            {
            set_symbol_attribute(symbolName,CODE);
            set_symbol_value(symbolName,IC);
            }
        }
        else
        insert_last(start,symbolName,IC,CODE,0,0);
    }
    if (isLable==1) IC=IC+4;
}
}



/*End of line reading*/
memset(dataInstruction,0,7);
return 0;
}/*End of line parsisng*/

/*A method for completing a certain command if spotted in a certain line via "SecoundLineParsing" method*/
void CommandCompletion(char commandName[],char contant[],int lineNumber)
{
	int i,z,commaCounter,LableExistsInSymbolTable;
	char destinationLable[32];
        codeImageCounter=0;
        i=0,z=0,commaCounter=0,LableExistsInSymbolTable=0;

        if (strcmp(commandName,"jmp")==0||strcmp(commandName,"la")==0||strcmp(commandName,"call")==0)
        {
            while ((contant[i])==' '||(contant[i]=='\t')) /*skipping blanks*/
            {
            i=i+1;
            }
            if (contant[i]!='$')/*assign new address only if destination isnt register*/
            {
                while (codeImageCounter<CODE_IMAGE_SIZE)
                {
                    if (CodeImage[codeImageCounter].WordIC==IC)
                    {
                        while (IsLowcase(contant[i])==1||IsHighcase(contant[i])==1||IsNumber(contant[i])==1)
                        {
                            destinationLable[z]=contant[i];
                            z++;
                            i++;
                        }
                    if (search_Symbol_Table(destinationLable)==1)
                    {
                        LableExistsInSymbolTable=1;
                        CodeImage[codeImageCounter].Word.J.address=AdressOfLable(destinationLable,lineNumber);
                    }
                    codeImageCounter=CODE_IMAGE_SIZE;
                    }
                    else codeImageCounter++;
                }
                if (LableExistsInSymbolTable==0)
                {
                    printf("ERROR - Destination symbol doesn't exists - line number %d (Destination symbol is - %s)\n",lineNumber,destinationLable);
                    Error=1;
                }
                if(isExternal(destinationLable,lineNumber)==1)
                {
                    if (extOpened==0)
                    {
                        char extFileName[MAX_FILE_NAME_SIZE];
                        strcpy(extFileName,fileName);
                        strcat(extFileName,".ext");
                        extFilePointer = fopen(extFileName, "w");
                        if (extFilePointer == NULL)
                        {
                            printf("Error in opening the Output external file\n");
                            exit(1);
                        }
                        extOpened=1;
                    }
                    fprintf(extFilePointer,"%s 0%d\n",destinationLable,IC);
                }
            }
        }










        if (strcmp(commandName,"bne")==0||strcmp(commandName,"beq")==0||strcmp(commandName,"blt")==0||strcmp(commandName,"bgt")==0)
        {
            while (codeImageCounter<CODE_IMAGE_SIZE)
            {

                if (CodeImage[codeImageCounter].WordIC==IC)
                {

                    while(commaCounter!=2)
                    {
                        if (contant[i]==',') commaCounter++;
                        i++;
                    }
                    while ((contant[i])==' '||(contant[i]=='\t')) /*skipping blanks*/
                    {
                    i=i+1;
                    }
                    while (IsLowcase(contant[i])==1||IsHighcase(contant[i])==1||IsNumber(contant[i])==1) /*skipping blanks*/
                    {
                    destinationLable[z]=contant[i];
                    z++;
                    i++;
                    }
                    if (search_Symbol_Table(destinationLable)==1)
                    {
                        LableExistsInSymbolTable=1;
                        CodeImage[codeImageCounter].Word.I.immed=(AdressOfLable(destinationLable,lineNumber)-IC);
                    }

                    codeImageCounter=CODE_IMAGE_SIZE;
                }
                else codeImageCounter++;
            }

            if (isExternal(destinationLable,lineNumber)==1)
            {
                printf("ERROR - Destination symbol is external - line number %d (Destination symbol is - %s)\n",lineNumber,destinationLable);
                Error=1;
            }
            else if (LableExistsInSymbolTable==0)
            {
                printf("ERROR - Destination symbol doesn't exists in symbol table - line number %d (Destination symbol is - %s)\n",lineNumber,destinationLable);
                Error=1;
            }
        }
        memset(destinationLable,0,32);
}


/*A method for going throu a certain line again and completing what could not be complete in the first gothrou (in lineParsing)*/
void SecoundLineParsing(char line[],int lineNumber)
{
char symbolName[32],dataInstruction[7],contant[78],commandName[5];
int z,i;
memset(symbolName,0,32);
memset(dataInstruction,0,7);
memset(contant,0,78);
memset(commandName,0,5);

z=0,i=0;
while ((line[i]==' ')||(line[i]=='\t')) /*ignoring all spaces or tabs from the beginning of the line*/
{
i=i+1;
}
while (IsNumber(line[i]))
{
printf("ERROR - no line can start with a number. line number - %d.\n",lineNumber);
Error=1;
i++;
}

/*blank line or comment line*/
if (line[i]=='\n') exit(0);/*blank line*/
if (line[i]==';') exit(0);/*if the first character in line who is not a white character is ; its a comment line thus skip the line*/


/*not a blank or comment santance*/
for (z=0;((line[i]!=':')&&(line[i]!='.')&&(line[i]!='\0')&&(line[i]!=' '));)/*inserting lable name/command into symbol name untill :/./space/end of line*/
{
   symbolName[z]=line[i];
   i++;
   z++;
}

       if((isR(symbolName))==1||(isI(symbolName))==1||(isJ(symbolName))==1)/*certain command is spotted no lable or data*/
           {
                for (z=0;(line[i]!='\n');)
                {
                    contant[z]=line[i];
                    i++;
                    z++;
                }
                CommandCompletion(symbolName,contant,lineNumber);
                IC=IC+4;
                i=100;/*flag for rest of the method*/
           }
if (i!=100)
{
if (line[i]==':') i++;
/*after lable detaction skip all blanks*/
 while ((line[i]==' ')||(line[i]=='\t'))
 {
    i=i+1;
 }

if (line[i]!='.') /*entry setting and checking if there is command after lable*/
{
         for (z=0;(line[i]!='\0');)
         {
            commandName[z]=line[i];
            i++;
            z++;
         }
            if  ((isR(commandName))==1||(isI(commandName))==1||(isJ(commandName))==1)
                {
                    i++;
                    for (z=0;(line[i]!='\0');)
                    {

                        contant[z]=line[i];
                        i++;
                        z++;
                    }
                }
                CommandCompletion(commandName,contant,lineNumber);
                IC=IC+4;
}
else
{
   for (z=0;(line[i]!=' ');)
         {
            commandName[z]=line[i];
            i++;
            z++;
         }
         commandName[z]='\0';
    if((strcmp(commandName,".entry")==0))
    {
        i++;
        for (z=0;(line[i]!='\0')&&(line[i]!='\n');)
        {

            contant[z]=line[i];
            i++;
            z++;
        }
                contant[z-1]='\0';

                set_symbol_entry(contant);
    }
}
}
}

/*A method for going throu the input file again and completing what could not be complete in the first gothrou (in first gothruo)*/
void SecoundGoThrou()
{
/*variables*/
int lineNumber;
char line[81];
lineNumber=0;
IC=100;
DC=ICF;

    /*Opening text file to read from*/

    InputFilePointer = fopen(assemblyFileName, "r");
    if (InputFilePointer == NULL)
    {
    printf("Error in opening the Input asembly file\n");
    exit(1);
    }
    /*Reading the file*/
    while(fgets(line,81,InputFilePointer)!=NULL)
    /*individual line managmant*/
    {
        lineNumber++;
        SecoundLineParsing(line,lineNumber);/*applying a method for parsing each line*/
    }
if (fclose(InputFilePointer) == EOF)
{
printf("Error in closing the Input file\n");
}
if (extOpened==1)
{
if (fclose(extFilePointer) == EOF)
{
printf("Error in closing the Input file\n");
}
}

}
/*A method for filling a certain part of a data or code line for later convertion to hex output*/
void lineFiller(int line[],int fromIndex,int toIndex,int number)
{
        int c, k,zeros;
        zeros=(toIndex-fromIndex);
        for (c = zeros; c >= 0; c--)
        {

            k = number >> c;
            if (k & 1)
              bitsLine[fromIndex]=1;
            else
              bitsLine[fromIndex]=0;

              fromIndex++;
        }
}
/*A method for converting a full code or data word to hex, and Outputing it to "obj" file*/
void fullLineBinaryToHex(int line [])
{
    int Hexes[8];
    int z,lineCounter;
    z=0;
    if ((k%8)==0) fprintf(obFilePointer,"\n0%d ",IC);
    for (lineCounter=0;lineCounter<29;)
    {
        Hexes[z]=line[lineCounter+3]+line[lineCounter+2]*2+line[lineCounter+1]*4+line[lineCounter]*8;
        z++;
        lineCounter=lineCounter+4;
    }
    fprintf(obFilePointer,"%X%X ",Hexes[6],Hexes[7]);
    IC++;
    k=k+2;
    if ((k%8)==0) fprintf(obFilePointer,"\n0%d ",IC);
    fprintf(obFilePointer,"%X%X ",Hexes[4],Hexes[5]);
    IC++;
    k=k+2;
    if ((k%8)==0) fprintf(obFilePointer,"\n0%d ",IC);
    fprintf(obFilePointer,"%X%X ",Hexes[2],Hexes[3]);
    IC++;
    k=k+2;
    if ((k%8)==0) fprintf(obFilePointer,"\n0%d ",IC);
    fprintf(obFilePointer,"%X%X ",Hexes[0],Hexes[1]);
    IC++;
    k=k+2;

}
/*A method for converting a half code or data word to hex, and Outputing it to "obj" file*/
void halfLineBinaryToHex(int line [])
{
    int Hexes[4];
    int i,z,lineCounter;
    z=0;
    if ((k%8)==0) fprintf(obFilePointer,"\n0%d ",IC);
    for (lineCounter=0;lineCounter<13;i--)
    {
        Hexes[z]=line[lineCounter+3]+line[lineCounter+2]*2+line[lineCounter+1]*4+line[lineCounter]*8;
        z++;
        lineCounter=lineCounter+4;
    }
    fprintf(obFilePointer,"%X%X ",Hexes[2],Hexes[3]);
    IC++;
    k=k+2;
    if ((k%8)==0) fprintf(obFilePointer,"\n0%d ",IC);
    fprintf(obFilePointer,"%X%X ",Hexes[0],Hexes[1]);
    IC++;
    k=k+2;
}
/*A method for converting a single byte data to hex, and Outputing it to "obj" file*/
void dataBinaryToHex(int line [])
{
        int Hexes[2];
        int i,z,lineCounter;
        z=0,lineCounter=0;
        if ((k%8)==0) fprintf(obFilePointer,"\n0%d ",IC);
        for (i=0;i<2;i++)
        {
            Hexes[z]=line[lineCounter+3]+line[lineCounter+2]*2+line[lineCounter+1]*4+line[lineCounter]*8;
            z++;
            lineCounter=lineCounter+4;
            k++;
        }
        fprintf(obFilePointer,"%X%X ",Hexes[0],Hexes[1]);
        IC++;
}
/*A method for Outputing an "obj" file for a certain "as" Input file*/
void Object()
{
	    int codeImageCounter;
            char objFileName[MAX_FILE_NAME_SIZE];
            strcpy(objFileName,fileName);
            strcat(objFileName,".ob");
            obFilePointer = fopen(objFileName, "w");
            if (obFilePointer == NULL)
            {
                printf("Error in opening the object output file file\n");
                exit(1);
            }
            IC=100;
            codeImageCounter=0;
            dataImageCounter=0;
            fprintf(obFilePointer,"     %d %d",ICF-100,DCF);
            while (IC<ICF)
            {
                if (CodeImage[codeImageCounter].WordIC==IC)
                {
                    /*1 for R,2 for I,3 for J*/
                    if((CodeImage[codeImageCounter].type)==1)
                    {
                        lineFiller(bitsLine,0,5,CodeImage[codeImageCounter].Word.R.opcode);
                        lineFiller(bitsLine,6,10,CodeImage[codeImageCounter].Word.R.rs);
                        lineFiller(bitsLine,11,15,CodeImage[codeImageCounter].Word.R.rt);
                        lineFiller(bitsLine,16,20,CodeImage[codeImageCounter].Word.R.rd);
                        lineFiller(bitsLine,21,25,CodeImage[codeImageCounter].Word.R.funct);
                        lineFiller(bitsLine,26,31,CodeImage[codeImageCounter].Word.R.notInUse);
                    }
                    if((CodeImage[codeImageCounter].type)==2)
                    {
                        lineFiller(bitsLine,0,5,CodeImage[codeImageCounter].Word.I.opcode);
                        lineFiller(bitsLine,6,10,CodeImage[codeImageCounter].Word.I.rs);
                        lineFiller(bitsLine,11,15,CodeImage[codeImageCounter].Word.I.rt);
                        lineFiller(bitsLine,16,31,CodeImage[codeImageCounter].Word.I.immed);
                    }
                    if((CodeImage[codeImageCounter].type)==3)
                    {
                        lineFiller(bitsLine,0,5,CodeImage[codeImageCounter].Word.J.opcode);
                        lineFiller(bitsLine,6,6,CodeImage[codeImageCounter].Word.J.reg);
                        lineFiller(bitsLine,7,31,CodeImage[codeImageCounter].Word.J.address);
                    }
                    fullLineBinaryToHex(bitsLine);
                }
                codeImageCounter++;
            }
            while (IC<DCF+ICF)
            {
                        if((DataImage[dataImageCounter].type)==1)/*ascii*/
                        {
                            lineFiller(bitsLine,0,7,DataImage[dataImageCounter].dataType.asciiChar.value);
                            dataBinaryToHex(bitsLine);
                        }
                        if((DataImage[dataImageCounter].type)==2)/*db*/
                        {
                            lineFiller(bitsLine,0,7,DataImage[dataImageCounter].dataType.db.value);
                            dataBinaryToHex(bitsLine);
                        }
                        if((DataImage[dataImageCounter].type)==3)/*dh*/
                        {
                            lineFiller(bitsLine,0,15,DataImage[dataImageCounter].dataType.dh.value);
                            halfLineBinaryToHex(bitsLine);
                        }
                        if((DataImage[dataImageCounter].type)==4)/*dw*/
                        {
                            lineFiller(bitsLine,0,31,DataImage[dataImageCounter].dataType.dw.value);
                            fullLineBinaryToHex(bitsLine);
                        }
                        dataImageCounter++;
                    }
                    if (fclose(obFilePointer) == EOF)
                    {
                        printf("Error in closing the ob Output file\n");
                    }
            }

/*A method for going throu the input file and to start filling what can be filled in the code and data image*/
/*And for making a symbol table so we can use it to complete filling the code and data images ,and execute outputs*/
void FirstGoThrou()
{
/*variables*/
int lineNumber;
char line[81];
lineNumber=0;


strcpy(assemblyFileName,fileName);
strcat(assemblyFileName,".as");
/*Opening text file to read from*/
InputFilePointer = fopen(assemblyFileName, "r");
if (InputFilePointer == NULL)
{
printf("Error in opening the Input assemblyy file\n");
exit(1);
}

/*Reading the file*/
while(fgets(line,81,InputFilePointer)!=NULL)
{
    lineNumber++;
    LineParsing(line,lineNumber);/*individual line parsing*/
}

if (fclose(InputFilePointer) == EOF)
{
printf("Error in closing the Input file\n");
}
}
/*main method*/
int main (int argc, char *argv[])
{
int i;
i=1;
start.link=&nextt;
DC=0;
IC=100;
Error=0;
codeImageCounter=0;
k=0;
extOpened=0;
entOpened=0;
while(i!=argc)
{

    if (Error==0)
    {
        strncpy(fileName,argv[i],strlen(argv[i])-3);/*removes ".as" from as file name for useing files name*/
        FirstGoThrou();
        ICF=IC;
        DCF=DC;
        SymbolTableUpdate();
        SecoundGoThrou();
        if (Error==0)/*Output*/
        {
            Object();
            EntryFile(start);
        }
    }
    i++;
    /*resetting veriables*/
    start.link=&nextt;
    nextt.link=NULL;
    DC=0;
    IC=100;
    Error=0;
    codeImageCounter=0;
    k=0;
    extOpened=0;
    entOpened=0;

}
return 0;
}












