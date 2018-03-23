int main() {
	char * s = "Ciao mondo";
	void * ptr = malloc(5);
	memcpy(ptr, s , strlen(s));
	return 0;
}
