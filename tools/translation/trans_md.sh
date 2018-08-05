awk -f tools/translation/comm_sepa_md.awk $1 > $1.spj
awk -f tools/translation/reptrans.awk JACOMMENT.txt > JACOMMENT.tmp
#trans -b ja:en -i JACOMMENT.tmp -o ENCOMMENT.tmp
/usr/local/bin/trans -b ja:en -e bing -i JACOMMENT.tmp -o ENCOMMENT.tmp
awk -f tools/translation/reptrans.awk ARG=r ENCOMMENT.tmp > ENCOMMENT.txt
awk -f tools/translation/trans_marge.awk $1.spj
rm -f JACOMMENT.txt JACOMMENT.tmp ENCOMMENT.tmp ENCOMMENT.txt $1.spj
