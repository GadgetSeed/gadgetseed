awk -v target=$1 -f tools/mkbuildtest.awk configs/*.conf > buildalltest.sh
echo "grep BUILD *.log" >> buildalltest.sh
echo "grep warning *.log" >> buildalltest.sh
chmod +x buildalltest.sh
