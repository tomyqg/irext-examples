libSearch.exe

Created by: 	Matt Kracht
Last Updated: 	1/9/2015
Version:	1.0.3

********************************************************************************

usage: libSearch.exe [-h] cfg xml libdir libloc [{FlashOnly,FlashROM}]

Used in conjunction with LPRF Stack Releases as an IAR prebuild step. Parses
provided .opt file and adds required libraries based on parameters.xml to project.

positional arguments:
  cfg                   Project local configuration file for Stack Project
  xml                   Generic template file that describes defines to match
                        against in cfg
  libdir                Directory containing library files
  libloc                Project local library file
  {FlashOnly,FlashROM}  Select whether the configuration requires FlashOnly or
                        FlashROM library. Defaults to FlashROM

optional arguments:
  -h, --help            show this help message and exit
