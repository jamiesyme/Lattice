#pragma once

typedef enum {
  JET_NONE,
  JET_NO_MORE,
  JET_INVALID_STRING,
  JET_UNEXPECTED_CHARACTER
} JsonErrorType;

typedef struct {
  JsonErrorType type;
  int offset;
} JsonError;

typedef enum {
  JT_ARRAY_START,
  JT_ARRAY_END,
  JT_OBJECT_START,
  JT_OBJECT_END,
  JT_BOOLEAN,
  JT_NULL,
  JT_NUMBER,
  JT_STRING
} JsonType;

typedef struct {
  JsonError error;
  JsonType type;
  int start;
  int end;
} JsonData;

typedef struct {
  char* str;
  int strLen;
  int offset;
} JsonParser;


// Use this to initialize the parser before use.
// This can also be used to reset the parser.
void initJsonParser(JsonParser* parser, char* str, int strLen);


// Parse the next piece of data from the json string.
// When there is no more data left, `data->error.type` will be set to
// `JET_NO_MORE`.
void parseJson(JsonParser* parser, JsonData* data);


// Converts the json data struct to a c-string.
// For every type besides string, their source string representation is used.
// For example, a true boolean json data would be returned as "true".
// For a string type, successive characters, like "\n", will be converted to
// their single-character respresentations, like '\n'. All control characters
// documented in the JSON spec at http://www.json.org are supported, EXCLUDING
// unicode digits: these will be rendered without the backslash.
// NOTE: It is up to the caller to free the returned string using free().
char* jsonDataToString(JsonParser* parser, JsonData* data);


// Converts a json data struct of type JT_NUMBER to a double.
// Other json data struct types will return 0.
double jsonDataToNumber(JsonParser* parser, JsonData* data);
