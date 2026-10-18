#!/usr/bin/perl
# Extend obj_functions to internal-linkage functions, which the stock harness
# omits. Keep all instruction normalization and scoring in the shared harness.
use strict; use warnings; use JSON::PP; use FindBin;
my $root=shift || "$FindBin::Bin/../../..";
my $out="$FindBin::Bin/../obj"; my $objdump='objdump';
open my $hf,'<',"$root/tools/flagsearch/flagsearch.pl" or die $!;
my $h=do {local $/;<$hf>};
my ($parser)=$h =~ /(sub obj_functions \{.*?)(?=# --- workers)/s;
$parser =~ s/\$cls == 2/\(\$cls == 2 || \$cls == 3\)/;
my ($normal)=$h =~ /(sub dis_norm \{.*?)(?=sub diffcount)/s;
open my $cf,'<',"$out/corpus-parts.json" or die $!;
my $c=decode_json(do {local $/;<$cf>});
eval $parser.$normal; die $@ if $@;
my %byobj;
push @{$byobj{$_->{object}}},$_ for @{$c->{sections}};
my $count=0;
for my $obj (sort keys %byobj) {
 my %all=obj_functions($obj);
 for my $s (@{$byobj{$obj}}) {
  next unless $s->{kind} eq '.text' && !$s->{name};
  my $code=pack('H*',$s->{raw});
  for my $r (@{$s->{relocs}}) { substr($code,$r->{offset},4)="\xef\xbe\xad\xde" if $r->{type}==6||$r->{type}==20; }
  for my $name (sort keys %all) {
   next unless $all{$name} eq $code && !exists($c->{functions}{$name});
   $code =~ s/[\xcc\x90]+$//;
   $c->{functions}{$name}={section=>$s->{index},object=>$obj,raw=>$s->{raw},masked=>unpack('H*',$code),relocs=>$s->{relocs},norm=>dis_norm($code.("\xcc"x8),0,length($code)),internal_static=>JSON::PP::true};
   $s->{name}=$name; $count++; last;
  }
 }
}
open my $j,'>',"$out/corpus-parts.json" or die $!; print $j JSON::PP->new->canonical->encode($c); close $j;
print "$count internal functions added\n";
