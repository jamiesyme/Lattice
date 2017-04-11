#include "json.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


void initJsonParser(JsonParser* parser, char* str, int strLen)
{
  parser->str = str;
  parser->strLen = strLen;
  parser->offset = 0;
}

void parseJson(JsonParser* parser, JsonData* data)
{
  // Reset the json data
  data->error.type = JET_NONE;
  data->error.offset = 0;
  data->type = 0;
  data->start = 0;
  data->end = 0;

  // Find the next json data
  while (parser->offset < parser->strLen) {

    // Check for array start
    if (parser->str[parser->offset] == '[') {
      data->type = JT_ARRAY_START;
      data->start = parser->offset;
      data->end = parser->offset++;
      return;
    }

    // Check for array end
    if (parser->str[parser->offset] == ']') {
      data->type = JT_ARRAY_END;
      data->start = parser->offset;
      data->end = parser->offset++;
      return;
    }

    // Check for object start
    if (parser->str[parser->offset] == '{') {
      data->type = JT_OBJECT_START;
      data->start = parser->offset;
      data->end = parser->offset++;
      return;
    }

    // Check for object end
    if (parser->str[parser->offset] == '}') {
      data->type = JT_OBJECT_END;
      data->start = parser->offset;
      data->end = parser->offset++;
      return;
    }

    // Check for true boolean
    if (parser->offset + 4 <= parser->strLen &&
        strncmp(parser->str + parser->offset, "true", 4) == 0) {
      data->type = JT_BOOLEAN;
      data->start = parser->offset;
      data->end = parser->offset + 3;
      parser->offset += 4;
      return;
    }

    // Check for false boolean
    if (parser->offset + 5 <= parser->strLen &&
        strncmp(parser->str + parser->offset, "false", 5) == 0) {
      data->type = JT_BOOLEAN;
      data->start = parser->offset;
      data->end = parser->offset + 4;
      parser->offset += 5;
      return;
    }

    // Check for null
    if (parser->offset + 4 <= parser->strLen &&
        strncmp(parser->str + parser->offset, "null", 4) == 0) {
      data->type = JT_NULL;
      data->start = parser->offset;
      data->end = parser->offset + 3;
      parser->offset += 4;
      return;
    }

    // Check for number
    if (parser->str[parser->offset] == '-' ||
        isdigit(parser->str[parser->offset])) {
      data->type = JT_NUMBER;
      data->start = parser->offset;

      if (parser->str[parser->offset] == '-') {
        parser->offset++;
      }

      while (isdigit(parser->str[parser->offset])) {
        parser->offset++;
      }

      if (parser->str[parser->offset] == '.') {
        parser->offset++;
      }

      while (isdigit(parser->str[parser->offset])) {
        parser->offset++;
      }

      if (parser->str[parser->offset] == 'e' ||
          parser->str[parser->offset] == 'E') {
        parser->offset++;

        if (parser->str[parser->offset] == '+' ||
            parser->str[parser->offset] == '-') {
          parser->offset++;
        }

        while (isdigit(parser->str[parser->offset])) {
          parser->offset++;
        }
      }

      data->end = parser->offset - 1;
      return;
    }

    // Check for string
    if (parser->str[parser->offset] == '"') {
      data->type = JT_STRING;
      data->start = parser->offset + 1;

      parser->offset++;
      while (parser->offset < parser->strLen &&
             parser->str[parser->offset] != '"') {

        // Backslashes have special rules
        if (parser->str[parser->offset] == '\\') {

          // Check for errors
          if (parser->offset + 1 >= parser->strLen) {
            data->error.type = JET_INVALID_STRING;
            data->error.offset = parser->offset;
            return;
          }

          // Check the control character
          parser->offset++;
          switch (parser->str[parser->offset]) {
          case '"':
          case '\\':
          case '/':
          case 'b':
          case 'f':
          case 'n':
          case 'r':
          case 't':
            parser->offset++;
            break;
          case 'u':
            parser->offset++;
            if (parser->offset + 4 > parser->strLen ||
                isxdigit(parser->str[parser->offset + 0]) == 0 ||
                isxdigit(parser->str[parser->offset + 1]) == 0 ||
                isxdigit(parser->str[parser->offset + 2]) == 0 ||
                isxdigit(parser->str[parser->offset + 3]) == 0) {
              data->error.type = JET_INVALID_STRING;
              data->error.offset = parser->offset;
              return;
            }
            parser->offset += 4;
            break;
          default:
            data->error.type = JET_INVALID_STRING;
            data->error.offset = parser->offset;
            return;
          }
          continue;
        }

        // Control characters will be ignored
        // TODO: Should these throw an error?
        if (iscntrl(parser->str[parser->offset])) {
          parser->offset++;
        }

        // Anything else is fair game
        parser->offset++;
      }

      // Make sure we found our closing quote
      if (parser->str[parser->offset] != '"') {
        data->error.type = JET_INVALID_STRING;
        data->error.offset = data->start - 1;
        return;
      }
      data->end = parser->offset - 1;
      parser->offset++;
      return;
    }

    // Skip whitespace
    if (isspace(parser->str[parser->offset])) {
      parser->offset++;
      continue;
    }

    // Skip commas
    // TODO: Make sure commas only come after key pairs and elements
    if (parser->str[parser->offset] == ',') {
      parser->offset++;
      continue;
    }

    // Skip colons
    // TODO: Make sure colons only come after key names
    if (parser->str[parser->offset] == ':') {
      parser->offset++;
      continue;
    }

    // What is this?!
    data->error.type = JET_UNEXPECTED_CHARACTER;
    data->error.offset = parser->offset;
    return;
  }

  // We found nothing
  data->error.type = JET_NO_MORE;
}

char* jsonDataToString(JsonParser* parser, JsonData* data)
{
  int fullOffset, strLen, strOffset, i;
  char* str;

  // Count the characters we need to allocate
  fullOffset = data->start;
  strLen = 0;
  while (fullOffset <= data->end) {
    if (parser->str[fullOffset] == '\\') {
      fullOffset++;
      switch (parser->str[fullOffset]) {
      case '"':
      case '\\':
      case '/':
      case 'b':
      case 'f':
      case 'n':
      case 'r':
      case 't':
        fullOffset++;
        strLen++;
        break;
      case 'u':
        fullOffset += 5;
        strLen += 5;
        break;
      }
      continue;
    }
    fullOffset++;
    strLen++;
  }

  // Create the null-terminated string
  str = malloc(sizeof(char) * (strLen + 1));
  str[strLen] = '\0';

  // Copy and interpret the string
  strOffset = 0;
  fullOffset = data->start;
  while (fullOffset <= data->end) {
    if (parser->str[fullOffset] == '\\') {
      fullOffset++;
      switch (parser->str[fullOffset]) {
      case '"':
      case '\\':
      case '/':
        str[strOffset++] = parser->str[fullOffset++];
        break;
      case 'b':
        str[strOffset++] = '\b';
        fullOffset++;
        break;
      case 'f':
        str[strOffset++] = '\f';
        fullOffset++;
        break;
      case 'n':
        str[strOffset++] = '\n';
        fullOffset++;
        break;
      case 'r':
        str[strOffset++] = '\r';
        fullOffset++;
        break;
      case 't':
        str[strOffset++] = '\t';
        fullOffset++;
        break;
      case 'u':
        for (i = 0; i < 5; ++i) {
          str[strOffset + i] = parser->str[fullOffset + i];
        }
        strOffset += 5;
        fullOffset += 5;
        break;
      }
      continue;
    }
    str[strOffset++] = parser->str[fullOffset++];
  }

  return str;
}

double jsonDataToNumber(JsonParser* parser, JsonData* data)
{
  if (data->type == JT_NUMBER) {
    return atof(parser->str + data->start);
  }
  return 0.0;
}
