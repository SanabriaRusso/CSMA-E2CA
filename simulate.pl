#!/usr/local/bin/perl

use warnings;
use strict;
use Switch;

die ("Getting out. You need some parameters, i.e.: --help\n") if (not exists $ARGV[0]);

chomp($ARGV[0]);

my %help = (
				help		=> 1,
				'-h'		=> 1,
				'--help'	=> 1
			);

die ("******Help\n", "ARGV. field:\n", "0. Repetitions 1. Time 2. Nmax 3. Nmin 4. Jumps 5. Bandwidth 6. Channel errors 7. protocol\n") 
	if (exists $help{$ARGV[0]});

my $rep = $ARGV[0];
my $time = $ARGV[1];
my $Nmax = $ARGV[2];
my $Nmin = $ARGV[3];
my $jump = $ARGV[4];
my $bandwidth = $ARGV[5];
my $errors = $ARGV[6];
my $protocol = $ARGV[7];
my $length = 1024;
my $batch = 1;
my ($stickiness,$hysteresis,$fairShare) = (0,0,0);
my $drift = 0;
my $shareDCF = 0;
my $maxAg = 0;

print ("Going to simulate:\n");
if ($Nmax == $Nmin){
	print ("\t$rep repetitions of $time seconds, ", $Nmin, " stations, $bandwidth Mbps, errors $errors and prot $protocol.\n\n\n");	
}else{
	print ("\t$rep repetitions jumping $jump of $time seconds, ", ($Nmax - $Nmin), " stations, $bandwidth Mbps, errors $errors and protocol $protocol.\n\n\n");
}

my $compile = './build_local';
my @command;
my @jumps;

system($compile);
die "Command failed\n" if ($? != 0);

#Simulating at $jump intervals
foreach ($Nmin .. $Nmax)
{
	push @jumps, $_
		if $_ % $jump == 0;

}
push @jumps, $Nmax
	if $jumps[-1] != $Nmax;

switch($protocol){
	case 0{
		$stickiness = 0;
		$hysteresis = 0;
		$fairShare = 0;
		$shareDCF = 1;
	}
	case 1{
		$stickiness = 1;
	}

}

OUTTER: foreach my $i (@jumps){
	INNER: foreach my $j (1 .. $rep){
		my $seed = int(rand()*1000);
		@command = ("./Sim_SlottedCSMA $time $i $length $bandwidth $batch $stickiness $hysteresis $fairShare $errors $drift $shareDCF $maxAg $seed"); 
		print ("\n\n****Node #$i of $Nmax ($?).\n");
		print ("****Iteration #$j of $rep.\n");
		print ("**** @command\n");
		system(@command);
		(print ("\n\n********Execution failed\n\tQuitting iterations\n") and last OUTTER) if ($? != 0);
	}
}


#Calling the parser
my $parserFile = 'process.pl';
my $dataFile = 'Results/multiSim.txt';
my @parseCommand = ("./$parserFile $dataFile");
system(@parseCommand);
(print ("\n\n********Processing failed\n") and last OUTTER) if ($? != 0);

