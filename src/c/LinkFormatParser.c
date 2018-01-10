#include "LinkFormatParser.h"
#include "LibcoapMod.h"

int optionValidation(char* source){ 
  char * pch;
  int counter = 0;
  pch=strchr(source,'=');
  while (pch!=NULL)
  {
    counter++;
    pch=strchr(pch+1,'=');
  }
  if (strlen(source) < 3 || source[0] == '=' || source[strlen(source)-1] == '=') return 0;
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
		
				/*only allow alphanum and '-'*/
				#ifdef RESTRICT_CHAR
				for(int i = 0; i < strlen(type);i++){
					if(!isalnum(type[i]) && type[i] != '-'){
						return 0;
					}
				}
				#endif
				/*only allow alphanum and '-' */				
				
				if(pch[1] == '"')
					strcpy(data,pch+2);
				else
					strcpy(data,pch+1);
					
				if(pch[strlen(pch+1)] == '"')
					data[strlen(data)-1] = '\0';
					
				/*only allow alphanum and '-'*/
				#ifdef RESTRICT_CHAR	
				for(int i = 0; i < strlen(data);i++){
					if(!isalnum(data[i]) && data[i] != '-'){
						return 0;
					}
				}
				#endif				
				/*only allow alphanum and '-'*/						
				
				return 1;
			}
		}
		return 0;
}
 
int calPathSize(char* source){
	
	if (source[0] == '<' && source[1] != '/' && source[strlen(source)-2] != '/' && source[strlen(source)-1] == '>' ){
		int path_size = strlen(source)-2;
		if (path_size > 0) return path_size;
		else return 0;
	}
	return 0;
}
int parsePath(char* source, char* path){
	
	if (calPathSize(source)){ 
		int path_size = strlen(source)-2;
		strncpy(path, source+1, path_size);
		path[path_size] = '\0';
		
		/*only allow alphanum and '/' and '-'*/	
		#ifdef RESTRICT_CHAR
		for(int i = 0; i < path_size;i++){
			if(!isalnum(path[i]) && path[i] != '/' && path[i] != '-'){
				return 0;
			}
		}
		#endif
		/*only allow alphanum and '/' and '-'*/
		 
		return 1;
	}
	return 0;
}

int optionRegister(coap_resource_t **resource , char* temp_str){
	
	int type_size, data_size;
	int upper_status = calOptionSize(temp_str,&type_size, &data_size);
	if (upper_status && type_size > 0 && data_size > 0){
		char* opt_type = malloc(sizeof(char) * (type_size+1));
		char* opt_data = malloc(sizeof(char) * (data_size+1));
		int status = parseOption(temp_str, opt_type,opt_data);
		if(status){
			coap_add_attr(*resource, opt_type, strlen(opt_type), opt_data, strlen(opt_data), COAP_ATTR_FLAGS_RELEASE_NAME | COAP_ATTR_FLAGS_RELEASE_VALUE);
		}
		else {
			free(opt_type);
			free(opt_data);
		}
		// TODO:free resource attr if status error
		return status;
	}
	return 0;
}

int pathRegister(coap_resource_t *old_resource, coap_resource_t **new_resource , char* temp_str){
	
	if (calPathSize(temp_str) > 0){
		int total_size = old_resource->uri.length + calPathSize(temp_str) + 2; // supposed to be 1, but for safety reason I add 1 more byte
		char* rel_path = malloc(sizeof(char) * (calPathSize(temp_str)+1)); 
		int status = parsePath(temp_str, rel_path);
		if(status){
			char* abs_path = malloc(sizeof(char) * (total_size + 1));
			char* parent_path = malloc(sizeof(char) * (old_resource->uri.length + 2));
			snprintf(parent_path,old_resource->uri.length+1, "%s", old_resource->uri.s);
			sprintf(abs_path,"%s/%s", parent_path, rel_path);
			free(parent_path);
			*new_resource = coap_resource_init(abs_path, strlen(abs_path), COAP_RESOURCE_FLAGS_RELEASE_URI); 
		}
		free(rel_path);
		return status;
	}
	return 0;
}

int parseLinkFormat(char* str, coap_resource_t* old_resource, coap_resource_t** resource){ 
	  char * pch;
	  int last_counter = 0;
	  int counter = 0;
	  int master_status = 0;
	  
	  /* Error Checker*/ 
	  if(strchr(str,',') != NULL) return 0;
	  if(str[0] == ';') return 0;
	  /* Error Checker*/
	  
	  pch=strchr(str,';');
	  while (pch!=NULL)
	  {
		counter = pch-str;
		
		int size = counter-last_counter;
		
		// first element of link format
		if(str[last_counter] != ';'){
			
			char* temp_str = malloc(sizeof(char) * (size+1));
			strncpy(temp_str, str+last_counter, size);
			temp_str[size] = '\0'; 
				int status = pathRegister(old_resource, resource,temp_str);
			free(temp_str);
			if (!status) return 0;
			else master_status = 1;
		}
		
		// in between first and last element of link format
		else{
			char* temp_str = malloc(sizeof(char) * (size));
			strncpy(temp_str, str+last_counter+1, size-1);
			temp_str[size-1] = '\0';
				int status = optionRegister(resource,temp_str); 
			//TODO : fix this free(), somehow it works now
			free(temp_str);
			if (!status) {
				coapFreeResource(*resource);
				return 0;
			}
			else master_status = 1;
		}
		
		// last element of link format
		pch=strchr(pch+1,';');
		if (pch == NULL){
			size = strlen(str+counter+1);
			char* temp_str = malloc(sizeof(char) * (size+1));
			strncpy(temp_str, str+counter+1, size);
			temp_str[size] = '\0';
				int status = optionRegister(resource,temp_str); 
			//TODO : fix this free(), somehow it works now
			free(temp_str); 
			if (!status) {
				coapFreeResource(*resource);
				return 0;
			}
			else master_status = 1;
		}
				
		last_counter = counter;
	  }
	  return master_status;
}

int parseOptionURIQuery(char* option_value, unsigned short option_length, char** query_name, char** query_value){
	char* temp_uri_query = malloc(sizeof(char) * (option_length+2));
	snprintf(temp_uri_query, option_length+1, "%s", option_value);
	
	int name_size, value_size;
	int upper_status = calOptionSize(temp_uri_query,&name_size, &value_size);
	if (upper_status && name_size > 0 && value_size > 0){
		*query_name = malloc(sizeof(char) * (name_size+1));
		*query_value = malloc(sizeof(char) * (value_size+1));
		int status = parseOption(temp_uri_query, *query_name, *query_value);
		if(status){
			free(temp_uri_query);
			return 1;
		}
		
		else{
			free(temp_uri_query);
			return 0;
		}
	}
	
	else {
		free(temp_uri_query);
		return -1;
	}
}

void dynamicConcatenate(char **str, char *str2) {
    char *tmp = NULL;

    // Reset *str
    if ( *str != NULL && str2 == NULL ) {
        free(*str);
        *str = NULL;
        return;
    }

	if ( str2 == NULL ) {
		printf("Error Source Empty\n");
        return;
    }

    // Initial copy
    if (*str == NULL) {
        *str = malloc( (strlen(str2)+1) * sizeof(char) );
        strcpy(*str, str2);
    }
    else { // Append
        tmp = malloc ( (strlen(*str)+1 )* sizeof(char) );
        strcpy(tmp, *str);
        *str = (char *) realloc((*str) ,  (strlen(*str)+strlen(str2)+1) * sizeof(char) );
        sprintf(*str, "%s%s", tmp, str2);
        free(tmp);
    }

} 
 
int calculateResourceLF(coap_resource_t* resource){
	if(resource != NULL){
		int lf_size = 0;
		/* </ uri > . added additional 3 char of <,/, and > */ 
		lf_size += resource->uri.length + 3;
		 
		coap_attr_t *attr; 
		LL_FOREACH(resource->link_attr, attr) {
			debug("Attr of %s with value of %s\n", attr->name.s, attr->value.s);
			/* ; name = value . added 2 additional char of ; and = */ 
			lf_size += (attr->name.length) + (attr->value.length) + 2;
		}
		if (resource->observable){
			/* if observable add 4 char of ";obs" */
			lf_size += 4;
		}
		return lf_size;
	}
} 
