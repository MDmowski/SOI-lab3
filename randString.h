#ifndef randString
#pragma once
#include <stdlib.h>
#include <string.h>
static char *rand_string(size_t size, char *s)
{
    const char charset[] = "ABC";
    if(size)
    {
        for(size_t n = 0; n < size; n++)
        {
            int key = rand() % (int)(sizeof charset - 1);
            *(s + n) = charset[key];
        }
        *(s + size) = '\0';
    }
    return s;
}

char *concat(const char *s1, const char *s2)
{
    char *result =
        malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

#endif // !randString