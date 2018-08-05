BEGIN {
	i = 1;
	wc = 1;

	while((getline comment[i] < "ENCOMMENT.txt") > 0) {
		#print comment[i];
		i ++;
	}

	while((getline words[wc] < "tools/translation/JATOENDICT.txt") > 0) {
		#print words[wc];	# DEBUG
		split(words[wc], word, " ");
		#print word[1], word[2];	# DEBUG
		fword[wc] = word[1];
		if(word[2] == "") {
			tword[wc] = word[1];
		} else {
			tword[wc] = word[2];
		}
		#print fword[wc], tword[wc];	# DEBUG
		wc ++;
	}

	line = 1;
}

{
	if(match($0, "///JACOMMENT")) {
		#print comment[line];	# DEBUG
		gsub("///JACOMMENT", comment[line]);
		line ++;
	}

	for(i=1; i<wc; i++) {
		gsub(fword[i], tword[i]);
	}

	print $0;
}
