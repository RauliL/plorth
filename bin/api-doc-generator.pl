#!/usr/bin/env perl
#
# Copyright (c) 2017, Rauli Laine
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

=pod

This handy dandy Perl script is meant to be used as API documentation
generator.

When you run it in project's root directory, it will go through all the
source, extract word documentation comments from them and convert them
into Markdown. This resulting Markdown is then printed to stdout, so that
you can pipe it to docs/api.md.

=cut

use strict;
use utf8;

my %prototypes;

for my $file (glob("./src/*.cpp"))
{
  my $in_doc_comment = 0;
  my $in_takes_list = 0;
  my $in_gives_list = 0;
  my %word;

  open(my $handle, $file) or die("Could not open $file: $!");

  while (my $line = <$handle>)
  {
    chomp($line);

    if (!$in_doc_comment)
    {
      # Beginning of doc comment?
      if ($line =~ /^\s*\/\*\*\s*$/)
      {
        $in_doc_comment = 1;
      }
      next;
    }

    # End of doc comment?
    if ($line =~ /^\s*\*\/\s*$/)
    {
      $in_doc_comment = 0;
      if ($word{name})
      {
        $prototypes{$word{prototype}}{$word{name}} = {
          gives => $word{gives},
          takes => $word{takes},
          description => $word{description}
        };
        %word = {};
      }
      next;
    }

    $line =~ s/^\s*\*\s?//;

    # Beginning of word documentation?
    if ($line =~ /^\s*Word:\s*(.+)\s*$/)
    {
      $word{name} = $1;
      next;
    }

    if (!$word{name})
    {
      next;
    }

    # Prototype name?
    if ($line =~ /^\s*Prototype:\s*(.+)\s*$/)
    {
      $word{prototype} = $1;
      next;
    }

    # Gives or takes list?
    if ($line =~ /\s*(Gives|Takes):\s*$/)
    {
      if ($1 eq "Gives")
      {
        $in_gives_list = 1;
        $in_takes_list = 0;
      } else {
        $in_gives_list = 0;
        $in_takes_list = 1;
      }
      next;
    }

    # Entry for gives or takes list?
    if ($line =~ /^\s*-\s*(.+)\s*$/ && ($in_takes_list || $in_gives_list))
    {
      push(@{$in_takes_list ? $word{takes} : $word{gives}}, $1);
      next;
    }

    if ($line =~ /^\s*$/)
    {
      if (length($word{description}) > 0 && $word{description} !~ /\n\n$/)
      {
        $word{description} .= "\n";
      }
      next;
    }

    if ($line =~ /^ {4}/)
    {
      $word{description} .= "\n";
    }

    $word{description} .= "$line\n";
  }

  close($handle);
}

print "# API reference\n\n<!-- This file is automatically generated. Do not edit by hand. -->\n\n";

for my $prototype (sort(keys(%prototypes)))
{
  my %words = %{$prototypes{$prototype}};

  if (!$prototype)
  {
    $prototype = "Global dictionary";
  }
  print "## $prototype\n\n";
  for my $word_name (sort(keys(%words)))
  {
    my %word = %{$words{$word_name}};

    print "---\n\n### $word_name\n\n";
    if ($word{takes} || $word{gives})
    {
      print "<dl>\n";
      if ($word{takes})
      {
        print "  <dt>Takes:</dt>\n";
        print "  <dd>" . join(", ", @{$word{takes}}) . "</dd>\n";
      }
      if ($word{gives})
      {
        print "  <dt>Gives:</dt>\n";
        print "  <dd>" . join(", ", @{$word{gives}}) . "</dd>\n";
      }
      print "</dl>\n\n";
    }
    if ($word{description})
    {
      print "$word{description}\n";
    }
  }
}
