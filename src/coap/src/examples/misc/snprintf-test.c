/* strchr example */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main ()
{
  char str[] = "Aldwin Akbar Hermanudin";
  char* test = malloc(sizeof(char) *(strlen(str)+2));
  printf("Number of char detected : %ld\n",sizeof(char) *(strlen(str)));
  printf("allocating memory of %ld\n",sizeof(char) *(strlen(str)+2));
  snprintf(test,strlen(str)+1,"%s",str);
  printf("Number of char detected : %ld\n",sizeof(char) *(strlen(test)));
  printf("%s\n", test);
  free(test);
  return 0;
}
