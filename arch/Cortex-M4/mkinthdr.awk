BEGIN {
	VECTNUM = 104;

	printf("\t.syntax unified\n");
	printf("\t.cpu cortex-m4\n");
	printf("\t.fpu fpv4-sp-d16\n");
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
