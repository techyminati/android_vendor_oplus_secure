//
//    Copyright 2017 Egis Technology Inc.
// 
//    This software is protected by copyright, international
//    treaties and various patents. Any copy, reproduction or otherwise use of
//    this software must be authorized by Egis in a license agreement and
//    include this Copyright Notice and any other notices specified
//    in the license agreement. Any redistribution in binary form must be
//    authorized in the license agreement and include this Copyright Notice
//    and any other notices specified in the license agreement and/or in
//    materials provided with the binary distribution.
//
#ifndef __INI_PARSER_H__
#define __INI_PARSER_H__

#include <stdint.h>

void ini_parser_set_string(char* string);
int ini_parser_get_int(const char* section, const char* name, int default_value);
int ini_parser_get_string(const char* section, const char* name, char* value, int size, const char* default_value);
double ini_parser_get_double(const char* section, const char* name, double default_value);

#endif