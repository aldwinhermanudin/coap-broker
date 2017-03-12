#include <stdio.h>
#include <stdlib.h>
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

int calOptionSize(char* source, int* type, int* data){
	
	if (optionValidation(source) == 1){
		*type = (int) strcspn(source, "=");
		char* pch = strchr(source,'=')+1;
		*data = (int) strlen(pch);
		return 1;			
	}
	return 0;
} 

int parseOption(char * source, char* type, char* data){
 
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
 
int calPathSize(char* source){
	
	if (source[0] == '<' && source[1] != '/' &&source[strlen(source)-1] == '>' ){
		int path_size = strlen(source)-2;
		return path_size;
	}
	return -1;
}
int parsePath(char* source, char* path){
	
	if (source[0] == '<' && source[1] != '/' &&source[strlen(source)-1] == '>' ){
		int path_size = strlen(source)-2;
		strncpy(path, source+1, path_size);
		path[path_size] = '\0';
		return 1;
	}
	return 0;
}

int registerLinkFormat(char* str){
	  char * pch;
	  int last_counter = 0;
	  int counter = 0;
	  
	  pch=strchr(str,';');
	  while (pch!=NULL)
	  {
		counter = pch-str;
		
		int size = counter-last_counter;
		char* temp_str;
		
		// first element of link format
		if(str[last_counter] != ';'){
			temp_str = malloc(sizeof(char) * (size+1));
			strncpy(temp_str, str+last_counter, size);
			temp_str[size] = '\0'; 
			  char* path = malloc(sizeof(char) * (calPathSize(temp_str)+1)); 
			  parsePath(temp_str, path);
			  printf ("type => %s\n",path); 
			  free(path); 
			printf("%s\n", temp_str);
			free(temp_str);
		}
		
		// in between first and last element of link format
		else{
			temp_str = malloc(sizeof(char) * (size));
			strncpy(temp_str, str+last_counter+1, size-1);
			temp_str[size-1] = '\0';
			  int type_size, data_size;
			  calOptionSize(temp_str,&type_size, &data_size);
			  char* opt_type = malloc(sizeof(char) * (type_size+1));
			  char* opt_data = malloc(sizeof(char) * (data_size+1));
			  parseOption(temp_str, opt_type,opt_data);
			  printf ("type => %s\n",opt_type);
			  printf ("data => %s\n",opt_data);
			  free(opt_type);free(opt_data);
			printf("%s\n", temp_str);
			free(temp_str);
		}
		
		// last element of link format
		pch=strchr(pch+1,';');
		if (pch == NULL){
			size = strlen(str+counter+1);
			temp_str = malloc(sizeof(char) * (size+1));
			strncpy(temp_str, str+counter+1, size);
			temp_str[size] = '\0';
			  int type_size, data_size;
			  calOptionSize(temp_str,&type_size, &data_size);
			  char* opt_type = malloc(sizeof(char) * (type_size+1));
			  char* opt_data = malloc(sizeof(char) * (data_size+1));
			  parseOption(temp_str, opt_type,opt_data);
			  printf ("type => %s\n",opt_type);
			  printf ("data => %s\n",opt_data);
			  free(opt_type);free(opt_data);
			printf("%s\n", temp_str);
			free(temp_str); 
		}
				
		last_counter = counter;
	  }
	  return 1;
}

int main ()
{
	
	char str[] ="<sensors/temp>;qwrt=\"temperature-c\";if=\"sensor\";ct=40;rt=\"sensor\"";
	char str2[] ="<sensors/temp>;if=\"sensor\";ct=40;rt=\"sensor\"";
	registerLinkFormat(str);
	
	
  return 0;
}
