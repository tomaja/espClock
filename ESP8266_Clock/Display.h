#include <string>
#include <algorithm>

#define BUFFER_DATA     0 // D3
#define BUFFER_CLK      5 // D1
#define BUFFER_LATCH    2 // D4

const char positionMap[] = { 4, 3, 2, 5, 1, 6, 0, 7, 8 };

// Char map
const char mapChars[] =
{

//             0
//           1   2
//             3
//           4   5
//             6    7
    // Digits
    0b01110111, // '0'
    0b00100100, // '1'
    0b01011101, // '2'
    0b01101101, // '3'
    0b00101110, // '4'
    0b01101011, // '5'
    0b01111011, // '6'
    0b00100101, // '7'
    0b01111111, // '8'
    0b01101111, // '9'

    // Letters
    0b00111111, // 'A'
    0b01111111, // 'B'
    0b01010011, // 'C'
    0b01111100, // 'D'
    0b01011011, // 'E'
    0b00011011, // 'F'
    0b01110011, // 'G'
    0b00111110, // 'H'
    0b00100100, // 'I'
    0b01110100, // 'J'
    0b01110000, // 'K'
    0b00111000, // 'L'
    0b01010101, // 'M' (n with line above)
    0b00111000, // 'N'
    0b01011100, // 'O'
    0b01110011, // 'P'
    0b11100111, // 'Q'
    0b01010000, // 'R'
    0b01101101, // 'S' (same as 5)
    0b01011010, // 'T'
    0b00111110, // 'U'
    0b00011100, // 'V'
    0b00011101, // 'W' (v with line above)
    0b01110110, // 'X'
    0b01101110, // 'Y'
    0b01011011, // 'Z' (same as 2)

    0b00000000, // ' '
    0b00001000, // '-' (same as :)
};

const unsigned char DISPLAY_CHARS = 9;
char aDisplayData[DISPLAY_CHARS] = { ' ' };

// Send single bit to MAX6921
void SendBit(const unsigned char cOn)
{
  if(cOn != 0)
  {
    digitalWrite(BUFFER_DATA, HIGH);
  }
  else
  {
    digitalWrite(BUFFER_DATA, LOW);
  }

  //delayMicroseconds(2);
  digitalWrite(BUFFER_CLK, HIGH);
  //delayMicroseconds(2);
  digitalWrite(BUFFER_CLK, LOW);

  //delayMicroseconds(2);
  digitalWrite(BUFFER_DATA, LOW);
}

void Latch()
{
  digitalWrite(BUFFER_LATCH, HIGH);
  //delayMicroseconds(2);
  digitalWrite(BUFFER_LATCH, LOW);
}

void PrintChar(unsigned char ucChar, unsigned char ucPosition, unsigned char ucDot)
{
  ESP.wdtFeed();
  unsigned char i = 0;

  if (ucChar >= '0' && ucChar <= '9')
  {
    ucChar -= '0';
  }
  else if(ucChar >= 'A' && ucChar <= 'Z')
  {
    ucChar -= 'A' + 10;
  }
  else if(ucChar == ' ')
  {
      ucChar = 36;
  }
  else if(ucChar == '-')
  {
      ucChar = 37;
  }
  else if(ucChar == ':')
  {
      ucChar = 37;
  }

  for(i = 0; i < 3; i++) // 3 padding bits
  {
      SendBit(0);
  }

  for(i = 0; i < 9; i++)
  {
    if(i != positionMap[ucPosition])
    {
      SendBit(0);
    }
    else
    {
      SendBit(1);
    }
  }

  if(ucChar == '.')
  {
    ucChar = 36;
    ucDot = 1;
  }
  
  for(i = 0; i < 7; i++)
  {
      SendBit(mapChars[ucChar] & 1 << i);
  }
  SendBit(ucDot);

  Latch();
}

void PrintString(const char* szText)
{
  unsigned char i = 0;
  while(i < 8 && i < strlen(szText))
  {
    PrintChar(szText[i], i, 0);
    i++;
  }
  while(i < 8)
  {
    PrintChar(' ', i, 0);
    i++;
  }
}

void PrintString(String strText)
{
  unsigned char i = 0;
  while(i < 8 && i < strText.length())
  {
    PrintChar(strText[i], i, 0);
    i++;
  }
}

void PrintString(const std::string& strText)
{
  unsigned char i = 0;
  std::string strSafeText = strText.substr(0, std::min(size_t(8), strText.length()));
  int offset = 4 - strSafeText.length() / 2;
  while(i < offset)
  {
    PrintChar(' ', i, 0);
    i++;
  }
  i == offset;
  while(i < 8 && i < offset + strSafeText.length())
  {
    PrintChar(strText[i], i, 0);
    i++;
  }
  while(i < 8)
  {
    PrintChar(' ', i, 0);
    i++;
  }
}

void PrintForMS(const char* szText, long long ms)
{
  int loops = ms * 2;
  for(int i = 0; i < loops; i++)
  {
    PrintString(szText);
    delayMicroseconds(5);
  }
}

void PrintForMS(String strText, long long ms)
{
  int loops = ms * 2;
  for(int i = 0; i < loops; i++)
  {
    PrintString(strText);
    delayMicroseconds(5);
  }
}

void PrintForMS(const std::string& strText, long long ms)
{
  int loops = ms * 2;
  for(int i = 0; i < loops; i++)
  {
    PrintString(strText);
    delayMicroseconds(5);
  }
}
