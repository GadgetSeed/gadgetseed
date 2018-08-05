BEGIN {
	VECTNUM = 98;

	printf("\t.syntax unified\n");
	printf("\t.cpu cortex-m4\n");
	printf("\t.eabi_attribute 27, 3\n");
	printf("\t.fpu fpv4-sp-d16\n");
	printf("\t.eabi_attribute 20, 1\n");
	printf("\t.eabi_attribute 21, 1\n");
	printf("\t.eabi_attribute 23, 3\n");
	printf("\t.eabi_attribute 24, 1\n");
	printf("\t.eabi_attribute 25, 1\n");
	printf("\t.eabi_attribute 26, 1\n");
	printf("\t.eabi_attribute 30, 2\n");
	printf("\t.eabi_attribute 34, 1\n");
	printf("\t.eabi_attribute 18, 4\n");
	printf("\t.thumb\n");
	printf("\t.text\n");
	printf("\t.align\t4\n");

	for(i=0; i<VECTNUM; i++) {
		printf("\t.global	int%03d\n", i);
		printf("\t.type	int%03d, %%function\n", i);
	}

	printf("\n");
	printf("\t.extern	inthdr_func\n\n");

	for(i=0; i<11; i++) {
		printf("int%03d:\n", i);
		printf("\tmov	r0, #%d\n", i);
		printf("\tb	inthdr_fault\n\n", i);
	}

	for(i=11; i<VECTNUM; i++) {
		printf("int%03d:\n", i);
		printf("\tmov	r0, #%d\n", i);
		printf("\tb	inthdr_func\n\n", i);
	}

	printf("\t.end\n");
}
