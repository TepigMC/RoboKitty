#include "ABCNoteParser.h"

// ABC Notation
char inputChar;
boolean inBrackets;
float meterValue;
float defaultNoteLength;
long beatsPerMinute;
long defaultNoteDuration;

// Frequencies from Wikipedia:
// https://en.wikipedia.org/wiki/Scientific_pitch_notation
// https://en.wikipedia.org/wiki/Piano_key_frequencies
// Middle C is represented as C
// C' is equivalent to c
int frequencies[] = {
  // C     C#/Db    D        D#/Eb    E        F
  // F#/Gb G        G#/Ab    A        A#/Bb    B
  16.352,  17.324,  18.354,  19.445,  20.602,  21.827,  // Octave 0 (0-11) C,,,,
  23.125,  24.5,    25.957,  27.5,    29.1352, 30.8677,
  32.7032, 34.6478, 36.7081, 38.8909, 41.2034, 43.6535, // Octave 1 (12-23) C,,,
  46.2493, 48.9994, 51.9131, 55,      58.2705, 61.7354,
  65.4064, 69.2957, 73.4162, 77.7817, 82.4069, 87.3071, // Octave 2 (24-35) C,,
  92.4986, 97.9989, 103.826, 110,     116.541, 123.471,
  130.813, 138.591, 146.832, 155.563, 164.814, 174.614, // Octave 3 (36-47) C,
  184.997, 195.998, 207.652, 220,     233.082, 246.942,
  261.626, 277.183, 293.665, 311.127, 329.628, 349.228, // Octave 4 (48-59) C
  369.994, 391.995, 415.305, 440,     466.164, 493.883,
  523.251, 554.365, 587.33,  622.254, 659.255, 698.456, // Octave 5 (60-71) c
  739.989, 783.991, 830.609, 880,     932.328, 987.767,
  1046.5,  1108.73, 1174.66, 1244.51, 1318.51, 1396.91, // Octave 6 (72-83) c'
  1479.98, 1567.98, 1661.22, 1760,    1864.66, 1975.53,
  2093,    2217.46, 2349.32, 2489.02, 2637.02, 2793.83, // Octave 7 (84-95) c''
  2959.96, 3135.96, 3322.44, 3520,    3729.31, 3951.07,
  4186.01, 4434.9,  4698.6,  4978,    5274,    5587.7,  // Octave 8 (96-107) c'''
  5919.9,  6271.9,  6644.9,  7040,    7458.6,  7902.1,
  8372,    8869.8,  9397.3,  9956.1,  10548.1, 11175.3, // Octave 9 (108-119) c''''
  11839.8, 12543.9, 13289.8, 14080,   14917.2, 15804.3,
  16744,   17739.7, 18794.5, 19912.1, 21096.2, 22350.6, // Octave 10 (120-131) c'''''
  23679.6, 25087.7, 26579.5, 28160,   29834.5, 31608.5
};

// Accidentals for each standard key signatures
/*int keySignatures[][7] = {
// C  D  E  F  G  A  B
  {1, 1, 1, 1, 1, 1, 1}, // 7 sharps - C#
  {1, 1, 1, 1, 1, 1, 0}, // 6 sharps - F#
  {1, 1, 0, 1, 1, 1, 0}, // 5 sharps - B
  {1, 1, 0, 1, 1, 0, 0}, // 4 sharps - E
  {1, 0, 0, 1, 1, 0, 0}, // 3 sharps - A
  {1, 0, 0, 1, 0, 0, 0}, // 2 sharps - D
  {0, 0, 0, 1, 0, 0, 0}, // 1 sharp - G
  {0, 0, 0, 0, 0, 0, 0}, // 0 sharps/flats - C
  {0, 0, 0, 0, 0, 0, -1}, // 1 flat - F
  {0, 0, 0, -1, 0, 0, -1}, // 2 flats - Bb
  {0, 0, 0, -1, 0, -1, -1}, // 3 flats - Eb
  {0, -1, 0, -1, 0, -1, -1}, // 4 flats - Ab
  {0, -1, 0, -1, -1, -1, -1}, // 5 flats - Db
  {-1, -1, 0, -1, -1, -1, -1}, // 6 flats - Gb
  {-1, -1, -1, -1, -1, -1, -1} // 7 flats - Cb
};*/

ABCNoteParser::ABCNoteParser() {
  reset();
}

void ABCNoteParser::reset() {
  inputChar = ' ';
  inBrackets = false;
  meterValue = 1.0f;
  defaultNoteLength = 0.125f;
  beatsPerMinute = 120;
  defaultNoteDuration = 0;
}

void ABCNoteParser::getNextNote(Stream* stream, int* freq, int* dur) {
  while (true) {
    // Get the next char for next iteration if we haven't already got it
    // (sometimes we have already 'previewed' a char to check if the note is over or not)
    skipCharacters(stream, &inputChar, " ");
    // If we reached the end of the stream, abort finding the next note
    if (inputChar == EOF) return;
    // If we find double quotes or +, "escape" what is inside them
    if (inputChar == '"') {
      inputChar = stream->read();
      skipCharactersUntil(stream, &inputChar, "\"");
    }
    if (inputChar == '+') {
      inputChar = stream->read();
      skipCharactersUntil(stream, &inputChar, "+");
    }

    // 3 - Information fields
    // Any line beginning with a letter in the range A-Z or a-z and
    // immediately followed by a colon (:) is an information field.
    switch (inputChar) {
      // Ignore all unsupported header lines
      //case 'A': // 3.1.5 - area
      //case 'B': // 3.1.16 - book
      //case 'C': // 3.3.3 - composer
      //case 'D': // 3.1.16 - discography
      //case 'E': // unused
      //case 'F': // 3.1.16 - file url
      //case 'G': // 3.1.12 - group
      case 'H': // 3.1.13 - history
      case 'I': // 3.1.17 - instruction
      case 'J': // unused
      //case 'K': // 3.1.14 - key
      //case 'L': // 3.1.7 - unit note length
      //case 'M': // 3.1.6 - meter
      //case 'm': // 3.1.18 - macro
      case 'N': // 3.1.11 - notes
      case 'O': // 3.1.4 - origin
      case 'P': // 3.1.9 - parts
      //case 'Q': // 3.1.8 - tempo
      case 'R': // 3.1.15 - rhythm
      //case 'r': // 3.1.18 - remark
      case 'S': // 3.1.16 - source
      //case 's': // 3.1.18 - symbol line
      case 'T': // 3.2.2 - tune title
      case 'U': // 3.1.18 - user defined
      case 'V': // 3.1.18 - voice
      case 'W': // 3.1.18 - words
      //case 'w': // 3.1.18 - words
      case 'X': // 3.1.1 - reference number
      case 'Y': // unused
      case 'Z': // 3.1.10 - transcription
      case '%':
        skipCharactersUntil(stream, &inputChar, "\n");
        break;

      // Key
      // also marks the end of the header
      case 'K':
        Serial.println(F("Handling header: K - Key"));
        // Currently only supports default Key C major
        // Marks the end of the official header, so scroll forward til endline
        skipCharactersUntil(stream, &inputChar, "\n]");

        Serial.println(F("Final Settings: "));
        Serial.print(F("Meter: "));
        Serial.println(meterValue);
        Serial.print(F("Note Length: "));
        Serial.println(defaultNoteLength, 4);
        Serial.print(F("BPM: "));
        Serial.println(beatsPerMinute);
        Serial.print(F("Note Duration: "));
        Serial.println(defaultNoteDuration);
        Serial.println(F("Finished handling header: K - Key"));

        break;

      case 'M': // Meter
        Serial.println(F("Handling header: M - Meter"));
        // Remove the expected ':' character
        inputChar = stream->read();
        skipCharacters(stream, &inputChar, ": ");
        // Get the value for the meter from the fractional form (ex: 4/4 or 6/8)
        meterValue = (float) getIntegerFromStream(stream, &inputChar);
        if (inputChar == '/') {
          inputChar = stream->read(); // Move past the '/'
          meterValue /= (float) getIntegerFromStream(stream, &inputChar);
        }
        // Calculate the default note length based on the Meter
        defaultNoteLength = meterValue < 0.75f ? 0.0625f : 0.125f;
        // Calculate the default note duration based off of our (potentially new) note length
        defaultNoteDuration = delayTimeInMilliseconds(defaultNoteLength, beatsPerMinute);
        break;

      case 'Q': // Tempo
        Serial.println(F("Handling header: Q - Tempo"));
        // Remove the expected ':' character
        inputChar = stream->read();
        skipCharacters(stream, &inputChar, ": ");
        // Get the integer value for the tempo
        defaultNoteDuration = getIntegerFromStream(stream, &inputChar);
        // If a '/' is present, it means that they have specified
        // which note to assign the tempo to, so we must make calculations
        // based on this assumption
        if (inputChar == '/') {
          inputChar = stream->read();
          double tempoNoteLength = (double) defaultNoteDuration / (double) getIntegerFromStream(stream, &inputChar);
          // Bypass the filler characters
          while (inputChar != '=') inputChar = stream->read();
          inputChar = stream->read();
          while (inputChar == ' ') inputChar = stream->read();
          // Ihe next int from the stream will be our bpm for the given note length
          beatsPerMinute = getIntegerFromStream(stream, &inputChar);
          // Calculate the default note duration based off of our (potentially new) bpm
          defaultNoteDuration = delayTimeInMilliseconds(defaultNoteLength, beatsPerMinute);
        } else {
          // If '/' was not present, then they just gave us our tempo
          beatsPerMinute = defaultNoteDuration;
          // Calculate the default note duration based off of our bpm
          defaultNoteDuration = delayTimeInMilliseconds(defaultNoteLength, beatsPerMinute);
        }
        Serial.println(F("Finished handling header: Q - Tempo"));
        break;

      case 'L': // Default Note Length
        Serial.println(F("Handling header: L - Note Length"));
        // Remove the expected ':' character
        inputChar = stream->read();
        skipCharacters(stream, &inputChar, ": ");
        // Get the value for the length from the fractional form (ex: 4/4)
        defaultNoteLength = (float) getIntegerFromStream(stream, &inputChar);
        if (inputChar == '/') {
          inputChar = stream->read();
          defaultNoteLength /= (float) getIntegerFromStream(stream, &inputChar);
        }
        // Calculate the default note duration based off of our (potentially new) note length
        defaultNoteDuration = delayTimeInMilliseconds(defaultNoteLength, beatsPerMinute);
        break;

      case '[':
        inBrackets = true;
        inputChar = stream->read();
        break;

      case '|':
        // Some combinations of brackets [] actually have aesthetic meanings in ABC Notation
        // Once such combo is [| which we don't want to be seen as chord brackets []
        inBrackets = false;
        inputChar = stream->read();
        break;

      case ']':
        inBrackets = false;
        inputChar = stream->read();
        break;

      default:
        // If not a header, treat it as a note
        *freq = getFrequency(stream, &inputChar);
        // If we didn't successfully get a note, 
        // reset our char and exit this loop's iteration
        if (*freq == -1) {
          inputChar = ' ';
          continue;
        }
        // Otherwise continue to get the duration next
        *dur = getDuration(stream, &inputChar);

        // If we are in a set of brackets, that means the music wants to
        // play multiple notes at once, which the Arduino piezo does not support
        // So we must escape all remaining notes until the end bracket
        if (inBrackets) skipCharactersUntil(stream, &inputChar, "]");

        // Make sure to return once we have our next valid note
        return;
    }
  }
}

int ABCNoteParser::getIntegerFromStream(Stream* stream, char* previewedChar) {
  int num = 0;
  while ('0' <= *previewedChar && *previewedChar <= '9') {
    num = (num*10) + atoi(previewedChar);
    *previewedChar = stream->read();
  }
  return num;
}

double ABCNoteParser::delayTimeInMilliseconds(double noteLength, float bpm) {
  return 240000 * noteLength / bpm;
}

int ABCNoteParser::getDuration(Stream* stream, char* input) {
  // Start off with the default duration
  int duration = defaultNoteDuration;

  // First see if we can find a multiplier (no preceding chars, just digits)
  int modifier = getIntegerFromStream(stream, input);

  if (modifier > 0) {
    // If there was a digit, then it means we multiply our current duration by that number
    duration *= modifier;
  }

  while (*input == '/') {
    // If a / is found, it means we divide our duration by the next number
    // If no number is provided, the default is /2
    modifier = getIntegerFromStream(stream, input);
    if (modifier > 0) duration /= modifier;
    else duration /= 2;
    *input = stream->read();
  }

  while (*input == '>') {
    // A > (aka a hornpipe) represents a 'dotted' note
    // (therefore adds half its current duration to itself)
    duration += duration/2;
    *input = stream->read();
  }

  return duration;
}

int ABCNoteParser::getFrequency(Stream* stream, char* input) {
  // Setup some note-specific defaults first
  boolean hasAccidentals = false;
  int accidentals = 0;
  int octave = 4;
  int noteStep = -1;

  // Get the accidental modifier(s)
  while (strchr("^=_", *input)) {
    hasAccidentals = true;
    if (*input == '^') accidentals++;
    else if (*input == '_') accidentals--;
    *input = stream->read();
  }

  // If z (rest), return frequency of 0
  if (*input == 'z') return 0;

  // TODO
  if (hasAccidentals) {
    // remove accidentals from key signature
  }

  // If lowercase, set up an octave
  char noteName = toupper(*input); // NOTE: not sure if this is the correct pointer use
  if (noteName != *input) octave++;

  // Get the half steps relative to C
  noteStep = getNaturalSteps(noteName);

  // If, for whatever reason, we still don't have a note, exit with a failure
  if (noteStep == -1) return -1;

  *input = stream->read();

  // Get the octave modifier (optional)
  if (*input == '\'') {
    // Apostrophe ' puts the note up an octave
    octave++;
    *input = stream->read();
  }    
  if (*input == ',') {
    // Comma , puts the note down an octave
    octave--;
    *input = stream->read();
  }

  // Take our note frequency info and get the actual frequency
  return frequencies[12 * octave + noteStep + accidentals];
}

int ABCNoteParser::getNaturalSteps(char note) {
  switch (note) {
    case 'C': return 0;
    case 'D': return 2;
    case 'E': return 4;
    case 'F': return 5;
    case 'G': return 7;
    case 'A': return 9;
    case 'B': return 11;
  }
  return -1;
}

void ABCNoteParser::skipCharacters(Stream* stream, char* input, char* skipChars) {
  while (strchr(skipChars, *input)) {
    *input = stream->read();
  }
}

void ABCNoteParser::skipCharactersUntil(Stream* stream, char* input, char* skipUntil) {
  while (!strchr(skipUntil, *input)) {
    *input = stream->read();
  }
}
