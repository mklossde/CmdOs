
# CmdOs Filesystem

As default a SPIFF filesystem is used


## fsDir MATCH
list the filesystem to log
with MATCH list only fiel that match (e.g. fsDir .gif => show all gif-files)

## fsDirSize MATCH
get number of files 
with MATCH get numbner of files that match (e.g. fsDirSize *.cmd => numer of cmd-files)

## fsFile MATCH INDEX TYPE
get info from INDEX file that MATCH (e.g. fsFile .gif 0 0 => get name of first gif-file)
	TYPE=0 NAME
	TYPE=1 SIZE


## fsCat FILE
cat the FILE to log

## fsWrite FILE MESSAGE
write MESSAGE into FILE

## fsDel FILE
delete FILE

## fsRen FILE NEWFILE
rename FILE to NEWFILE

## fsDownload URL FILE
deonload contetn of URL into [FILE] 
e.g. fsDownload https://www.w3.org/Icons/64x64/home.gif  => fill load url into file "home.gif"

## fsFormat 
format filesystem	

![LOGO](images/CmdOS_logo.gif) a OpenOn.org project - develop by mk@almi.de 

 