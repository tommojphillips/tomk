/* libc/string/strcpy.c */

char* strcpy(char* restrict dest, const char* restrict src) {
	char* ret = dest;
	while (*src != '\0') {
		*dest = *src;
		src++;
		dest++;
	}
	return ret;
}
