#!/usr/bin/perl -w

for (my $i = 0; $i < 256; $i++){
	print "vector$i: \n";

	if (!($i == 8 || ($i >= 10 && $i <= 14) || $i == 17)){
		print " push \$0\n";
	}
	print " push \$$i\n";
	print " jmp isr_common\n\n";
}

print ".globl vectors\n";
print "vectors: \n";
for (my $i = 0; $i < 256; $i++){
	print ".quad vector$i\n";
}
