# Coding Challenges

Doing Coding Challenges from https://codingchallenges.fyi/challenges/intro

### MWC tool

**Use GNU compiler to compile the c program, place it in a path folder to use anywhere**

**Or use the .exe in folder to test on the .txt file**

Basic usage:
```
<exe name> -w <.txt file> 'counts the words'
<exe name> -c <.txt file> 'counts the bytes'
<exe name> -m <.txt file> 'counts the chars'
<exe name> -l <.txt file> 'counts the lines'
<exe name> <.txt file> 'counts all'
```

A simple MWC-clone, that outputs the words, bytes, chars and lines of a txt file.

### JSON-parser

**Use GNU compiler to compile...**

**Or use the .exe in folder to test on 2 test json files**

```
<exe name> <json file>
```

A JSON-parser that lexes chars into tokens, then parses them and validates correct formatting

### Compression tool

**Use GNU compiler to compile...**

**Or use the .exe in folder to test on 3 .txt files**

Basic usage:
```
<exe name> <.txt file> 'compress .txt file'
<exe name> <compressed file> -d 'decompress compressed file'
<exe name> <.txt file> -o 'compress .txt file with new file name'
```

Compression file uses Huffman encoding to compress the individual chars in a txt file so they are represented by less bits than normal.
Creates a header to decompress later.

### Loadbalancer

**Use GNU compiler to compile...**

**Or use the .exe in folder to test on words.txt**

Basic usage:
**.exe must be used on '\n' newline seperated textfiles**
```
<exe name> [optional parameters] <.txt file>
```

**-u** only prints unique words
**-sorting** specifies sorting algorithm to use (quicksort, radixsort, heapsort, mergesort)
**-random** uses (semi)randomized sorting (sorts by (semi)random hash key)