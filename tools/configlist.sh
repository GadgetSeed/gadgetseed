if [ "$1" = "m" ]; then
    echo "# GadgetSeed コンフィグレーション\n"
    echo "configs/\*.conf および configs/systems/\*.conf ファイルには以下のコンフィグレーション項目を設定することができます。\n"
    echo "| コンフィグレーション名 | 内容 |"
    echo "|------------------------|------|"
fi

awk -f tools/configlist.awk ARG=$1 Makefile common.mk apps/*/*.[ch] arch/*/*.[ch] arch/*/Makefile arch/*/*/*.[ch] drivers/*.[ch] font/*.[ch] fs/*.[ch] graphics/*.[ch] include/*.h include/*/*.h kernel/*.[ch] kernel/*/*.[ch] libs/*.[ch] main.c net/*.[ch] shell/*.[ch] uilib/*.[ch] | sort
