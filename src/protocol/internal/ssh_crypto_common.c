#include "ssh_crypto_common.h"
#include <string.h>

/* We need the first algorithm in the client side,
   so search l first
*/

int
search_name(name_list l, const char *s)
{
	size_t ns = 1;
	size_t i,j;
	const char *p;

	if (*s=='\0')
		return -1;

	for (p=s; *p; p++) {
		if (*p==',') ns++;
	}
	const char *start[ns], *end[ns];

	p = s;
	for (i=0; i<ns; i++) {
		start[i] = p;
		end[i] = strchr(p, ',');
		p = end[i]+1;
	}
	end[ns-1] = start[ns-1]+strlen(start[ns-1]);

	for (i=0; l[i].name!=NULL; i++) {
		size_t len = strlen(l[i].name);
		for (j=0; j<ns; j++) {
			if (start[j]+len==end[j] && strncmp(start[j],l[i].name,len)==0)
				return i;
		}
	}
	return -1;
}
