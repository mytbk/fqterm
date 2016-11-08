#ifndef SSH_CRYPTO_COMMON_H
#define SSH_CRYPTO_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct
	{
		const char *name;
		void *f;
	} *name_sp, name_list[]; /* Do not write as *name_list!!! */

	int search_name(name_list l, const char *s);

#ifdef __cplusplus
}
#endif

#endif
