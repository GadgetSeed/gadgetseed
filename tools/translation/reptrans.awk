BEGIN {
	FS = "[ \"]";
	wc = 1;

	while((getline word[wc] < "tools/translation/NOTRANSWORD.txt") > 0) {
		#print word[wc];	# DEBUG
		wc ++;
	}
}

{
	for(i=1; i<wc; i++) {
		rep = sprintf("9876543210%d", i);
		#print rep;	# DEBUG
		if(ARG == "r") {
			gsub(rep, word[i]);
			gsub(/\] \(/, "](");	# !!!
		} else {
			gsub(word[i], rep);
		}
	}

	print $0;
}
