#!/usr/bin/perl -w
# script scans system log for invalid users ssh reports ( ssh attacks ) & saves IPs of attackers in file
# using: script <result IP's file> [report file]
#  Sergey Kibrik sakib@meta.ua 2009
#
use strict;
use warnings;
#################
my $logfile="/var/log/messages";
my $resultfile=shift;
my $report=shift;
my $max_ip_count=30;
#################

my $errno=0;
my $report_msg="";
my %ip_table;

unless ($report){
	$report=">&STDOUT";
}

open(REPORT,">",$report) or die "Failed to open reporting routine: $!\n";
unless ( open(LOGFILE,"<",$logfile)){
	$report_msg .= "Failed to open $logfile: $!\n";
	$errno=1;
	goto QUIT;
}
unless ( open(RESULTFILE, ">", $resultfile)){
	$report_msg .= "Failed to open $resultfile: $!\n";
	$errno=1;
	goto QUIT;
}
while (<LOGFILE>){
next unless /sshd.*Invalid\s+user\s+(\w+)\s+from\s+(.+)$/;
	++$ip_table{$2};
}
foreach (keys %ip_table){
	if ($ip_table{$_} >= $max_ip_count) {
		print RESULTFILE $_,"\n";
	}
}
close(LOGFILE);
close(RESULTFILE);

QUIT:
print REPORT $report_msg;
close(REPORT);
exit $errno;
