#!/usr/bin/perl -w

# Confirm that perl is located in the /usr/bin/perl folder on the server

#All perl scripts should use strict
use strict;

use CGI;
my $cgi_object = new CGI();

# Print out the http header 
print $cgi_object->header();


# Retrieve the value of the skey parameter 
my $input = $cgi_object->param('author');

# This is the hashtable storing a sample of archived articles in HTML format 
my @articles = (
    { searchkey => "GW",
      content => "<h1>The Shadow of the Torturer</h1>
                  <h2>by Gene Wolfe</h2>
                  <p><b>In Short:</b> The first part of the classic 4-part series
                     set in a distant fantastical future</p>
                  <p>4 Stars</p>
                  <p>Set on Earth (Urth) perhaps millions of years into the future; 
                     Gene Wolfe introduces us to one of the most interesting 
                     characters in the history of science fiction, a young 
                     torturer named Severian. Severian serves out his apprenticeship 
                     in the Matachin Tower, which astute readers will realize is a 
                     grounded spaceship (Severian is unaware of this and unaware of 
                     other artifacts of a long-dead powerful civilization.) But the 
                     residues of technology are secondary in interest to the journeys 
                     of Severian, as he wanders Urth after being exiled for committing 
                     the crime of mercy.</p>

                   <p>Wolfe draws upon a rich literary tradition in constructing his 
                      novel. The opening chapter is reminiscent of the opening of 
                      <em>Great Expectations</em> and I could spot dozens of classic 
                      allusions scattered across each chapter. Wolfe's language is 
                      exquisite. With many terms based on Latin or Greek, all with a
                      phenomenal rightness to what they identify or - often - suggest.
                      Badelaire, lansquenet, amchasphand, chrisos, orichalk, pinakothek, 
                      salpinx, ephor ... and the tricky thing is that every now and then 
                      one of them is a real word . The novel highlights Wolfe's wonderful 
                      command of language and enriches the tapestry of this deep 
                      and penetrating work.</p>

                   <p>Don't be put off by that preceding paragraph. The novel is as fun, 
                      exciting, and engrossing as any potboiler. Note that this is the 
                      first book in the series, The Book of the New Sun. I know of no 
                      better or more literate series in the history of science fiction.
                   </p>"
    },
    { searchkey => "RAH",
      content => "<h1>The Moon is a Harsh Mistress</h1>
                  <h2>by Robert Heinlein</h2>
                  <p><b>In Short:</b> Perhaps the greatest novel from the Grandmaster of
                     Science Fiction and thus perhaps the greatest science fiction
                     novel period.</p>
                  <p>4 Stars</p>
                  <p>The idea of a colony on the Moon or Mars or Alpha Centauri 
                     staging a revolution against Earth was hardly new when 
                     Heinlein wrote <em>The Moon is a Harsh Mistress</em> in the 60's. 
                     Yet, this novel, perhaps Heinlein's greatest, never feels staged or 
                     old or tiresome. Why is that?</p>
                  <p>I think it's because the novel is <em>not</em> really about the 
                     revolution. That's just a stage props. The novel is really about 
                     economics ... no, wait, it's about artificial life ... hold on, it's 
                     really about sexual mores, not wait it's about the nature of humor, no .. 
                     wait ...</p>
                  <p>No, you see, <em>The Moon is a Harsh Mistress</em> is about all of 
                     those things and many more. Heinlein uses his time and talent to 
                     explore so many facets of the human condition that each page is 
                     littered with engrossing ideas and discussions. Some of those ideas 
                     are engaging, some are illuminating, some are enraging, but none are 
                     boring.</p>
                  <p>In this novel, Heinlein introduced the famous concept - TANSTAAFL - 
                     (there 'aint no such thing as a free lunch) (a phrase that has entered 
                     the English lexicon and penetrated the American psyche) is particularly 
                     evident on the moon, where even air most be paid for. The truths of 
                     objectivism that were pioneered by Ayn Rand become very clear on 
                     Heinlein's moon - where death by suffocation is always only a few inches 
                     of steel away and every resource must be painstakingly worked for and no 
                     amount of self-pity or crying will help you stay alive. Hard work, 
                     action, creativity and thus creation are the most valuable 
                     commodities.</p>
                  <p>Please do yourself a favor and read this book. It will entertain you 
                     immensely and provide food for thought for a long time after you 
                     finish reading it.</p>"
    }
); 

# Keep track of the number of hits using the count variable
my $count = 0;
foreach my $row (@articles) {
   if ( $row->{searchkey} =~ /^$input$/i) {
      $count ++;
      print "$row->{content}"
   }
}

# If no hits, print this fact
if ($count == 0) {
   print "<h1>No book review Found</h1><p>More reviews soon to come!</p>";
}