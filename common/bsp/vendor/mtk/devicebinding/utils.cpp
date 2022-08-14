 /*
 * Copyright (c) 2014 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

void hexStringToBin(uint8_t* pArray, char* hexString, size_t* pStrlen) {
    char* pos = hexString;
    if (NULL == pArray || NULL == hexString) {
        *pStrlen = 0;
    } else {
        int count = 0;
        while (NULL != pos && count < *pStrlen) {
            sscanf(pos, "%2hhx", &pArray[count]);
            pos += 2 * sizeof(char);
            count++;
        }
        *pStrlen = count;
    }
}

size_t suidBinToChar(char* cSuid, uint8_t* pSuid, size_t length) {
    if (NULL == cSuid || NULL == pSuid) {
        return 0;
    } else {
        char* pos = cSuid;
        int i = 0;
        while (NULL != pos && i < length) {
            sprintf(pos, "%02x", pSuid[i]);
            pos += 2 * sizeof(char);
            i++;
        }
        return i;
    }
}

size_t readFile(
    const char* pPath,
    uint8_t** ppContent)
{
    FILE*   pStream;
    long    filesize;
    uint8_t* content = NULL;

   /* Open the file */
   pStream = fopen(pPath, "rb");
   if (pStream == NULL)
   {
      fprintf(stderr, "Error: Cannot open file: %s.\n", pPath);
      return 0;
   }

   if (fseek(pStream, 0L, SEEK_END) != 0)
   {
      fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
      goto error;
   }

   filesize = ftell(pStream);
   if (filesize < 0)
   {
      fprintf(stderr, "Error: Cannot get the file size: %s.\n", pPath);
      goto error;
   }

   if (filesize == 0)
   {
      fprintf(stderr, "Error: Empty file: %s.\n", pPath);
      goto error;
   }

   /* Set the file pointer at the beginning of the file */
   if (fseek(pStream, 0L, SEEK_SET) != 0)
   {
      fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
      goto error;
   }

   /* Allocate a buffer for the content */
   content = (uint8_t*)malloc(filesize);
   if (content == NULL)
   {
      fprintf(stderr, "Error: Cannot read file: Out of memory.\n");
      goto error;
   }

   /* Read data from the file into the buffer */
   if (fread(content, (size_t)filesize, 1, pStream) != 1)
   {
      fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
      goto error;
   }

   /* Close the file */
   fclose(pStream);
   *ppContent = content;

   /* Return number of bytes read */
   return (size_t)filesize;

error:
   if (content != NULL)
   {
       free(content);
   }
   fclose(pStream);
   return 0;
}
