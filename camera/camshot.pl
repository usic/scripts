#!/usr/bin/perl
# Script for taking shots using Web-camera & distributing pictures to network
#
# Every time conection to daemon is opened, picture is got and transmitted to client closing connection afterwards.
#
#  Author Sergey Kibrik sakib@meta.ua
#  USIC 2010


use strict;
use warnings;
use IO::Socket::INET;


# command that is used to take camera shot
my $cmd = "cat /tmp/webcam.jpg";

# function that handles termination signals
sub terminate;
$SIG{INT} = \&terminate;
$SIG{TERM} = \&terminate;

my $sock = IO::Socket::INET->new(
					LocalAddr => '0.0.0.0',
					LocalPort => '6666',
					Proto => 'tcp',
					Listen => 5,
					Type => SOCK_STREAM
				)	
or die "Can't bind: $@\n";

my $incoming;
while ( 1 ){
	$incoming = $sock->accept();
	if ( defined $incoming ){
	       	print $incoming qx{ $cmd };
		close( $incoming );
	}
}

#  close listening socket & terminate main loop
sub terminate {
	my $sig = shift;
	print "$sig requested. Terminating\n";
	close( $sock );
	exit( 0 );
}

# BYE!
