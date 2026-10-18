#!/usr/bin/perl
# Reuse the project harness's parser and instruction normalization verbatim.
use strict; use warnings; use JSON::PP; use File::Path qw(make_path);
use FindBin;
my $root = shift || "$FindBin::Bin/../../..";
my $out = "$FindBin::Bin/../obj";
my $mode = shift || 'unity';
my $exe = "$root/Exported/SQZ.EXE";
my $objdump = 'objdump';
my (%size, %callers);
open my $nf, '<', "$root/nm/fns_all.txt" or die $!;
while (<$nf>) { if (/^([0-9a-f]+) size=(\d+)\s+callers=(\d+)/) { $size{hex($1)}=$2; $callers{hex($1)}=$3; } }
my @sorted = sort {$a <=> $b} keys %size;
open my $hf, '<', "$root/tools/flagsearch/flagsearch.pl" or die $!;
my $h = do { local $/; <$hf> };
my $pepart = substr($h, index($h, 'local $/; open my $ef'), index($h, '# --- normalized disassembly')-index($h, 'local $/; open my $ef'));
my ($normal) = $h =~ /(sub dis_norm \{.*?)(?=sub diffcount)/s;
my ($parser) = $h =~ /(sub obj_functions \{.*?)(?=# --- workers)/s;
my ($cc) = $h =~ /(sub cc_size \{.*)/s;
my $driver = do { local $/; <DATA> };
eval $pepart . $normal . $parser . $cc . $driver; die $@ if $@;
__DATA__
my @objects = $mode eq 'parts' ? sort glob("$out/parts/*.obj") : ("$out/QvLib.obj");
my (@sections,%funcs); my $section_base=0;
for my $obj (@objects) {
my %f = obj_functions($obj);
open my $of, '<:raw', $obj or die $!; my $d = do { local $/; <$of> };
my ($ns,$sp,$nsym,$coffopt) = unpack('x2 v x4 V V v',$d);
my $st = $sp + $nsym*18; my (%sym,%section_name);
for (my $i=0; $i<$nsym;) {
 my ($n,$v,$s,$t,$c,$a)=unpack('a8 V s v C C',substr($d,$sp+$i*18,18));
 my $name = substr($n,0,4) eq "\0\0\0\0" ? unpack('Z*',substr($d,$st+unpack('V',substr($n,4)),1000)) : unpack('Z8',$n);
 $sym{$i}={name=>$name,value=>$v,section=>$s,type=>$t,class=>$c};
 $section_name{$s}=$name if $c==2 && $s>0;
 $i+=1+$a;
}
for my $k (0..$ns-1) {
 my ($sn,$vs,$va,$rs,$rp,$rl,$lp,$nr,$nl,$fl)=unpack('a8 V V V V V V v v V',substr($d,20+$coffopt+$k*40,40));
 my @rel;
 for my $r (0..$nr-1) {
  my ($a,$si,$t)=unpack('V V v',substr($d,$rl+$r*10,10));
  push @rel, {offset=>$a,type=>$t,symbol=>$sym{$si}{name},section=>($sym{$si}{section}>0 ? $section_base+$sym{$si}{section} : 0),value=>$sym{$si}{value},addend=>unpack('V',substr($d,$rp+$a,4))};
 }
 my $name=$section_name{$k+1} || '';
 push @sections,{name=>$name,index=>$section_base+$k+1,object=>$obj,kind=>unpack('Z8',$sn),raw=>unpack('H*',substr($d,$rp,$rs)),relocs=>\@rel};
 if (exists $f{$name}) {
  my $bytes=strip_pad($f{$name});
  $funcs{$name}={section=>$section_base+$k+1,object=>$obj,raw=>unpack('H*',substr($d,$rp,$rs)),masked=>unpack('H*',$bytes),relocs=>\@rel,norm=>dis_norm($bytes.("\xcc"x8),0,length($bytes))};
 }
}
$section_base+=$ns;
}
print STDERR scalar(keys %funcs)," source functions normalized\n";
my %original;
if ($mode eq 'parts' && -f "$out/corpus.json") {
 open my $old,'<',"$out/corpus.json" or die $!; my $previous=decode_json(do {local $/; <$old>}); %original=%{$previous->{exe}};
} else {
for my $addr (grep {$_>=0x47e600 && $_<0x48e000} @sorted) {
 my %lens=($size{$addr}=>1, cc_size($addr)=>1);
 my $end=$addr+$size{$addr};
 for (;;) { my ($n)=grep {$_ >= $end && $_ <= $end+8} @sorted; last unless defined($n) && $size{$n}<=40 && $callers{$n}<=1; $end=$n+$size{$n}; }
 $lens{$end-$addr}=1;
 my @variants;
 for my $sz (sort {$a<=>$b} keys %lens) {
  next if $sz>16000;
  my $bytes=strip_pad(orig_bytes($addr,$sz));
  push @variants,{size=>$sz,masked=>unpack('H*',$bytes),norm=>dis_norm(orig_bytes($addr,$sz+8),$addr,length($bytes))};
 }
 $original{$addr}={size=>$size{$addr},variants=>\@variants};
}
}
open my $j,'>',"$out/corpus".($mode eq 'parts' ? '-parts' : '').".json" or die $!;
print $j JSON::PP->new->canonical->encode({functions=>\%funcs,exe=>\%original,sections=>\@sections}); close $j;
print scalar(keys %funcs)," source functions; ",scalar(keys %original)," EXE functions\n";
