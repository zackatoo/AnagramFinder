/*******************************************************************************
 * Name          : anagramfinder.cpp
 * Author        : Zack Schieberl
 * Version       : 1.0
 * Date          : November 10th, 2019
 * Last modified : November 15th, 2019
 * Description   : Finds anagrams using a user provided dictionary
 * Pledge        : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/

/*
 * ASSUMPTIONS
 *  - Single newline character between each word (no other/extra whitespace).
 *  - Valid words are only between A-Z and a-z. Anything else gets ignored.
 *  - No valigrind (stdio.h does not free all of their file allocations, so I will always be 6 frees short).
 */

/*
 * OVERVIEW
 *  - Opens the file and reads contents into dictionary struct
 *      - Goes character by character making necessary malloc calls to store strings
 *      - Assumes most strings will be 10 characters or less to make optimizations
 *      - Ignores any strings which are a single character since they cannot have anagrams
 *      - Ignores any strings which are not completely alphabetic
 * - Creates a duplicate dictionary which all strings are sorted in alphabetic order
 *      - Makes the mallocs in the createDuplicateDictionary function. Expected to free later using freeDictionary().
 *      - Uses insertion sort, best for small amount of data
 * - Finds anagrams based off if two sorted strings are the same
 *      - Stores anagrams in a vector of Anagram structs
 *      - Each anagram struct is sorted alphabetically relaive to the other words
 * - Prints the largest set of anagrams character by character
 */ 

#include <iostream>
#include <cstring>
#include <vector>
#include <stdio.h>

using namespace std;

#define MAX_WORDS 100000
#define MAX_CHARS 30
#define SMALL_CHARS 10

// Simple string immplementation
struct Word {
    char* chars;

    Word(char* _chars)
    {
        chars = _chars;
    }
};

// Array of all of the strings with a given length
struct Width {
    vector<Word> words;
};

// Array of all of the given lengths. This has every word in the dictionary
struct Dictionary {
    Width** widths;
    unsigned char length = 0;
};

struct Anagram {
    // The words in these anagrams are not malloc-ed so do not free them, they belong to a dictionary
    // These words are stored in alphabetic order relative to each other
    vector<Word> words;
    unsigned char length;

    void insert(Word newWord)
    {
        // Inserts newWord into the words vector so that words is in alphabetical order
        size_t size = words.size() - 1;
        for (size_t i = 0; i <= size; ++i)
        {
            bool insert = true;
            char* chars = words[i].chars;

            for (unsigned char j = 0; j < length; ++j)
            {
                if (chars[j] < newWord.chars[j])
                {
                    insert = false;
                    break;
                }
                else if (chars[j] > newWord.chars[j])
                {
                    break;
                }
            }
            if (insert)
            {
                // Writing a custom insert method could be faster, but I'm tired.
                words.insert(words.begin() + i, newWord);
                break;
            }
            if (i == size)
            {
                words.insert(words.end(), newWord);
                break;
            }
        }
    }
};

unsigned char min(unsigned char x, unsigned char y)
{
    return x < y ? x : y;
}

void insert(vector<Anagram>& allAnagrams, const Anagram& toInsert)
{
    // Inserts an anagram into a vector in alphabetical order based off the first word in the anagram
    size_t theSize = allAnagrams.size();
    if (theSize == 0) 
    {
        allAnagrams.push_back(toInsert);
    }
    else
    {
        --theSize;
        for (size_t i = 0; i <= theSize; ++i)
        {
            bool insert = true;
            bool same = true;
            char* chars = allAnagrams[i].words[0].chars;
            char* insertChars = toInsert.words[0].chars;
            for (unsigned char j = 0; j < min(allAnagrams[i].length, toInsert.length); ++j)
            {
                if (chars[j] < insertChars[j])
                {
                    insert = false;
                    same = false;
                    break;
                }
                else if (chars[j] > insertChars[j])
                {
                    same = false;
                    break;
                }
            }
            if (insert && same && allAnagrams[i].length < toInsert.length)
            {
                // If the words are the same and insert's length is larger, then run again to see if it should insert later
                if (i == theSize)
                {
                    allAnagrams.insert(allAnagrams.end(), toInsert);
                }
                continue;
            }
            else if (insert && same && allAnagrams[i].length > toInsert.length)
            {
                allAnagrams.insert(allAnagrams.begin() + i - 1, toInsert);
                break;
            }
            else if (insert)
            {
                allAnagrams.insert(allAnagrams.begin() + i, toInsert);
                break;
            }
            
            if (i == theSize)
            {
                allAnagrams.insert(allAnagrams.end(), toInsert);
            }
        }
    }
}

void findAnagrams(const vector<Word>& words, const vector<Word>& sortedWords, const unsigned char length, vector<Anagram>& allAnagrams, size_t& maxAnagrams)
{
    constexpr char mask_toLower = 1 << 5;
    const size_t size = words.size();
    const int intSize = sizeof(int);
    bool isAnagram = true;
    bool dupAnagrams = true;

    // Visisted size is ceil(size / intSize)
    const int visitedSize = size % intSize == 0 ? size / intSize : size / intSize + 1;

    // Visited is a bit vector of the anagrams which have already been found so we don't touch them again
    // The () at the end of the array just zeros the array
    int* visited = new int[visitedSize]();

    for (size_t i = 0; i < size; ++i)
    {
        // If this current word was already found as an anagram of another word, no point in checking it against all other words
        if (visited[i / intSize] & (1 << (i % intSize))) continue;

        Anagram anagram;
        Word current = sortedWords[i];
        dupAnagrams = true;
        for (size_t j = i + 1; j < size; ++j)
        {
            // No point in checking it against this word
            if (visited[j / intSize] & (1 << (j % intSize))) continue;

            char* target = current.chars;
            char* compare = sortedWords[j].chars;
            isAnagram = true;
            for (unsigned char k = 0; k < length; ++k)
            {
                if ((target[k] | mask_toLower) != (compare[k] | mask_toLower))
                {
                    isAnagram = false;
                    break;
                }
            }

            if (isAnagram)
            {
                if (dupAnagrams) 
                {
                    // push_back for the first time because insert doesn't work with no elements in the array
                    anagram.words.push_back(words[i]);
                    anagram.length = length;
                    dupAnagrams = false;
                }
                anagram.insert(words[j]);

                visited[j / intSize] |= 1 << (j % intSize);
            }
        }
        size_t anagramSize = anagram.words.size();
        if (anagramSize != 0)
        {
            if (anagramSize > maxAnagrams)
            {
                maxAnagrams = anagramSize;
                allAnagrams.clear();
                insert(allAnagrams, anagram);
            }
            else if (anagramSize == maxAnagrams)
            {
                insert(allAnagrams, anagram);
            }
        }
    }

    delete[] visited;
}

vector<Anagram> findAnagrams(Dictionary* dic, Dictionary* sorted, size_t& maxAnagrams)
{
    Width** widths = dic->widths;
    Width** sortedWidths = sorted->widths;
    vector<Anagram> allAnagrams;
    for (int i = 0; i <= sorted->length; ++i)
    {
        if (widths[i] != nullptr)
        {
            findAnagrams(widths[i]->words, sortedWidths[i]->words, i, allAnagrams, maxAnagrams);
        }
    }
    return allAnagrams;
}

void createSortedDictionary(Dictionary* dic, Dictionary* sorted)
{
    // Creates a duplicate dictionary in which each word is sorted in alphabetical order
    for (unsigned char i = 0; i <= dic->length; ++i)
    {
        if (dic->widths[i] != nullptr)
        {
            sorted->widths[i] = new Width;
            vector<Word> sortedWords = sorted->widths[i]->words;
            vector<Word> unsortedWords = dic->widths[i]->words;
            size_t size = unsortedWords.size();
            sortedWords.reserve(size);
            
            for (size_t j = 0; j < size; ++j)
            {
                // Perform insertion sort for each word to make it alphabetical
                char* chars = (char*)malloc(i);
                char* unsortedChars = unsortedWords[j].chars;
                chars[0] = unsortedChars[0];
                for (unsigned char k = 1; k < i; ++k)
                {
                    chars[k] = unsortedChars[k];
                    char temp;
                    for (unsigned char l = k; l > 0 && chars[l-1] > chars[l]; --l)
                    {
                        temp = chars[l];
                        chars[l] = chars[l-1];
                        chars[l-1] = temp;
                    }
                }

                sortedWords.push_back(Word(chars));
            }
            sorted->widths[i]->words = sortedWords;
        }
    }
}

void printAnagrams(const vector<Anagram>& allAnagrams, size_t maxAnagrams)
{
    size_t numAnagrams = allAnagrams.size();
    if (numAnagrams == 0)
    {
        cout << "No anagrams found." << endl;
        return;
    }
    
    const char* s = "Max anagrams: ";
    for (unsigned char i = 0; i < 14; ++i)
    {
        putchar(s[i]);
    }

    // Since there are at max 100,000 words we can use 5 characters to store it
    char number[5];
    unsigned char index = 5;
    do
    {
        --index;
        number[index] = (maxAnagrams % 10) + '0';
    } while (maxAnagrams /= 10);
    
    for (unsigned char i = index; i < 5; ++i)
    {
        putchar(number[i]);
    }
    putchar('\n');

    --numAnagrams;
    for (size_t i = 0; i <= numAnagrams; ++i)
    {
        size_t thisSize = allAnagrams[i].words.size() - 1;
        for (size_t j = 0; j <= thisSize; ++j)
        {
            char* chars = allAnagrams[i].words[j].chars;
            for (unsigned char k = 0; k < allAnagrams[i].length; ++k)
            {
                putchar(chars[k]);
            }
            if (!(i == numAnagrams && j == thisSize)) putchar('\n');
        }
        // This if statement allows for good branch prediction because it only runs once in the program
        if (i != numAnagrams) putchar('\n');
    }
}

void readFile(FILE* file, Dictionary* dic)
{
    char c = getc(file);

    // Almost all words are expected to be less than 10 characters so it is more efficent to declare a buffer of size 10
    // But words could be over 10 so then we reallocate to a 30 character buffer instead
    // This will be more efficent if a majority of the words are less than 10 characters
    char* buffer = (char*)malloc(SMALL_CHARS);
    char* bigBuffer = nullptr;
    bool valid = true;
    bool big = false;

    char mask_toLower = 1 << 5;

    unsigned char length = 0;
    while (c != EOF)
    {
        if (c == '\r')
        {
            c = getc(file);
            continue;
        }
        if (c == '\n')
        {
            if (valid)
            {
                if (dic->widths[length] == nullptr)
                {
                    // Allocate a new width if it hasn't been seen before
                    dic->widths[length] = new Width;
                    if (length > dic->length)
                    {
                        dic->length = length;
                    }
                }
                if (big)
                {
                    dic->widths[length]->words.push_back(Word(bigBuffer));
                    big = false;
                }
                else
                {
                    dic->widths[length]->words.push_back(Word(buffer));
                    buffer = (char*)malloc(SMALL_CHARS);
                }
            }
            else
            {
                valid = true;
            }
            length = -1;
        }
        else if (valid)
        {
            char temp = c | mask_toLower;
            if (temp < 'a' || temp > 'z')
            {
                // Set state to invalid and continue the loop
                valid = false;
            }
            else
            {
                if (length == SMALL_CHARS)
                {
                    // reallocate to bigBuffer
                    bigBuffer = (char*)malloc(MAX_CHARS);
                    memcpy(bigBuffer, buffer, length);
                    big = true;
                }
                if (big)
                {
                    // operate on big buffer
                    bigBuffer[length] = c;
                }
                else
                {
                    // operate on normal buffer
                    buffer[length] = c;
                }
            }
        }
        
        length++;
        c = getc(file);
    }

    // Catches last word that might not have a newline before the EOF
    if (valid && length > 1)
    {
        if (dic->widths[length] == nullptr)
        {
            dic->widths[length] = new Width;
            if (length > dic->length)
            {
                dic->length = length;
            }
        }

        if (big)
        {
            dic->widths[length]->words.push_back(Word(bigBuffer));
            // Since the last one is a big one, it needs to free the small buffer
            free(buffer);
        }
        else
        {
            dic->widths[length]->words.push_back(Word(buffer));
        }
    }
    else
    {
        free(buffer);
    }
}

void freeDictionary(Dictionary* dic)
{
    for (int i = 0; i <= dic->length; ++i)
    {
        if (dic->widths[i] != nullptr)
        {
            Width* curWidth = dic->widths[i];
            for (size_t j = 0; j < (curWidth->words).size(); ++j)
            {
                free( (curWidth->words)[j].chars );
            }
            delete curWidth;
        }
    }
    delete[] dic->widths;
}

int main(int argc, char* argv[])
{
    ios::sync_with_stdio(false);
    cin.tie(NULL);

    if (argc != 2)
    {
        cout << "Usage: ./anagramfinder <dictionary file>\n";
        return 1;
    }

    FILE* file;
    file = fopen(argv[1], "r");
    if (file == NULL)
    {
        cerr << "Error: File '" << argv[1] << "' not found.\n";
    }
    else
    {
        Dictionary dic;
        dic.widths = new Width*[MAX_CHARS + 1];
        for (unsigned char i = 0; i < MAX_CHARS + 1; ++i)
        {
            dic.widths[i] = nullptr;
        }

        readFile(file, &dic);
        fclose(file);

        Dictionary sorted;
        sorted.length = dic.length;
        sorted.widths = new Width*[dic.length + 1];
        for (unsigned char i = 0; i <= sorted.length; ++i)
        {
            sorted.widths[i] = nullptr;
        }

        createSortedDictionary(&dic, &sorted);
        size_t maxAnagrams = 0;
        vector<Anagram> ana = findAnagrams(&dic, &sorted, maxAnagrams);
        printAnagrams(ana, maxAnagrams);

        // Even though it's faster to not free the memory I guess we have to otherwise papa valgrind will be disapointed
        freeDictionary(&dic);
        freeDictionary(&sorted);
    }
}