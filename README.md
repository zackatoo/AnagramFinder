# AnagramFinder
Searches for anagrams in a file. It prints the anagram or group of anagrams which have the most alterations.

## Overview
- Opens the provided file via command line arguments and reads contents into dictionary struct.
  - Goes character by character making necessary malloc calls to store strings
  - Assumes most strings will be 10 characters or less to make optimizations
  - Ignores any strings which are a single character since they cannot have anagrams
  - Ignores any strings which are not completely alphabetic
- Creates a duplicate dictionary which all strings are sorted in alphabetic order
  - Makes the mallocs in the createDuplicateDictionary function. Expected to free later using freeDictionary().
  - Uses insertion sort, best for small amount of data
- Finds anagrams based off if two sorted strings are the same
  - Stores anagrams in a vector of Anagram structs
  - Each anagram struct is sorted alphabetically relaive to the other words
- Prints the largest set of anagrams character by character
