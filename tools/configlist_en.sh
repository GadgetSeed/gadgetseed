sh ./tools/configsearch.sh n > cfgname.tmp
sh ./tools/configsearch.sh d > cfgdisc.tmp
/usr/local/bin/trans -b ja:en -e bing -i cfgdisc.tmp > cfgdisc_en.tmp
paste -d , cfgname.tmp cfgdisc_en.tmp > cfglist.tmp

echo "# GadgetSeed Configuration\n"
echo "You can set the following configuration items in the configs/\*.conf and configs/systems/\*.conf files\n"
echo "| Configuration Name     | Description |"
echo "|------------------------|-------------|"

awk -f tools/listformat.awk cfglist.tmp | sort
