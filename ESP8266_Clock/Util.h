#include <vector>
#include <string>

std::vector<std::string> Split(const std::string& strInput, char separator)
{
  Serial.printf("Splitting IP...\n");
  std::vector<std::string> vParts;
  size_t pos = 0;
  size_t prev_pos = 0;
  while(pos != std::string::npos)
  {
    prev_pos = pos;
    pos = strInput.find(separator, prev_pos + 1);
    size_t len = pos != std::string::npos ? pos - (prev_pos ? prev_pos + 1 : 0) : pos;
    Serial.printf(strInput.substr(prev_pos ? prev_pos + 1 : 0, len).c_str());
    Serial.printf("\n");
    vParts.push_back(strInput.substr(prev_pos ? prev_pos + 1 : 0, len));
  }
  return vParts;
}
