# OwlConsole

## Command-Line Options

```
Usage: owlconsole [options]


```

## What is OwlConsole?
OwlConsole is a terminal application that allows users to browse message boards through the command line. OwlConsole uses the same backend as Owl and therefore supports the same variety of message board software.

## How do I use it?

OwlConsole supports two 

OwlConsole creates a psuedo-filesystem hiearchy of the message board that allow users to navigate the message board by selecting folders and threads in much the same way one may selected folders and files in a filesystem console.

## Terminal Commands (Quick Reference)

### Logging In

    Usage: login [url] {username} {password} [--parser=<parser-name>]

### Displaying Parsers

    Usage: parsers 

### List Forums

    Usage: forums [--ids|-i]

The `forums` command lists all of the sub-forums of the current forum. Forum IDs can be shown by passing the `--id` switch. This command has the alias `lf`.

### List Threads 

    Usage: threads <page-number> <per-page> [--ids|-i] [--stickies|-s] [--times|-t]

The `page-number` is calculated by the `per-page` parameter. If `per-page` is not specified then the default is 10. The default for `page-number` is one. By default, the IDs of the threads are not shown, but can be shown with the `--ids` parameter. Also, sticky threads will not be shown by default, but can be shown with the `--stickies` option. By default only the date of the last post is shown but users can display the time with the `--times` option. This command has the alias `lt`.

Example: 
```
[wtf.com] /Sparta!/Life Sucks> lt 1 5 --ids --stickies
1> *Life Sucks - The Rules! [17745] (1) - 2010-1-13 by Jason
2> *Trash thieves... [48103] (19) - 2016-8-12 by -=iNsANe=-ADJ
3. *height concerns [48105] (27) - 2016-8-9 by YUCK FOU!!!
4. *Getting a lot off my chest [47922] (9) - 2016-8-8 by BRiT
5. *Can anyone relate to my situation ? [48089] (12) - 2016-8-3 by CoprophagousCop
```
The above example displays the first 5 threads, including stickies and IDs. 

There is a good deal of information packed onto a single line. To explain the different parts consider the highlighted message below:

<span style="font-size:14px; font-family: Menlo"><span style="background-color:pink">2</span><span style="background-color:#00FF00">&gt;</span> <span style="background-color:yellow">*</span><span style="background-color:#E69138">Trash thieves...</span> <span style="background-color:#00FFFF">[48103]</span> <span style="background-color:#CCCCCC">(19)</span> <span style="background-color:#FFE599">2016-8-12</span> by <span style="background-color:#FF00FF">-=iNsANe=-ADJ</span></span>

|Value    |Definition|
|---------|----------|
|<span style="background-color:pink">&nbsp;2&nbsp;</span>|The 1-based index position of the post.|
|<span style="background-color:#00FF00">&nbsp;&gt;&nbsp;</span>| This value will be either `.` or `>`. A `>` indicates the thread is sticky and a value of `.` indicates a normal threads.|
|<span style="background-color:yellow">*</span>| The `*` indicates the thread has unread posts. If there are no unread posts then this is omitted.|
|<span style="background-color:#E69138">Trash thieves...</span>| The title of the thread.|
|<span style="background-color:#00FFFF">[48103]</span>| The thread id if `--ids` is specified.|
|<span style="background-color:#CCCCCC">(19)</span>| The number of replies to the thread.|
|<span style="background-color:#FFE599">2016-8-12</span>| The date of the last post in the thread (time ommitted in this example).|
|<span style="background-color:#FF00FF">-=iNsANe=-ADJ</span>| The author of the last post in the thread|

### List Posts 

    Usage: posts <page-number> <per-page> [--ids|-i]

### Application Settings

Settings can be set either through the command line interface or the JSON file. 

`save [filename]`<br/>

`load [filename]`<br/>

`set <setting> <value>`<br/>

`list`<br/>

### Thread and Post Template

When creating a new thread or post, Owl will generate a template for the user to fill out in the configured text editor. The template contains fields to define the subject, tags and text of the thread or post. The text editor used can be configure (see *Configuring Default Text Editor* below).

Below is the template with some sample content.

```
# All lines beginning with '#' are comments if they appear above the post 
# marker ("----"). 

# The text after "Title:" will be the thread or post's title. The entire title
# must be on the same line. Any whitespace at the beginning or the end of the
# title is trimmed.
Title: Leaving for Alaska today!

# Tags can be entered on a single line seperated by a comma. All tags must be on a 
# single line. Any whitespace at the beginning or end of a tag is trimmed.
Tags: thursday, alaska, travel

# The "Sticky" setting only applies to new threads and is ignored for posts. The 
# default value is 'false'. Valid true values are `true|on|yes|1` and valid false
# values are `false|off|no|0`. If the user attempts to create a sticky thread but
# does not have the correct permissions the thread may not be created.
Sticky: false

# The post marker "----" below indicates the beginning of the post. All text below 
# this marker is considered part of the post. All whitespace, including empty lines, 
# is preserved.
----
Hi everyone! Today is the day I've been waiting for for the past year. The family and I are leaving for Alask! :woohoo:

We will fly into Anchorage where we will spend three days. Then we are driving up to Denali where we will camp for another three days. Then to Fairbanks, then Juneau and then back home!

I will make sure to post pictures when I get back. Have a great week!
```

This template is very verbose with comments. The comments can be turned off by setting the app-setting `editor.template.verbose` to `false`, which case the template will appear like so:

```
Title: Leaving for Alaska today!
Tags: thursday, alaska, travel
Sticky: false
----
Hi everyone! Today is the day I've been waiting for for the past year. The family and I are leaving for Alask! :woohoo:

We will fly into Anchorage where we will spend three days. Then we are driving up to Denali where we will camp for another three days. Then to Fairbanks, then Juneau and then back home!

I will make sure to post pictures when I get back. Have a great week!
```

```
Title: 
Tags: 
Sticky: false
----

```

### Configuring Default Text Editor

The default on macOS and Linus is vi. The default on Windows is Notepad.

The command to start vi may look like: `vi +:cal\ cursor(%2,%3) %1`. The arguments are:

`%1` - the temporary filename Owl uses to save and parse the post.
`%2` - the line number where the cursor should be placed when the editor loads
`%2` - the column number where the cursor should be placed when the editor loads