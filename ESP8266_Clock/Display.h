
#define BUFFER_DATA     0
#define BUFFER_CLK      5
#define BUFFER_LATCH    2

const char positionMap[] = { 4, 3, 2, 5, 1, 6, 0, 7, 8 };

// Char map
const char mapChars[] =
{

//             0
//           1   2
//             3
//           4   5
//             6
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
    0b01110111, // 'a'
    0b01111100, // 'b'
    0b01011000, // 'c'
    0b01011110, // 'd'
    0b01111011, // 'e'
    0b01110001, // 'f'
    0b01101111, // 'g'
    0b01110100, // 'h'
    0b00000100, // 'i'
    0b00011110, // 'j'
    0b01110000, // 'k'
    0b00111000, // 'l'
    0b01010101, // 'm' (n with line above)
    0b01010100, // 'n'
    0b01011100, // 'o'
    0b01110011, // 'p'
    0b11100111, // 'q'
    0b01010000, // 'r'
    0b01101101, // 's' (same as 5)
    0b01111000, // 't'
    0b00111110, // 'u'
    0b00011100, // 'v'
    0b00011101, // 'w' (v with line above)
    0b01110110, // 'x'
    0b01101110, // 'y'
    0b01011011, // 'z' (same as 2)

    0b00000000, // ' '

    0b01000000, // '-' (same as :)
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

  delayMicroseconds(2);
  digitalWrite(BUFFER_CLK, HIGH);
  delayMicroseconds(2);
  digitalWrite(BUFFER_CLK, LOW);

  delayMicroseconds(2);
  digitalWrite(BUFFER_DATA, LOW);
}

void Latch()
{
  digitalWrite(BUFFER_LATCH, HIGH);
  delayMicroseconds(2);
  digitalWrite(BUFFER_LATCH, LOW);
}

void PrintChar(unsigned char ucChar, unsigned char ucPosition, unsigned char ucDot)
{
    unsigned char i = 0;

  if (ucChar >= '0' && ucChar <= '9')
  {
    ucChar -= '0';
  }
  else if(ucChar >= 'a' && ucChar <= 'z')
  {
    ucChar -= 'a' + 10;
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

    for(i = 0; i < 7; i++)
    {
        SendBit(mapChars[ucChar] & 1 << i);
    }
    SendBit(ucDot);

  Latch();
}

void PrintString(char* szText)
{
  unsigned char i = 0;
  while(i < 8)
  {
    PrintChar(szText[i], i, 0);
    i++;
  }
}
