#ifndef STRING_H
#define STRING_H

void append(char s[], char n);
void backspace(char s[]);

void itoa(int n, char str[]);
void itoh(int val, char hex[]);

int strlen(char s[]);
int strcmp(char s1[], char s2[]);
char * strcpy(char *dest, char *src);

#endif
