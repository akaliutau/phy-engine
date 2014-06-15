/* file.h: open/close a file recognizing certain file extensions */

#ifndef _PHY_FILE_H_
#define _PHY_FILE_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* opens a file with given name and fopen() open_mode ("w" or "r" e.g.). Returns the
 * FILE * or NULL if opening the file was not succesful. Returns in ispipe whether
 * or not the file has been opened through a pipe. File extensions
 * .Z, .gz, .bz and .bz2 are recognised and lead to piped input/output with the
 * proper compress/uncompress commands. Also if the first character of the file name is
 * equal to '|', the file name is opened as a pipe. */
extern FILE *OpenFile(char *filename, char *open_mode, int *ispipe);

/* closes the file taking into account whether or not it is a pipe */
extern void CloseFile(FILE *fp, int ispipe);

#ifdef __cplusplus
}
#endif

#endif /*_PHY_FILE_H_*/
