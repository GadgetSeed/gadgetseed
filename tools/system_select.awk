BEGIN {
	printf("*** Select target system ***\n");

	num = 1;
}

{
	desc = "";
	sysname = "";
	cmd = sprintf("cat $SYSCONF_DIR/%s | grep ^@description | cut -d ' ' -f 2-", $1);
	cmd | getline desc;
	close(cmd);
	cmd = sprintf("cat $SYSCONF_DIR/%s | grep ^@system | cut -d ' ' -f 2-", $1);
	cmd | getline sysname;
	close(cmd);
	printf("%3d : %-30s : %-45s : %s\n", num, sysname, desc, $1);
	sysnames[num] = sysname;
	confnames[num] = $1;
	num ++;
}

END {
	printf("Input No. : ");
	keyin = ""; 

	getline keyin <"-";

	if((keyin != 0) && (keyin < num)) {
		sysname = sysnames[keyin];
		confname = confnames[keyin];
		printf("Select : %d\n", keyin);
		printf("Target system : %s (%s)\n", sysname, confname);
	} else {
		printf("Select canceled.\n");
		exit(1);
	}
	gsub(".conf", "", confname);
	cmd = sprintf("echo \"SYSTEM_CONFIG =\" \"%s\" > config.tmp", confname);
	system(cmd);
	cmd = sprintf("cd %s; grep -H @target *.conf | grep -w %s | cut -d ':' -f 1 > $PRJ_DIR/conf_list", ENVIRON["CONFIGS_DIR"], sysname);
	system(cmd);
}
