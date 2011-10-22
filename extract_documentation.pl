#!/bin/perl

$datei = "modules/rt_server/scilab_loader.sce";
$datei = $ARGV[1];
$module_name = $ARGV[0];

open (in,"<$datei") || die $!;


$status_found_block = 0;
$status_block_name = "";
$status_block_call = "";


# The header

print "M O D U L E -- $module_name\n\n\n\n";


while (<in>){
  #print $_;

  $line = $_;

  if ($status_found_block == 1) {
    # find all line beginning with //
    if ($line =~ m/\s*\/\/(.*)/) {
      $doculine = $1;
      print "    " .$doculine . "\n";
    } else {

    #if ($line =~ m/\s*endfunction/ ) {
      # block finished
      $status_found_block = 0;
      print "\n\n\n\n\n";
    }
  } 

  # find beginning of a block
  if ($line =~ m/\s*function\s*(\[\s*sim.*)/ ) {
    $status_block_call = $1;

    #print $1;

    $status_found_block = 1;
    if ( $line =~ m/[^\=]\=\s*(\w*)/ ) {
      $status_block_name = $1;

     # print $1;

      # introduce the new block
      print "BLOCK -- $status_block_name\n";
      print "\n";
      print "  CALLING SEQUENCE\n";
      print "\n";
      print "    $status_block_call\n";
      print "\n";
      print "  DESCRIPTION\n";
      print "\n";

    }
  }



# in $_ steht die aktuelle Zeile
}
close in;