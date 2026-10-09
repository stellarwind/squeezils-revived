#!/usr/bin/perl
# triage.pl <ref.txt> <got.txt> [label]
# Classify a byte-match divergence so nobody re-tries a register swap a tenth time.
# Reads the normalized instruction lists flagsearch.pl's dump_show writes
# (one instruction per line, relocations already masked as X, local jumps as "jmp +N").
# Prints: LABEL first_divergence_line classification detail
use strict; use warnings;

sub load { my ($f) = @_; open my $h, '<', $f or die "no $f"; my @l = <$h>; chomp @l; return \@l; }

# Canonicalise general-purpose registers by first appearance so a pure register swap collapses.
sub canon {
    my ($lines) = @_;
    my %map; my $idx = 0; my @out;
    for my $l (@$lines) {
        my $o = $l;
        $o =~ s/\b(eax|ebx|ecx|edx|esi|edi|ebp|esp|ax|bx|cx|dx|si|di|al|ah|bl|bh|cl|ch|dl|dh)\b/do {
            my $r = lc $1;
            if (!exists $map{$r}) { $map{$r} = "r" . $idx++; }
            $map{$r};
        }/gex;
        push @out, $o;
    }
    return \@out;
}

my $label = $ARGV[2] || "fn";
my ($ref, $got) = (load($ARGV[0]), load($ARGV[1]));
my ($n, $m) = (scalar(@$ref), scalar(@$got));

# First divergence location (1-based in the ref); also whether it is in the prologue.
my $first = undef;
my $len = $n < $m ? $n : $m;
for (my $i = 0; $i < $len; $i++) {
    if ($ref->[$i] ne $got->[$i]) { $first = $i + 1; last; }
}
$first = $len + 1 unless defined $first;
my $where = $first <= 4 ? "prologue" : "body";

# 1. Pure register swap: canonicalised sequences are identical.
my ($cr, $cg) = (canon($ref), canon($got));
if (join("\n", @$cr) eq join("\n", @$cg)) {
    print "$label\t$first\t$where\tallocator-register-swap\tleave (register roles swapped, source equivalent)\n";
    exit 0;
}

# 2. EH cleanup boundary: everything equal except jmp +N <-> jmp X.
my $only_jmp = 1; my $jmpn = 0;
if ($n == $m) {
    for (my $i = 0; $i < $n; $i++) {
        my ($a, $b) = ($ref->[$i], $got->[$i]);
        next if $a eq $b;
        my $ok = ($a =~ /^j\w+ \+/ && $b =~ /^j\w+ X$/) || ($a =~ /^j\w+ X$/ && $b =~ /^j\w+ \+/);
        if (!$ok) { $only_jmp = 0; last; }
        $jmpn++;
    }
    if ($only_jmp && $jmpn) {
        print "$label\t$first\t$where\teh-funclet-boundary\tleave (local jmp +N vs external jmp X: COMDAT fold)\n";
        exit 0;
    }
}

# 3. Pure reorder: same multiset, different order (x87 fld order / operand order).
my @sr = sort @$ref; my @sg = sort @$got;
if (join("\n", @sr) eq join("\n", @sg)) {
    print "$label\t$first\t$where\treorder\tmaybe-source (scheduler/x87 fld order or operand order)\n";
    exit 0;
}

# 4. Length difference: if the surplus lines are pure register moves, it is an allocator choice.
my $extra = $m - $n;
if ($extra != 0) {
    my ($long, $short) = $extra > 0 ? ($got, $ref) : ($ref, $got);
    my $k = abs $extra;
    # rough: find lines in the longer list that are plain reg->reg moves and not present in the shorter list
    my $movs = 0;
    my %sh = map { $_ => 1 } @$short;
    for my $l (@$long) { $movs++ if $l =~ /^mov\s+\w+,\w+$/ && !$sh{$l}; }
    if ($movs >= $k) {
        print "$label\t$first\t$where\tallocator-register-move\tleave (surplus lines are register moves: allocator choice)\n";
        exit 0;
    }
    print "$label\t$first\t$where\tlength$extra\tmissing/extra instructions (check inlined helper, intrinsic vs call, EH frame)\n";
} else {
    print "$label\t$first\t$where\tstructural\tsource-fixable candidate (operand order, branch shape, constant width, type)\n";
}
