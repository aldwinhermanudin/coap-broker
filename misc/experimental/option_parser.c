/* strchr example */
#include <stdio.h>
#include <string.h>


int optionValidation(char* source){ 
  char * pch;
  int counter = 0;
  pch=strchr(source,'=');
  while (pch!=NULL)
  {
    counter++;
    pch=strchr(pch+1,'=');
  }
	return counter;
}

int getOption(char * source, char* type, char* data){
 
		if (optionValidation(source) == 1){
			
			int counter = strcspn(source, "=");
			char* pch = strchr(source,'=');
			if (pch != NULL){
				strncpy(type,source,counter);
				type[counter] = '\0';
				
				if(pch[1] == '"')
					strcpy(data,pch+2);
				else
					strcpy(data,pch+1);
					
				if(pch[strlen(pch+1)] == '"')
					data[strlen(data)-1] = '\0';
					
				return 1;
			}
		}
		return 0;
}

int main ()
{
  char str[] = "rt=lalaasd";
  char opt_type[32], opt_data[32];
  getOption(str, opt_type,opt_data);
  printf ("type => %s\n",opt_type);
  printf ("data => %s\n",opt_data);
  return 0;
}
