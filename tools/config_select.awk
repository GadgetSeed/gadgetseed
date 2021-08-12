BEGIN {
	printf("*** Select target apprication ***\n");

	num = 1;
}

{
	desc = "";
	targets = "";
	cmd = sprintf("cat $CONFIGS_DIR/%s | grep ^@description | cut -d ' ' -f 2-", $1);
	cmd | getline desc;
	close(cmd);
	printf("%3d : %-40s : %s\n", num, desc, $1);
	confnames[num] = $1;
	descs[num] = desc;
	num ++;
}

END {
	printf("Input No. : ");
	keyin = ""; 

	getline keyin <"-";

	if((keyin != 0) && (keyin < num)) {
		confname = confnames[keyin];
		desc = descs[keyin];
		printf("Select : %d\n", keyin);
		printf("Target apprication : %s (%s)\n", desc, confname);
	} else {
		printf("Select canceled.\n");
		exit(1);
	}
	gsub(".conf", "", confname);
	cmd = sprintf("echo \"APPLICATION_CONFIG =\" \"%s\" >> config.tmp", confname);
	system(cmd);
}
