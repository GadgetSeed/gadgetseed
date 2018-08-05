BEGIN {
	FS = "[ ,\t]";
	printf("# Generated from");
	for(i=1; i<=ARGC; i++) {
		printf(" %s", ARGV[i]);
	}
	printf("\n");
}

{
	if($1 == "+") {
		cmd = sprintf("awk -f tools/mktargetconfig_mk.awk -v INCLUDE=1 configs/%s", $2);
		#printf("%s\n", cmd);
		system(cmd);
	} else if($1 !~ "^#") {
		if(NF == 1) {
			if(cont == 0) {
				printf("export %s = YES\n", $1);
			} else {
				printf("	%s\n", $1);
			}
			if($NF == "\\") {
				cont = 1;
			} else {
				cont = 0;
			}
		} else if(NF >= 2) {
			val = 0;
			if(cont == 0) {
				printf("export %s =", $1);
				for(i=2; i<=NF; i++) {
					if($(i) != "") {
						if($(i) !~ /^#/) {
							printf(" %s", $(i));
							val = 1;
						} else {
							break;
						}
					}
				}
				if(val == 0) {
					printf(" YES");
				}
			} else {
				printf("	%s", $1);
				for(i=2; i<=NF; i++) {
					printf(" %s", $(i));
				}
			}
			printf("\n");
			if($NF == "\\") {
				cont = 1;
			} else {
				cont = 0;
			}
		}
	}
}
