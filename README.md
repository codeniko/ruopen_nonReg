#RUopen? - Rutgers Course Spotter
Is the course you need to graduate closed? Or maybe you want that Easy A course that is always closed. RUopen has got you covered! RUopen spots when the section of a course you want opens, and even notifies you through text.

*Programmed by Nikolay Feldman*

**History**
Java GUI version created in ~2012 (GUI looked pretty bad with gridbag)
C# GUI version created ~2013 (Beautiful GUI)
C++ CLI version created ~2014 (Built with speed in mind and to run on server)

##Installation
**Dependencies**
The versions specified below are the versions in which RUopen was developed.
LibCurl  > *7.26.0*
Boost > *1.55.0*

To build, simply run
> make
You will end up with an executable named **ruopen**
##Configuration
The configuration file is **ruopen.conf**

**NOTE** 
If you're not using a setting or want the program to default, leave the value of the setting as an empty line. You can also remove the setting entirely.

The campus to load courses for.
<pre>
 [CAMPUS] 
 *Values: New Brunswick, Newark, Camden*
 Default: *New Brunswick*
</pre>

The school semester that you want to load courses for. This can be left blank and the current semester will be used *(either Spring or Fall)*
<pre>
 [SEMESTER] 
 *Values: <SEASON> <YEAR>*
 Examples: *Spring 2014, Summer 2013, Fall 2012, Winter 2011, etc...*
 Default: *<EMPTY LINE> signifying to use current semester (determined automatically).*
</pre>

Enter a Yahoo email that is registered to you. This is the email that will send out an SMS message to you. It is **highly** recommended to create a new email specifically for this purpose.
<pre>
 [SMS EMAIL] 
 *Value: <someEmailName@yahoo.com>*
</pre>

Enter the password to the "[SMS EMAIL] email specified above."
<pre>
 [SMS PASSWORD] 
 *Value: <Yahoo email password>*
</pre>
The mobile phone number that will be notified when your class has been spotted.
<pre>
 [SMS PHONE NUMBER]
 *Value: <##########>*

**Note** Don't have any spaces in the number
</pre>

Pre-load the courses/sections to spot for on program execution.
<pre>
 [COURSES] 
 *Values: <DEPARTMENT>:<COURSE>:<SECTION>*
 Examples: *198:111:03 or 640:250:01*
 Default: *<EMPTY LINE> signifying not to spot any courses (can be set within program).*

 **NOTE** Have each course on a separate line with no empty lines inbetween.
</pre>


