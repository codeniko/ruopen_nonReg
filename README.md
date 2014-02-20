#RUopen? - Rutgers Course Spotter
Is the course you need to graduate closed? Or maybe you want that Easy A class that is always closed. RUopen has got you covered! RUopen spots when the section of a course you want opens, and even notifies you through text.

*Programmed by Nikolay Feldman*

**History**<br>
Java GUI version created in ~2012 *(GUI looked pretty bad with gridbag)*<br>
C# GUI version created ~2013 *(Beautiful GUI)*<br>
C++ CLI version created ~2014 *(Built with speed in mind and to run on server)*

##Installation
**Dependencies**<br>
The versions specified below are the versions in which RUopen was developed.<br>
LibCurl  > *7.26.0*<br>
Boost > *1.55.0*

To build, simply run
> make

You will end up with a binary file named **ruopen**
##Configuration
The configuration file is **ruopen.conf**

**NOTE** 
If you're not using a setting or want the program to default, leave the value of the setting as an empty line. You can also remove the setting entirely.

The campus to load courses for.
<pre>
 [CAMPUS] 
 Values: <i>New Brunswick, Newark, Camden</i>
 Default: <i>New Brunswick</i>
</pre>

The school semester that you want to load courses for. This can be left blank and the current semester will be used *(either Spring or Fall)*
<pre>
 [SEMESTER] 
 Values: <i>&lt;SEASON&gt; &lt;YEAR&gt;</i>
 Examples: <i>Spring 2014, Summer 2013, Fall 2012, Winter 2011, etc...</i>
 Default: <i>&lt;EMPTY LINE&gt; signifying to use current semester (determined automatically).</i>
</pre>

Enter a Yahoo email that is registered to you. This is the email that will send out an SMS message to you. It is **highly** recommended to create a new email specifically for this purpose.
<pre>
 [SMS EMAIL] 
 Value: <i>&lt;someEmailName@yahoo.com&gt;</i>
</pre>

Enter the password to the "[SMS EMAIL] email specified above."
<pre>
 [SMS PASSWORD] 
 Value: <i>&lt;Yahoo email password&gt;</i>
</pre>
The mobile phone number that will be notified when your class has been spotted.
<pre>
 [SMS PHONE NUMBER]
 Value: <i>&lt;##########&gt;</i>

<b>NOTE</b> Don't have any spaces in the number
</pre>

Should the program display its progress and course checks (to stdout), or should it be silent. If silent == true, an alert and an SMS message will still be sent when a course is spotted.
<pre>
 [SILENT]
 Values: <i>true, false</i>
 Default: <i>false</i>
</pre>

Pre-load the courses/sections to spot for on program execution.
<pre>
 [COURSES] 
 Values: <i>&lt;DEPARTMENT&gt;:&lt;COURSE&gt;:&lt;SECTION&gt;</i>
 Examples: <i>198:111:03 or 640:250:01</i>
 Default: <i>&lt;EMPTY LINE&gt; signifying not to spot any courses (can be set within program).</i>

 <b>NOTE</b> Have each course on a separate line with no empty lines inbetween.
</pre>


