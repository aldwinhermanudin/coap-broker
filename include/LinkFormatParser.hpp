#ifndef LF_PARSER_HPP
#define LF_PARSER_HPP
#define RESTRICT_CHAR
/* Link Format Parser starts here */
#include <iostream>
#include <string.h> 
#include <coap.h> 

int optionValidation(char* source);
int calOptionSize(char* source, int* type, int* data);
int parseOption(char * source, char* type, char* data); 
int calPathSize(char* source);
int parsePath(char* source, char* path);
int optionRegister(coap_resource_t **resource , char* temp_str);
int pathRegister(coap_resource_t *old_resource, coap_resource_t **new_resource , char* temp_str);
int parseLinkFormat(char* str, coap_resource_t* old_resource, coap_resource_t** resource);
int parseOptionURIQuery(char* option_value, unsigned short option_length, char** query_name, char** query_value);
void dynamicConcatenate(char **str, char *str2);
int calculateResourceLF(coap_resource_t* resource);
#endif
