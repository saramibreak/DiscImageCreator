#ifdef _WIN32
// https://stackoverflow.com/questions/42354008/strcasestr-still-not-working/45194545#45194545
char* strcasestr(const char* haystack, const char* needle) {
    /* Edge case: The empty string is a substring of everything. */
    if (!needle[0]) return (char*)haystack;

    /* Loop over all possible start positions. */
    for (size_t i = 0; haystack[i]; i++) {
        bool matches = true;
        /* See if the string matches here. */
        for (size_t j = 0; needle[j]; j++) {
            /* If we're out of room in the haystack, give up. */
            if (!haystack[i + j]) return NULL;

            /* If there's a character mismatch, the needle doesn't fit here. */
            if (tolower((unsigned char)needle[j]) !=
                tolower((unsigned char)haystack[i + j])) {
                matches = false;
                break;
            }
        }
        if (matches) return (char*)(haystack + i);
    }
    return NULL;
}
#endif
