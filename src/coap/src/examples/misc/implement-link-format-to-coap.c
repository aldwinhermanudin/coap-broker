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

int optionRegister(char* temp_str){
	int type_size, data_size;
	calOptionSize(temp_str,&type_size, &data_size);
	char* opt_type = malloc(sizeof(char) * (type_size+1));
	char* opt_data = malloc(sizeof(char) * (data_size+1));
		int status = parseOption(temp_str, opt_type,opt_data);
	printf ("type => %s\n",opt_type);
	printf ("data => %s\n",opt_data);
	free(opt_type);free(opt_data);
	printf("%s\n", temp_str);
	return status;
}

int pathRegister(char* temp_str){
	char* path = malloc(sizeof(char) * (calPathSize(temp_str)+1)); 
		int status = parsePath(temp_str, path);
	printf ("path => %s\n",path); 
	free(path); 
	return status;
}

int parseLinkFormat(char* str, int (*pathRegisterHandler)(char*),int (*optionRegisterHandler)(char*)){
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
			printf("assign memory of %d \n", size);
			temp_str = malloc(sizeof(char) * (size+1));
			strncpy(temp_str, str+last_counter, size);
			temp_str[size] = '\0'; 
				int status = (*pathRegisterHandler)(temp_str);
			free(temp_str);
			printf("freeing memory\n");
			if (!status) return 0;
		}
		
		// in between first and last element of link format
		else{
			printf("assign memory of %d \n", size);
			temp_str = malloc(sizeof(char) * (size));
			strncpy(temp_str, str+last_counter+1, size-1);
			temp_str[size-1] = '\0';
				int status = (*optionRegisterHandler)(temp_str); 
			free(temp_str);
			printf("freeing memory\n");
			if (!status) return 0;
		}
		
		// last element of link format
		pch=strchr(pch+1,';');
		if (pch == NULL){
			size = strlen(str+counter+1);
			printf("assign memory of %d \n", size);
			temp_str = malloc(sizeof(char) * (size+1));
			strncpy(temp_str, str+counter+1, size);
			temp_str[size] = '\0';
				int status = (*optionRegisterHandler)(temp_str); 
			free(temp_str); 
			printf("freeing memory\n");
			if (!status) return 0;
		}
				
		last_counter = counter;
	  }
	  return 1;
}

int main ()
{
	
	char str[] ="<temp/qwq>;lol=wqwq;if=\"sensor\";ct=40;rt=\"sensor\"";
	char str2[] ="<sensors/temp>;if=\"sensor\";ct=40;rt=\"sensor\"";
	parseLinkFormat(str, &pathRegister, &optionRegister);
	char str3[] ="<temp>;if=\"se=nsor\";ct=40;rt=\"sensor\"";
	parseLinkFormat(str3, &pathRegister, &optionRegister);
	char str4[] ="</temp>;if=\"sensor\";ct=40;rt=\"sensor\"";
	parseLinkFormat(str4, &pathRegister, &optionRegister);
	char str5[] ="<temp>;if=\"sensor\";ct=40;rt=\"sensor\"";
	parseLinkFormat(str5, &pathRegister, &optionRegister); 
	
	
  return 0;
}
