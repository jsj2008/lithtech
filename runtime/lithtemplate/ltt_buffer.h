#ifndef __LTT_BUFFER_H__
#define __LTT_BUFFER_H__

//c headers
#ifndef __STDIO_H__
#include <stdio.h>
#define __STDIO_H__
#endif

#ifndef __STDARG_H__
#include <stdarg.h>
#define __STDARG_H__
#endif

//a template class for a fixed length buffer class.
template <int32 buffer_len>
class CBuffer {
  public:
    CBuffer();
    //pass parametes like printf
    CBuffer(const char *format_string, ...);
    CBuffer(const char *format_string, va_list args);

    //sets the string.  pass parameters like printf.
    void Set(const char *format_string, ...);

    //returns the name as a null terminated string.
    operator const char *() const;

    //returns true if the string is not empty.
    bool IsEmpty() const ;

    //copies the given null-terminated string.  NULL or empty strings set the buffer to ""
    const char *operator=(const char *str);

    //compares us to a given string.
    bool operator==(const char *str) const;

    //makes the string lower case.
    void ToLower();

  protected:
    char buffer[buffer_len];
};

template <int32 buffer_len>
CBuffer<buffer_len>::CBuffer() {
    buffer[0] = '\0';
}

template <int32 buffer_len>
CBuffer<buffer_len>::CBuffer(const char *format_string, ...) {
    //get our parameter list.
    va_list args;
    va_start(args, format_string);

    //call _vsnprintf
    LTVSNPrintF(buffer, buffer_len, format_string, args);

    //put a null at the end.
    buffer[buffer_len - 1] = '\0';
}

template <int32 buffer_len>
CBuffer<buffer_len>::CBuffer(const char *format_string, va_list args) {
    //call _vsnprintf
    LTVSNPrintF(buffer, buffer_len, format_string, args);

    //put a null at the end.
    buffer[buffer_len - 1] = '\0';
}

template <int32 buffer_len>
void CBuffer<buffer_len>::Set(const char *format_string, ...) {
    //get our parameter list.
    va_list args;
    va_start(args, format_string);

    //call _vsnprintf
    LTVSNPrintF(buffer, buffer_len, format_string, args);

    //put a null at the end.
    buffer[buffer_len - 1] = '\0';
}

template <int32 buffer_len>
CBuffer<buffer_len>::operator const char *() const {
    return buffer;
}

template <int32 buffer_len>
bool CBuffer<buffer_len>::IsEmpty() const {
    return buffer[0] == '\0';    
}

template <int32 buffer_len>
const char *CBuffer<buffer_len>::operator=(const char *str) {
    if (str == NULL || str[0] == '\0') {
        buffer[0] = '\0';
        return buffer;
    }

    //copy the string
    strncpy(buffer, str, buffer_len - 1);

    //make the last character null.
    buffer[buffer_len - 1] = '\0';

    return buffer;
}

template <int32 buffer_len>
void CBuffer<buffer_len>::ToLower() {
    //check each character.
    for (int32 i = 0; i < buffer_len; i++) {
        //see if this one is upper case.
        if (isupper(buffer[i])) {
            //convert to lower case.
            buffer[i] = (char)tolower(buffer[i]);
        }
    }
}

template <int32 buffer_len>
bool CBuffer<buffer_len>::operator==(const char *str) const {
    if (str == NULL) {
        if (buffer[0] == '\0') return true;
        return false;
    }

    return strncmp(buffer, str, buffer_len) == 0;
}



//some buffer types.
typedef CBuffer<16> buffer16;
typedef CBuffer<32> buffer32;
typedef CBuffer<64> buffer64;
typedef CBuffer<128> buffer128;
typedef CBuffer<256> buffer256;


#endif
