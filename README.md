# Quieso

<img src="./images/cheese.png" style="zoom:20%;" />

Quieso is simple port scanner written in C. It do not rely on high level libraries for scanning. This was mainly written for beginners to have idea about port scanning in C. All over Internet, I found a lack of port scanning code in C. Most programs are obsolete and don't even work. There are great open source scanner like nmap or rust scan but that code can not be understood by beginners. So this is simple code written towards people searching for write port scanner in C.



## Dependencies

Quieso is made to be very light. It had some dependencies

- pthread.h
- argp.h

*Most of these libraries comes pre-installed on many Linux Distributions but still if  you got some error than refer to your Linux Distribution Documentation.*



## Some Notes

Quieso is still in steady development for more efficient and readable code. Practice will be to made existing code more understandable to beginners and will keep it working.

**Do not use this tool on work place or professional purpose. This is just a minimal scanner. Use nmap or rust scan instead.**

More importantly scanning website or server without owner's permission is illegal. Use it on your own machine or [scanme.nmap.org](scanme.nmap.org/).
