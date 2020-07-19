
/*
* 2020 Jon Robinson 
* MIT License
* Arduino sketch which using envelope/peak detector circuit output signal to detect if music playing.
* Opens/ closes a relay accordingly - relay intended to switch amp on and off.
*/

// Two analog pins connected to envelope detector circuits
// one for left channel, one for right channel
int _aSoundPin = A5;

int val = 0;  // variable to store the signal read from pins 

const int _onoffPin = 7; // manual switch to turn function on and off

/*
* LED show status:
* - green only: music playing (relay closed)
* - red only: no music (relay open)
* - green and red: no musuc playing, will open relay in X seconds unless music detected again
*/
const int _redPin= 4; 
const int _greenPin = 5;

const int _relayControlPin = 2;
const int SILENCE_THRESHOLD = 5;
const long SILENCE_TIME_TO_TRIGGER_POWEROFF = 1000*20; // 20 secs of silence 
const long TIME_TO_TRIGGER_SILENCE = 1000;
const long AFTER_RELAY_CHANGE_DELAY = 300; //ms. To address noisy ground relay seems to cause.

const int STATUS_MUSIC = 0;
const int STATUS_SILENCE = 1;
const int STATUS_POWEROFF = 2;
const int STATUS_UNDEFINED = 100;
int _status = STATUS_SILENCE;
int _oldStatus = STATUS_UNDEFINED;


int _onoff;

unsigned long _lastSilentStartTime = 0;
unsigned long _lastNotSilentTime = 0;
unsigned long _lastRelayChangeTime = 0;


void setup()
{
 // Serial.begin(9600);
    pinMode(_onoffPin,INPUT_PULLUP);

    // rgb led
    pinMode(_redPin, OUTPUT);
    pinMode(_greenPin, OUTPUT);


    // relay
    pinMode(_relayControlPin, OUTPUT);
}
void loop()
{
    _onoff = digitalRead(_onoffPin);

    if (_onoff == LOW){
      // Turn off/by-pass control function
      digitalWrite(_relayControlPin, HIGH); //open relay
      _lastRelayChangeTime = millis();
      _oldStatus = STATUS_UNDEFINED;
      digitalWrite(_greenPin, LOW);
      digitalWrite(_redPin,LOW);
    } 

    //read enveloped sound value on soundPin
  if (millis() - _lastRelayChangeTime >=  AFTER_RELAY_CHANGE_DELAY && _onoff == HIGH){
    val = analogRead(_aSoundPin);

    if (val>SILENCE_THRESHOLD){
        _lastNotSilentTime = millis();
        _status = STATUS_MUSIC;         // debug value
    } else {
        if (_status==STATUS_MUSIC){
          unsigned long dt = millis() - _lastNotSilentTime;
          if (dt >= TIME_TO_TRIGGER_SILENCE) {
            _status = STATUS_SILENCE;  
            _lastSilentStartTime = millis();
          }     
        } else if (_status == STATUS_SILENCE) {
          unsigned long dt = millis() - _lastSilentStartTime;
          if (dt >= SILENCE_TIME_TO_TRIGGER_POWEROFF){
              _status = STATUS_POWEROFF;
          }
        }      
    }

    if (_oldStatus!=_status){
        switch(_status){
          case (STATUS_MUSIC):
              digitalWrite(_relayControlPin, HIGH); //open relay
              _lastRelayChangeTime = millis();
              digitalWrite(_greenPin, HIGH);
              digitalWrite(_redPin,LOW);
              break;
          case (STATUS_SILENCE):
              digitalWrite(_greenPin, HIGH);
              digitalWrite(_redPin,HIGH);
              break;
          case (STATUS_POWEROFF):
              digitalWrite(_greenPin, LOW);
              digitalWrite(_redPin,HIGH);
              digitalWrite(_relayControlPin, LOW); //close relay
              _lastRelayChangeTime = millis();
              break;
        } 
        _oldStatus = _status;
    }
  }
}
