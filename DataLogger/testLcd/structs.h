#define CONFIG_VERSION "l.0"
#define CONFIG_START 32

struct DATA {
  int IntTemp;
  int ExtTemp;
  int Humidity;
  int Presure;
  int Volt1;
  int Volt2; 
};

// valve struct
//  0 - enabled [0|1]
//  1 - day [1-7]
//  2 - hour
//  3 - muinutes 
//  4 - length 
//  5 - ?
//  6 - ?
// 
struct SETTINGS{
  char Relay1 [7];
  char Relay2 [7];
  char Relay3 [7];
  char Relay4 [7];
  char version_of_program[4];
};

struct RELAY_SETTINGS{
  char Enabled;
  char Day;
  char Hour;
  char Minutes;
  char Length;
  char Reserved1;
  char Reserved2;
};
