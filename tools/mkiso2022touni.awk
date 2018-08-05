BEGIN {
	FS = "[ \t]";
}

{
	if($1 != "##") {
		ku = substr($1,1,1);
		jis = strtonum("0x" substr($1,3));
		uni = strtonum("0x" substr($2,3));
		if(ku != 4) {
			#printf("%d JIS %04X -> UNI %04X\n", ku, jis, uni);
			if(uni < 0x10000) {
				table[jis] = uni;
			}
		}
	}
}

END {
	printf("const unsigned short jis2uni_table[] = {\n");

	for(i=0; i<0x10000; i++) {
		printf("\t0x%04x, // 0x%04x\n", table[i], i);
	}

	printf("};\n");
}
