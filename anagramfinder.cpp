/*******************************************************************************
 * Name          : anagramfinder.cpp
 * Author        : Zack Schieberl
 * Version       : 1.0
 * Date          : November 10th, 2019
 * Last modified : November 15th, 2019
 * Description   : Finds anagrams using a user provided dictionary
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
 */ 

#include <iostream>
#include <cstring>
#include <vector>
#include <stdio.h>

using namespace std;

#define MAX_WORDS 100000
#define MAX_CHARS 30
#define SMALL_CHARS 10

void print(char* str, unsigned char len)
{
    for (unsigned char i = 0; i < len; i++)
    {
        cout << str[i];
    }
    cout << endl;
}

// Ghetto (no hr pls) string immplementation
struct Word {
    char* chars;
    unsigned char len;

    Word(char* _chars, unsigned char l)
    {
        chars = _chars;
        len = l;
    }

    void sprint()
    {
        print(chars, len);
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
    // These words are not stored in any sorted order
    vector<Word> words;
};

void findAnagrams(const vector<Word>& words, const vector<Word>& sortedWords, const unsigned char length, vector<Anagram>& allAnagrams)
{
    constexpr char mask_toLower = 1 << 5;
    const size_t size = words.size();
    const int intSize = sizeof(int);
    bool isAnagram = true;

    // Visisted size is ceil(size / intSize)
    const int visitedSize = size % intSize == 0 ? size / intSize : size / intSize + 1;

    // Visited is a bit vector of the anagrams which have already been found so we don't touch them again
    // The () at the end of the array just zeros the array
    int* visited = new int[visitedSize]();

    for (size_t i = 0; i < size; ++i)
    {
        // If this current word was already found as an anagram of another word, no point in checking it against all other words
        if (visited[i / intSize] & (1 << i % intSize)) continue;

        Anagram anagram;
        Word current = sortedWords[i];
        for (size_t j = i + 1; j < size; ++j)
        {
            // If this current word was already found as an anagram of another word, no point in checking it against the word
            if (visited[j / intSize] & (1 << j % intSize)) continue;

            char* target = current.chars;
            char* compare = sortedWords[j].chars;
            isAnagram = true;
            for (unsigned char k = 0; k < length; ++k)
            {
                if (target[k] != compare[k])
                {
                    isAnagram = false;
                    break;
                }
            }

            if (isAnagram)
            {
                anagram.words.push_back(words[i]);
                visited[j / intSize] &= 1 << j % intSize;
            }
        }
        if (anagram.words.size() != 0)
        {
            allAnagrams.push_back(anagram);
        }
    }

    delete[] visited;
}

vector<vector<Anagram>> findAnagrams(Dictionary* dic, Dictionary* sorted, vector<size_t>& sizes, size_t& numAnagrams)
{
    Width** widths = dic->widths;
    Width** sortedWidths = sorted->widths;
    vector< vector<Anagram> > myAnagrams;
    for (int i = 0; i <= sorted->length; ++i)
    {
        if (widths[i] != nullptr)
        {
            vector<Anagram> allAnagrams;
            findAnagrams(widths[i]->words, sortedWidths[i]->words, i, allAnagrams);
            myAnagrams.push_back(allAnagrams);
            sizes.push_back(allAnagrams.size());
            numAnagrams += allAnagrams.size();
        }
    }
    return myAnagrams;
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
                //TODO: remove all affiliations of len
                sortedWords.push_back(Word(chars, unsortedWords[j].len));
            }
            sorted->widths[i]->words = sortedWords;
            
            for (size_t j = 0; j < size; ++j)
            {
                sorted->widths[i]->words[j].sprint();
            }
        }
    }
}

void printDictionary(Dictionary* dic)
{
    // Debug purposes
    for (int i = 0; i <= dic->length; ++i)
    {
        if (dic->widths[i] != nullptr)
        {
            Width* curWidth = dic->widths[i];
            cout << "\nLength: " << i << endl;
            for (size_t j = 0; j < curWidth->words.size(); ++j)
            {
                curWidth->words[j].sprint();
            }
        }
    }
}

void printAnagrams(const vector<vector<Anagram>>& allAnagrams, const vector<size_t>& sizes, size_t numAnagrams)
{
    if (allAnagrams.size() == 0)
    {
        cout << "No anagrams found." << endl;
        return;
    }
    
    const char* s = "Max Anagrams: ";
    for (unsigned char i = 0; i < 13; ++i)
    {
        putchar(s[i]);
    }

    // Since there are at max 100,000 words we can use 5 characters to store it
    char number[5];
    unsigned char index = 5;
    do
    {
        --index;
        number[index] = (numAnagrams % 10) + '0';
    } while (numAnagrams /= 10);
    
    for (unsigned char i = index; i < 5; ++i)
    {
        putchar(number[i]);
    }
    putchar('\n');

    for (size_t i = 0; i < sizes.size(); ++i)
    {
        for (size_t k = 0; k < allAnagrams[i].size(); ++k)
        {
            for (size_t l = 0; l < allAnagrams[i][k].words.size(); ++l)
            {
                char* str = allAnagrams[i][k].words[l].chars;
                for (unsigned char j = 0; j < sizes[i]; ++j)
                {
                    putchar(str[j]);
                }
                putchar('\n');
            }
            putchar('\n');
        }
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
                    dic->widths[length]->words.push_back(Word(bigBuffer, length));
                    big = false;
                }
                else
                {
                    dic->widths[length]->words.push_back(Word(buffer, length));
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
            dic->widths[length]->words.push_back(Word(bigBuffer, length));
            // Since the last one is a big one, it needs to free the small buffer
            free(buffer);
        }
        else
        {
            dic->widths[length]->words.push_back(Word(buffer, length));
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
        cout << "Usage: ./anagramfinder <dictionary.txt>\n";
    }

    FILE* file;
    file = fopen(argv[1], "r");
    if (file == NULL)
    {
        cerr << "Error opening file.\n";
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

        printDictionary(&dic);
        printDictionary(&sorted);
        size_t numAnagrams = 0;
        vector<size_t> sizes;
        auto ana = findAnagrams(&dic, &sorted, sizes, numAnagrams);
        printAnagrams(ana, sizes, numAnagrams);
        freeDictionary(&dic);
        freeDictionary(&sorted);
    }
}