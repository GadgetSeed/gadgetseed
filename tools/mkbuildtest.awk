BEGIN {
	printf("#!/bin/bash\n");
}
{
	if($1 == "@target") {
		for(i=2; i<=NF; i++) {
			if(match($i, "emu") != 0) {
				if(target != "all") {
					continue;
				}
			}
			appconf = FILENAME;
			gsub("configs/", "", appconf);
			gsub(".conf", "", appconf);

			printf("make objclean;make configclean\n");
			printf("find . -name *.[oa] | xargs rm\n");
			printf("find . -name \".depend\" | xargs rm\n");

			printf("echo \"SYSTEM_CONFIG = %s\" > config.mk\n", $i);
			printf("echo \"APPLICATION_CONFIG = %s\" >> config.mk\n", appconf);

			printf("make hex -j 2>&1 | tee %s_%s.log\n", $i, appconf);

			printf("if [ ${PIPESTATUS[0]} -ne 0 ]; then echo \"BUILD [[31mNG[m]\" ; else echo \"BUILD [[32mOK[m]\" ; fi >> %s_%s.log\n",
			       $i, appconf);
		}
	}
}
