BEGIN {
	FS = "[ \\[\\]() <>;\t+-/*={}&|]";
	gsc_num = 0;
	line = 1;
}

{
	if(FILENAME != fname) {
		fname = FILENAME;
		line = 1;
	}

	for(i=1; i<=NF; i++) {
		name = "";
		desc = "";
		cnum = -1;
		flg_find = 0;
		if(match($i, /GSC_.+/)) {
			flg_find = 1;
			name = $i;
			gsub("GSC_", "", name);
			#print name;
			for(j=1; j<=NF; j++) {
				if($j == "$gsc") {
					desc = substr($0, index($0,"$gsc ")+5, length($0));
					break;
				}
			}
		} else if($i == "$gsc") {
			for(j=1; j<=NF; j++) {
				if($j == "$") {
					#print $0;
					#print $(j+1);
					flg_find = 1;
					name = $(j+1);
					desc = substr($0, index($0,"$gsc ")+5, length($0));
					break;
				} else if($1 == "ifdef") {
					flg_find = 1;
					name = $2;
					desc = substr($0, index($0,"$gsc ")+5, length($0));
					break;
				} else if($1 == "ifndef") {
					flg_find = 1;
					name = $2;
					desc = substr($0, index($0,"$gsc ")+5, length($0));
					break;
				}
			}
		}

		for(j=0; j<gsc_num; j++) {
			if(gsc_name[j] == name) {
				cnum = j;
				break;
			}
		}

		if(flg_find != 0) {
			if(cnum == -1) {
				gsc_name[gsc_num] = name;
				gsc_desc[gsc_num] = desc;
				gsc_file[gsc_num] = FILENAME;
				gsc_line[gsc_num] = line;
				gsc_num ++;
			} else {
				if(desc != "") {
					gsc_desc[cnum] = desc;
					gsc_file[cnum] = FILENAME;
					gsc_line[cnum] = line;
				}
			}
		}
	}

	line ++;
}

END {
	for(i=0; i<gsc_num; i++) {
		if(gsc_desc[i] == "") {
			gsc_desc[i] = "Without explanation";
		}
	}

#	if(ARG == "m") {
#		printf("| %-34s | %-60s |\n", " コンフィグ名", "内容");
#		printf("|--------------------------------|------------------------------------------------------------|\n");
#	}

	for(i=0; i<gsc_num; i++) {
		#printf("%3d : %-40s : %s\n", i+1, gsc_name[i], gsc_desc[i]);
		if(ARG == "v") {
			printf("%-34s : %s %s:%d\n", gsc_name[i], gsc_desc[i], gsc_file[i], gsc_line[i]);
		} else if(ARG == "m") {
			printf("| %-34s | %-60s |\n", gsc_name[i], gsc_desc[i]);
		} else {
			printf("%-34s : %s\n", gsc_name[i], gsc_desc[i]);
		}
	}
}
