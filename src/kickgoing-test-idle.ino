/*
 * Project kickgoing-test-idle
 * Description:
 * Author:
 * Date:
 */

SYSTEM_MODE(MANUAL);
SerialLogHandler logHandler(LOG_LEVEL_TRACE);

#define uart_SC Serial1
#define TIMEOUT_RECV_SC 12
#define mode_SC D0

bool flag_transmit;

void set_flag_transmit(){
  flag_transmit = true;
}

Timer timer_transmit(20, set_flag_transmit);

uint8_t data_idle[20] = {0x55, 0xaa, 0x07, 0x20, 0x65, 0x00, 0x04, 0x27, 0x27, 0x00, 0x00, 0x21, 0xff};
uint8_t data_request[20] = {0x55, 0xaa, 0x09, 0x20, 0x64, 0x00, 0x06, 0x27, 0x27, 0x00, 0x00, 0x68, 0x00, 0xb6, 0xfe};

uint8_t mode;
uint32_t iiindex;
void receive(uint8_t bytes_to_read);

// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.
  uart_SC.begin(115200);
  uart_SC.halfduplex(true);
  RGB.control(true);
  RGB.color(0x00, 0x00, 0x00);
  pinMode(mode_SC, OUTPUT);
  timer_transmit.start();
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  Logger log("app");
  // The core of your code will likely live here.

  if(flag_transmit){
    flag_transmit = false;
    iiindex++;

    if((iiindex % 10) == 0){
      uart_SC.write(data_request, 15);
      receive(0);
      receive(12);
      return;
    }

    if((iiindex % 25) >12){
      digitalWrite(mode_SC, LOW);
    }else{
      digitalWrite(mode_SC, HIGH);
    }

    uart_SC.write(data_idle, 13);
  }
}

void receive(uint8_t bytes_to_read){
  Logger log("app.receive");
  String log_s;
  unsigned long calledTime = millis();
  uint8_t data[80];
  uint8_t bytes_read = 0;

  // bytes_to_read == 0인 경우 rx 버퍼를 비운다.
  if(!bytes_to_read){
    while(uart_SC.available()){

      uart_SC.read();
      bytes_read++;

      if(((millis() - calledTime) > TIMEOUT_RECV_SC) || (bytes_read > 80)){

        log_s = String("SC buf flushing timeout. num bytes ");
        log_s.concat(String(bytes_read));
        log.warn(log_s);

        return;
      }

    }
    return;
  }

  // 데이터를 받는 부분
  while(bytes_read < bytes_to_read){

    if(uart_SC.available()){

      data[bytes_read] = uart_SC.read();

      // packet의 시작을 감지하기 위한 부분
      if(bytes_read){
        bytes_read++;
      }else{
        if(data[bytes_read] == 0x55){
          bytes_read++;
          log.trace("packet started");
        }
      }

    }

    if(((millis() - calledTime) > TIMEOUT_RECV_SC) || (bytes_read > 80)){

      log_s = String("SC receiving timeout. num bytes ");
      log_s.concat(String(bytes_read));
      log.warn(log_s);

      return;
    }

  }

  log_s = String("receiving done. num bytes ");
  log_s.concat(bytes_read);
  log.trace(log_s);

  mode = data[6];



  if(mode > 1){
    RGB.color(0x00, 0xff, 0x00);
    log.info("eco");
    log.warn(String(mode));
  }else{
    RGB.color(0xff, 0xff, 0xff);
  }

}
