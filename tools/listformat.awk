BEGIN {
	FS = ",";
}

{
	printf("| %-34s | %-60s |\n", $1, $2);
}
