#ifndef _STRINGTOKENIZER_H_
#define _STRINGTOKENIZER_H_

#include <stdio.h>
#include <string.h>

class StringTokenizer
{

public:
  StringTokenizer(char *pData, char *pDelimiter)
  {
    data = pData;
    dataLen = (int)strlen(data);
    delimiter = pDelimiter;
    delimiterLen = (int)strlen(pDelimiter);
    offset = 0;
    done = false;
  }

  virtual ~StringTokenizer(){}    

char* nextToken()
  {
    tokenSize = 0;
    token[0] = '\0';
    for(int i = 0; offset < dataLen; i++) 
    {
      token[i] = data[offset];
      token[i+1] = '\0'; //Forces string end. Keeps non-ending strings from ever happening.

      for(int d = 0; d < delimiterLen; d++)
      {
        if(token[i] == delimiter[d])
        {
          token[i] = '\0';
          if(offset == dataLen)
            done = true;
          offset++;
          tokenSize = i;
          return token;
        }
      }

      offset++;
    }
    done = true;
    return token;
  }

bool hasMoreTokens() const
  {
    return !done;
  }

int getDataLength() const
  {
    return dataLen;
  }

int getTokenSize() const
  {
    return tokenSize;
  }

int getNumTokens()
  {
    int temp = offset;
    int num = 0;
		bool doneswap = done;	

    while(hasMoreTokens())
    {
      if (nextToken() != NULL)
      {
        ++num;
      }
    }
		
		done = doneswap; //reset this to it's previous state
    
    offset = temp;
    return num;
  }

void reset()
{
  offset = 0;
}


private:
  char *data;
  char *delimiter;
  int offset;
  bool done;
  int dataLen;
  int delimiterLen;
  int tokenSize;
  char token[512];

};

#endif
