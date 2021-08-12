BEGIN {
	FS = "[ ,\t]";
	printf("// BEGIN Generated from %s\n", ARGV[1]);
	if(INCLUDE == 0) {
		printf("#ifndef SYSCONFIG_H\n");
		printf("#define SYSCONFIG_H\n");
	}
}

{
	if($1 == "+") {
		cmd = sprintf("awk -f tools/mksysconfig_h.awk -v INCLUDE=1 configs/%s", $2);
		#printf("%s\n", cmd);
		system(cmd);
	} else if(($1 !~ /^#/) && ($1 !~ /^@/)) {
		if(NF == 1) {
			if(cont == 0) {
				printf("#define GSC_%s\n", $1);
			}
		} else if(NF >= 2) {
			if(cont == 0) {
				if($2 !~ /^#/) {
						printf("#define GSC_%s %s\n", $1, $2);
				} else {
					printf("#define GSC_%s\n", $1);
				}
			}
		}

		if($NF == "\\") {
			cont = 1;
		} else {
			cont = 0;
		}
	}
}

END {
	if(INCLUDE == 0) {
		printf("#endif // SYSCONFIG_H\n");
	}
	printf("// END Generated from %s\n", ARGV[1]);
}
