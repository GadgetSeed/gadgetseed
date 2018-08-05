BEGIN {
	printf("#!/bin/bash\n");
}
{
	if($2 == "@target") {
		for(i=3; i<=NF; i++) {
			appconf = FILENAME;
			gsub("configs/", "", appconf);
			gsub(".conf", "", appconf);

			printf("make clean;find . -name *.[oa] | xargs rm;\n");

			printf("echo \"SYSTEM_CONFIG = %s\" > config.mk\n", $i);
			printf("echo \"APPLICATION_CONFIG = %s\" >> config.mk\n", appconf);

			printf("make -j 2>&1 | tee %s_%s.log\n", $i, appconf);

			printf("if [ ${PIPESTATUS[0]} -ne 0 ]; then echo \"BUILD [[31mNG[m]\" ; else echo \"BUILD [[32mOK[m]\" ; fi >> %s_%s.log\n",
			       $i, appconf);
		}
	}
}
