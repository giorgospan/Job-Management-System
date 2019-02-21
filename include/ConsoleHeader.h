#ifndef CONSOLE_HEADER_H
#define CONSOLE_HEADER_H



/*Communication between console <---> coordinator*/
int console_communication(FILE*,int,int);


/*Reading operations from stdin*/
void scan_operation(char*);

#endif