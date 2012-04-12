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

@STORE = ({'key1' => 'value1', 'key2' => 'value2'} );
$Nblock = 0;

while (<in>){
  #print $_;

  $line = $_;

  if ($status_found_block == 1) {
    # find all line beginning with //
    if ($line =~ m/\s*\/\/(.*)/) {
      # find line containg the blocks name followed by " - "

      $doculine = $1;
      print "    " .$doculine . "\n";

      push( @{ $STORE[$Nblock]{'doculines'} } , $doculine );
    } else {

    #if ($line =~ m/\s*endfunction/ ) {
      # block finished
      $status_found_block = 0;
      print "\n\n\n\n\n";
    }
  } 

  # find beginning of a block
#  if ( $line =~ m/\A\s*function\s*(\[\s*sim.*)/ ) {
  # find something like "function [sim,out] = ld_switch2to1(sim, events, cntrl, in1, in2) // PARSEDOCU_BLOCK"
  if ( $line =~ m/\A\s*function\s*(\[[^\]]*\][^\=]\=\s*(\w*)\(([^\)]*)\))\s*\/\/\s*PARSEDOCU_BLOCK/ ) {
    $status_block_call = $1;  # would be "[sim,out] = ld_switch2to1(sim, events, cntrl, in1, in2)"
    $status_block_name = $2;  # would be ld_switch2to1

    #print $1;

    $status_found_block = 1;

    $Nblock = $Nblock + 1;

    push(@STORE, {'status_block_call' => $status_block_call, 'status_block_name' => $status_block_name, 'doculines' => []} );

#    if ( $line =~ m/[^\=]\=\s*(\w*)\(([^\)]*)\)\s*\/\/\s*PARSEDOCU_BLOCK/    )  {
#    if ( $line =~ m/[^\=]\=\s*(\w*)\(([^\)]*)\)\w*\/\//    )  {
#    if ( $line =~ m/[^\=]\=\s*(\w*)\([\,\w]*\)/ )  {
      #$status_block_name = $2;

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

 #   }
  }



# in $_ steht die aktuelle Zeile
}
close in;





for (my $i = 1; $i <= $#STORE; $i++) {
  #print $STORE[$i]{'status_block_call'} . ".\n";
  #print $STORE[$i]{'status_block_name'} . ".\n";

  $status_block_call = $STORE[$i]{'status_block_call'};
  $status_block_name = $STORE[$i]{'status_block_name'};
  $purpose = "";
  $text = "";

  @LINES = @{ $STORE[$i]{'doculines'} };
  for (my $j = 0; $j <= $#LINES; $j++) {
    #print $LINES[$j];
    #print $STORE[$i]{'doculines'}[$j] . "\n";
    $line = $STORE[$i]{'doculines'}[$j];

    # replace special chars
    $line =~ s/\"/&quot;/; 
    $line =~ s/\'/&apos;/; 
    $line =~ s/</&lt;/; 
    $line =~ s/>/&gt;/; 
    $line =~ s/&/&amp;/; 

    $text = $text . "<para>" . $line . "</para>\n"; # format new lines
  }


  
  
  #print $text;


$templ = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<refentry version=\"5.0-subset Scilab\" xml:id=\"$status_block_name\" xml:lang=\"en\"
          xmlns=\"http://docbook.org/ns/docbook\"
          xmlns:xlink=\"http://www.w3.org/1999/xlink\"
          xmlns:svg=\"http://www.w3.org/2000/svg\"
          xmlns:ns3=\"http://www.w3.org/1999/xhtml\"
          xmlns:mml=\"http://www.w3.org/1998/Math/MathML\"
          xmlns:db=\"http://docbook.org/ns/docbook\">
  <info>
    <pubdate>\$LastChangedDate: \$</pubdate>
  </info>

  <refnamediv>
    <refname>$status_block_name</refname>

    <refpurpose>$purpose</refpurpose>
  </refnamediv>

  <refsynopsisdiv>
    <title>Calling Sequence</title>

    <synopsis>$status_block_call</synopsis>
  </refsynopsisdiv>

  <refsection>
    <title>Description</title>

    <para>$text</para>

    
  </refsection>


</refentry>";

#   <refsection>
#     <title>Examples</title>
# 
#     <programlisting role=\"example\">c_sum(3,4)</programlisting>
#   </refsection>
#   <refsection>
#     <title>Authors</title>
# 
#     <simplelist type=\"vert\">
#       <member></member>
#     </simplelist>
#   </refsection>




  #print $templ;

  # place  to store
  # create dir scilab/ld_toolbox/help/en_US/$module_name/
  system("mkdir scilab/ld_toolbox/help/en_US/$module_name/");
  $filename = "scilab/ld_toolbox/help/en_US/$module_name/$status_block_name.xml";

  # save
  open (MYFILE, "> $filename");
  print MYFILE $templ;
  close (MYFILE); 
}


