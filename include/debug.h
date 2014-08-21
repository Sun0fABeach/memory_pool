#define VAL(x) printf("Value of " #x ": %g\n", (double)(x))
#define VAL_DET(x) printf("Value of " #x ": %g\t"\
						  "--- File: %s, Function: %s, Line: %d\n",\
						  (double)(x), __FILE__, __func__, __LINE__)
