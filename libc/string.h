#ifndef STRING_H
#define STRING_H

void int_to_ascii(int n, char str[]);
void reverse(char s[]);
int strlen(char s[]);
void backspace(char s[]);
void append(char s[], char n);
int strcmp(char s1[], char s2[]);
int strcpy(char *dest, char *src);
void int2hex(int val, char hex[]);

#endif