#! /usr/bin/perl -w
# to call this script, add in you CVSROOT/commitinfo :
# module_to_match THE_GOOF/PATH/commitinfo.pl

# (c)1999  Robert CHERAMY <tibob@via.ecp.fr>
# you are free to reuse this script

system ("running commitinfo.pl");

$last_dir_file = "/tmp/#visioncvs.files.lastdir";

$my_pgrp_id = getpgrp();

$commitdir = $ARGV[0];

#print "opening $last_dir_file.$my_pgrp_id\n";
open(LDFD, ">$last_dir_file.$my_pgrp_id") ||
  die "Cannot open file $last_dir_file.$my_pgrp_id";
print LDFD "$commitdir\n";
close(LDFD);
