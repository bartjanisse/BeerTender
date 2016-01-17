// log.cpp;

#include <stdlib.h>
#include <stdio.h>
//#include "system.h"        // SysShutdown();
#include "log.h"


//-------- initalization ---------

FILE *file;

int main()
{
    initLogFile(file);
    char greeting[] = "Hello output";
    writeMessage(greeting,file);
    closeFile(file);
}

//-------- Functions ---------

void initLogFile(FILE *inputFile)
{
    inputFile = fopen("x.log", "a+"); // a+ (create + append) option will allow appending which is useful in a log file
    if (inputFile == NULL) { /* Something is wrong   */}
    fprintf(inputFile, "Im logging somethig ..\n");
}

void writeMessage(char input[],FILE *inputFile)
{
    inputFile = fopen("x.log", "a+"); // a+ (create + append) option will allow appending which is useful in a log file
    if (inputFile == NULL) { /* Something is wrong   */}
    fprintf(inputFile, input);
}

void closeFile(FILE *inputFile)
{
  if (inputFile == NULL) {printf("%s\n","File not available to close" ); }
    fclose(inputFile);
}
