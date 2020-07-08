# Abextract - ADB backup extractor tool

## Features
abextract is a simple tool to extract backup made by android adb written in c.

It relies on tar utility for  unpacking the final tar archive.

## TODO
*Repack backup - ✅ Done! 

*Encryption

## Usage :
unpack:
        
        abextract unpack   <backup.ab>

pack:

       abextract pack   <backup.tar> <backup.ab>


## Installation
Clone the project

$ git clone https://github.com/anoop142/abextract.git

Build it!

$ make

## OR

Download precomplied binary  https://github.com/anoop142/abextract/releases
