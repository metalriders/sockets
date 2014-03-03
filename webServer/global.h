/**
 *
*/

#ifndef _GLOBAL_H
#define _GLOBAL_H

#define TRUE 1
#define FALSE 0

#define CONFIG_DEFAULT_PORT 8080

#define MINPORT 1
#define MAXPORT 65000

char * itoa(int i) {
  char * res = malloc(8*sizeof(int));
  sprintf(res, "%d", i);
  return res;
}

#endif
