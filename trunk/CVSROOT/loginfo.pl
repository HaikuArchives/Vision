#! /usr/bin/perl -w
# to call this script, add in you CVSROOT/loginfo
# module_to_match THE_GOOD_PATH/loginfo.pl sender_mail recipient_mail subject %{}

# (c)1999  Robert CHERAMY <tibob@via.ecp.fr>
# you are free to reuse this script

system("echo running loginfo.pl");


$last_dir_file = "/tmp/#cvs.files.lastdir";
$summary_file  = "/tmp/#cvs.files.summary";

$cvsroot    = $ENV{CVSROOT}."/";
$user       = $ENV{USER};


$my_pgrp_id = getpgrp();

$MAILER = "/usr/sbin/sendmail";

$mailfrom = shift;
$mailto   = shift;
$subject  = shift;
$cvsdirectory = shift;

system("echo mailfrom: $mailfrom");
system("echo mailto: $mailto");

system("echo user: $user");
system("echo subject: $subject");

system("echo directory: $cvsdirectory");

if (!open(LD_FD, "$last_dir_file.$my_pgrp_id")) {
    # last_dir file does not exist -> cvs add directory
    $mail = "To: $mailto
From: $mailfrom
Subject: $subject
";
    $skip = 0;
    while (<STDIN>) {
        $_ = $line;
        if( $skip ) {
          $skip = 0;
        } elsif ( /^In directory/ ) {
          $skip = 1;
        } else { $mail .= $_; }
    }
    $mail .= "-- \n$user";
    
    open(MAIL, "| $MAILER -t");
    print MAIL $mail;
    close(MAIL);
    system("echo mail sent");
	
    exit 0;
}

$last_directory = <LD_FD>;
chop($last_directory);
close(LD_FD);

$last_directory =~ s/^$cvsroot//;

if($last_directory ne $cvsdirectory) {
  # This is not last directory of commit
  open(S_FD, ">>$summary_file.$my_pgrp_id") || die "cannot open summary file $summary.$my_pgrp_id";
  while (<STDIN>) {
    last if (/^Log Message/);   # drop Log Message
    print S_FD $_;
  }
  print S_FD "\n";
  close(S_FD);
  exit 0;

} else {
  # This is last directory of commit
  @text = ();
  open(S_FD, ">>$summary_file.$my_pgrp_id") || die "cannot open summary file $summary.$my_pgrp_id";
  close(S_FD);
  open(S_FD, "$summary_file.$my_pgrp_id") || die "cannot open summary file $summary_file.$my_pgrp_id";
  while (<S_FD>) {
    push(@text, $_);
  }
  close(S_FD);
  while (<STDIN>) {
    if (/^Log Message/) { push(@text, "\n"); }
    push(@text, $_);
  }
  
  $mail = "To: $mailto
From: $mailfrom
Subject: $subject

";
  $skip = 0;
  foreach my $line (@text) {
    $_ = $line;
    if( $skip ) {
      $skip = 0;
    } elsif ( /^In directory/ ) {
      $skip = 1;
    } else { $mail .= "$line"; }
  }
  $mail .= "-- \n$user";

  open(MAIL, "| $MAILER -t");
  print MAIL $mail;
  close(MAIL);
  system("echo mail sent");

# make a bit cleanup here.
  unlink("$summary_file.$my_pgrp_id");
  unlink("$last_dir_file.$my_pgrp_id");

  exit 0;
}




