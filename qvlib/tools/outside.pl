#!/usr/bin/perl
# Verify stock Qv routines reached by vtables outside the nominal Qv range.
# Game overrides below 0x47e600 are recorded by map.py but never scored here.
use strict; use warnings; use JSON::PP; use FindBin;
my $root=shift || "$FindBin::Bin/../../..";
my $out="$FindBin::Bin/../obj"; my $objdump='objdump';
my $exe="$root/Exported/SQZ.EXE";
open my $hf,'<',"$root/tools/flagsearch/flagsearch.pl" or die $!;
my $h=do {local $/;<$hf>};
my $pepart=substr($h,index($h,'local $/; open my $ef'),index($h,'# --- normalized disassembly')-index($h,'local $/; open my $ef'));
my ($normal)=$h =~ /(sub dis_norm \{.*?)(?=# --- reference disassemblies)/s;
my @last_offsets;
$normal =~ s/my \(\$bytes,\$vma,\$limit\) = \@_;/my (\$bytes,\$vma,\$limit) = \@_; \@last_offsets=();/;
$normal =~ s/push \@res, \$t;/push \@res, \$t; push \@last_offsets, \$a-\$vma;/;
my ($cc)=$h =~ /(sub cc_size \{.*)/s;
open my $cf,'<',"$out/corpus-parts.json" or die $!; my $c=decode_json(do {local $/;<$cf>});
open my $mf,'<',"$FindBin::Bin/../mapping.json" or die $!; my $m=decode_json(do {local $/;<$mf>});
my $driver=do {local $/;<DATA>};
eval $pepart.$normal.$cc.$driver; die $@ if $@;
__DATA__
sub word_at {
 my $a=shift;
 for my $s (@secs) { if ($a-$base >= $s->[0] && $a-$base < $s->[0]+$s->[1]) {return unpack('V',substr($e,$s->[2]+$a-$base-$s->[0],4));} }
 return undef;
}
sub align_indices {
 my ($aa,$bb)=@_; my @dp;
 for (my $i=@$aa;$i>=0;$i--) { for (my $j=@$bb;$j>=0;$j--) {
  $dp[$i][$j]=($i==@$aa||$j==@$bb) ? 0 : ($aa->[$i] eq $bb->[$j] ? 1+$dp[$i+1][$j+1] : ($dp[$i+1][$j]>$dp[$i][$j+1] ? $dp[$i+1][$j] : $dp[$i][$j+1]));
 } }
 my ($i,$j)=(0,0); my %map;
 while ($i<@$aa && $j<@$bb) {
  if ($aa->[$i] eq $bb->[$j]) {$map{$i++}=$j++;}
  elsif ($dp[$i+1][$j]>=$dp[$i][$j+1]) {$i++;} else {$j++;}
 }
 return \%map;
}
my (%todo,%done,%results);
for my $name (keys %{$m->{outside_range}}) {
 for my $addr (@{$m->{outside_range}{$name}}) { $todo{$name}=$addr if $addr>=0x48e000 && $addr<0x49d000; }
}
while (my ($name)=sort grep {!$done{$_}} keys %todo) {
 $done{$name}=1; my $addr=$todo{$name}; my $f=$c->{functions}{$name}; next unless $f;
 my $length=length(pack('H*',$f->{masked}));
 my %sizes=($length=>1,cc_size($addr)=>1); my ($next)=sort {$a<=>$b} grep {$_>$addr} values %todo;
 $sizes{$next-$addr}=1 if defined($next) && $next-$addr<4096;
 my ($best,$bestsize,$bestnorm,$bestoff);
 for my $size (sort {$a<=>$b} keys %sizes) {
  my $bytes=strip_pad(orig_bytes($addr,$size));
  my $nn=dis_norm(orig_bytes($addr,$size+8),$addr,length($bytes));
  my $score=diffcount($nn,$f->{norm});
  if (!defined($best)||$score<$best) {($best,$bestsize,$bestnorm,$bestoff)=($score,$size,$nn,[@last_offsets]);}
 }
 $results{$name}={address=>$addr,size=>$bestsize,score=>$best,norm=>$bestnorm,offsets=>$bestoff,source_norm=>$f->{norm}};
 # Exact normalized bodies have matching instruction offsets, allowing direct
 # discovery of announce/printProperties and QvElement helpers.
 if ($best==0) {
  for my $r (@{$f->{relocs}}) {
   if ($r->{type}==6 && $r->{symbol}=~/^\?\?_7/ && $r->{section}>0) {
    my $vt=word_at($addr+$r->{offset}); my $section=$c->{sections}[$r->{section}-1];
    for my $entry (@{$section->{relocs}}) {
     my $s=$entry->{symbol}; $s=~s/^\?\?_E/??_G/ unless exists($c->{functions}{$s});
     next unless exists($c->{functions}{$s}); my $target=word_at($vt+$entry->{offset});
     $todo{$s}=$target if defined($target) && $target>=0x48e000 && $target<0x49d000;
    }
   }
   my $s=$r->{symbol}; next unless $r->{type}==20 && exists($c->{functions}{$s});
   my $o=$r->{offset}; my $raw=pack('H*',$f->{raw}); next unless $o && substr($raw,$o-1,1) =~ /[\xe8\xe9]/;
   my $w=word_at($addr+$o); next unless defined $w;
   my $target=($addr+$o+4+$w)&0xffffffff;
   $todo{$s}=$target if $target>=0x48e000 && $target<0x49d000;
  }
 }
 # Retain out-of-range call evidence for a separate sequence-alignment pass.
 my @calls;
 for my $i (0..$#$bestnorm) {
  next unless $bestnorm->[$i] eq 'call X'; my $off=$bestoff->[$i];
  my $w=word_at($addr+$off+1); next unless defined $w;
  push @calls,{index=>$i,offset=>$off,address=>(($addr+$off+5+$w)&0xffffffff)};
 }
 $results{$name}{calls}=\@calls;
 my $aligned=align_indices($f->{norm},$bestnorm);
 for my $r (@{$f->{relocs}}) {
  next unless $r->{type}==20 && exists($c->{functions}{$r->{symbol}});
  my $si; for my $i (0..$#{$f->{offsets}}) { $si=$i if $f->{offsets}[$i]<$r->{offset}; }
  next unless defined($si) && exists($aligned->{$si});
  my $ei=$aligned->{$si}; next unless $bestnorm->[$ei] eq 'call X';
  my $off=$bestoff->[$ei]; my $w=word_at($addr+$off+1); next unless defined($w);
  my $target=($addr+$off+5+$w)&0xffffffff;
  $todo{$r->{symbol}}=$target if $target>=0x48e000 && $target<0x49d000;
 }
}
open my $j,'>',"$out/outside-corpus.json" or die $!;
print $j JSON::PP->new->canonical->pretty->encode(\%results); close $j;
my %summary=map {my $r=$results{$_}; $_=>{address=>$r->{address},size=>$r->{size},score=>$r->{score}}} keys %results;
open $j,'>',"$FindBin::Bin/../outside.json" or die $!; print $j JSON::PP->new->canonical->pretty->encode(\%summary); close $j;
open my $t,'>',"$FindBin::Bin/../targets_qvlib_outside.txt" or die $!;
open my $main,'<',"$FindBin::Bin/../targets_qvlib.txt" or die $!; my $flags=<$main>; close $main; print $t $flags;
open my $sc,'>',"$FindBin::Bin/../scores_outside.txt" or die $!;
for my $name (sort {$results{$a}{address}<=>$results{$b}{address}} keys %results) {
 my $r=$results{$name}; my $addr=$r->{address}; my $regex='^'.quotemeta($name).'$';
 printf $t "%s %x qv_%x %d\n",$regex,$addr,$addr,$r->{size};
 printf $sc "qv_%x=%d\n",$addr,$r->{score};
}
close $t; close $sc;
print scalar(keys %results)," outside-range stock routines; ",scalar(grep {$_->{score}==0} values %results)," score zero\n";
