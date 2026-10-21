package TargetRows;
use strict;
use warnings;
use Exporter 'import';
our @EXPORT_OK = qw(parse_target_row);

# Keep grammar in agreement with target_rows.py; test_target_rows.py checks both.
sub parse_target_row {
    my ($line) = @_;
    my @fields = split ' ', $line;
    return undef if !@fields || $fields[0] =~ /^#/;
    die "expected expression address label [size] on one line\n"
        unless @fields == 3 || @fields == 4;
    die "invalid hexadecimal target address\n"
        unless $fields[1] =~ /\A(?:0[xX])?[0-9a-fA-F]{1,8}\z/;
    die "invalid target size\n" if @fields == 4
        && $fields[3] !~ /\A(?:[1-9][0-9]*|auto|ehend|cc)\z/;
    return \@fields;
}
1;
