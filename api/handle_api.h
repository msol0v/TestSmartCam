//
// Created by msol0v on 30.03.2026.
//

#ifndef ETHTEST_1_HANDLE_API_H
#define ETHTEST_1_HANDLE_API_H

#include "lwip/apps/httpd.h"
#include <string.h>

const char* minus_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* plus_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

#endif //ETHTEST_1_HANDLE_API_H