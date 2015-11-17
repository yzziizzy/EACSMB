
#ifndef __utilities_h__
#define __utilities_h__

 
// i pronounce this one like "Grexit", greece's only smart move which they won't make cause they're greedy, short-sighted and dumb. just like the rest of us. 
#define glexit(msg) _glexit(msg, __FILE__, __LINE__, __func__)

// returns NULL for no error, a human error string otherwise. the error is printed to stderr. 
#define glerr(msg) _glerr(msg, __FILE__, __LINE__, __func__)


// yeah yeah double evaluation. i'm only using them with variables and constants so shut up.
#define MAX(a,b) ((a) > (b) ?  (a) : (b))
#define MIN(a,b) ((a) < (b) ?  (a) : (b))
#define MAXE(a,b) ((a) >= (b) ?  (a) : (b))
#define MINE(a,b) ((a) <= (b) ?  (a) : (b))



char* readFile(char* path, int* srcLen);




#endif