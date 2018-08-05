BEGIN {
	flg_code = 0;
	system("rm -f JACOMMENT.txt");
}

{
	if($1 == "```sh") {
		flg_code = 1;
		print $0
		next;
	} else if($1 == "```") {
		flg_code = 0;
		print $0
		next;
	}

	if(flg_code == 0) {
		print $0 > "tmp";
		close("tmp");
		res = system("nkf -g tmp | grep UTF-8 > /dev/null");
		if(res == 0) {
			#print "JP:" # DEBUG
			for(i=1; i<=length($0); i++) {
				ch = substr($0, i, 1);
				if(ch == " ") {
					printf(" ");
				} else if(ch == "#") {
					printf("#");
				} else if(ch == "\t") {
					printf("\t");
				} else {
					break;
				}
			}
			printf("///JACOMMENT\n");
			jaline ++;
			gsub(/^ +/, "");
			gsub(/^#+/, "");
			gsub(/^ +/, "");
			print $0 > "JACOMMENT.txt";
		} else {
			print $0;
		}
	} else {
		print $0;
	}
}

END {
	system("rm -f tmp");
}
