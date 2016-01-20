// log.h; the header file which defines Log(); and LogErr();
#include <stdio.h>

extern FILE *file;

void initLogFile (FILE *inputFile);
void writeMessage (char input[],FILE *inputFile);
void closeFile (FILE *inputFile);