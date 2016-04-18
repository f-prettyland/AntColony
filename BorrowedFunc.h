#include <string.h>

// Taken from itsme86
// https://www.linuxquestions.org/questions/programming-9/replace-a-substring-with-another-string-in-c-170076/
char *replace_str(char *str, const char *orig, char *rep)
{
  static char buffer[4096];
  char *p;

  if(!(p = strstr(str, orig))) 
    // Original function had:
    // 		return str;
    // This is changed to account for multiple occurences.
  	return NULL;

  strncpy(buffer, str, p-str); 
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  return buffer;
}