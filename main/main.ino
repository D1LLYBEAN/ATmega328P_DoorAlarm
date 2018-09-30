// main.ino
// ATmega328P_DoorAlarm
// Dillon Mills

const unsigned long int FOSC = 16000000;          // Clock Speed (need data type for this)
const unsigned short int BAUD = 1200;             // Baud Rate

void setup()
{
  // ---------- Serial ---------- //
  
  // Set baud rate
  unsigned short int ubrr = (FOSC / (16 * BAUD)) - 1;
  UBRR0H = (unsigned char)(ubrr >> 8);
  UBRR0L = (unsigned char)ubrr;
  // Enable receiver and transmitter
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
  // Set frame format: 8data, 2stop bit
  UCSR0C = (1 << USBS0) | (3 << UCSZ00);

  Serial.print("Setup: Serial...COMPLETE\n");

  // ---------- Clock ---------- //

  Serial.print("Setup: Clock...");
  
  // # CLKPR - Clock Prescale Register
  // CLKPCE: Clock Prescaler Change Enable
  // CLKPS[3:0]: Clock Prescaler Select Bits 3-0
  CLKPR = (1 << CLKPCE); // CLKPCE = 1b, CLKPS[3:0] = 0000b, Mandatory to change CLKPS bits
  //                                                                           Prescaler | CIRCO 8 MHz |
  CLKPR |= (1 << CLKPS3) | (0 << CLKPS2) | (0 << CLKPS1) | (0 << CLKPS0);   //       256 |   31.25 kHz |
  //CLKPR |= (0 << CLKPS3) | (1 << CLKPS2) | (1 << CLKPS1) | (1 << CLKPS0); //       128 |   62.50 kHz |
  //CLKPR |= (0 << CLKPS3) | (1 << CLKPS2) | (1 << CLKPS1) | (0 << CLKPS0); //        64 |  125.00 kHz |
  //CLKPR |= (0 << CLKPS3) | (1 << CLKPS2) | (0 << CLKPS1) | (1 << CLKPS0); //        32 |  250.00 kHz |
  //CLKPR |= (0 << CLKPS3) | (1 << CLKPS2) | (0 << CLKPS1) | (0 << CLKPS0); //        16 |  500.00 kHz |
  //CLKPR |= (0 << CLKPS3) | (0 << CLKPS2) | (1 << CLKPS1) | (1 << CLKPS0); //         8 | 1000.00 kHz | (DEFAULT)
  //CLKPR |= (0 << CLKPS3) | (0 << CLKPS2) | (1 << CLKPS1) | (0 << CLKPS0); //         4 | 2000.00 kHz |
  //CLKPR |= (0 << CLKPS3) | (0 << CLKPS2) | (0 << CLKPS1) | (1 << CLKPS0); //         2 | 4000.00 kHz |
  //CLKPR |= (0 << CLKPS3) | (0 << CLKPS2) | (0 << CLKPS1) | (0 << CLKPS0); //         1 | 8000.00 kHz |

  Serial.print("COMPLETE\n");

  // ---------- Pins ---------- //

  Serial.print("Setup: Pins...");

  DDRD = (1 << DDD2); // set PIND2 as input.
  PORTD = (1 << PORTD2); // enable PIND2 pull-up.

  Serial.print("COMPLETE\n");
  
  // ---------- Timer ---------- //
  
  Serial.print("Setup: Timer...");

  // # TCCR0A - TC0 Control Register A
  // COM0An: Compare Output Mode for Channel A [n = 1:0]
  // COM0Bn: Compare Output Mode for Channel B [n = 1:0]
  // WGM0n: Waveform Generation Mode [n = 1:0]
  //                                      Mode | WGM02 | WGM01 | WGM00 | Timer/Counter Mode of Operation | TOP  | Update of OCR0x at | TOV Flag Set on
  TCCR0A = (0 << WGM01) | (0 << WGM00); // 0    | 0     | 0     | 0     | Normal                          | 0xFF | Immediate          | 0xFF

  // # TCCR0B - TC0 Control Register B
  // FOC0A: Force Output Compare A
  // FOC0B: Force Output Compare B
  // WGM02: Waveform Generation Mode
  // CS0[2:0]: Clock Select 0 [n = 0..2]
  TCCR0B = (0 << WGM02); // see TCCR0A.
  //                                                   CA02 | CA01 | CS00 | Clock              | Description
  TCCR0B |= (0 <<CS02) | (0 << CS01) | (0 << CS00); // 0    | 0    | 0    |     0.000000000 Hz | No clock source (Timer/Counter stopped).
  //TCCR0B |= (0 <<CS02) | (0 << CS01) | (1 << CS00); // 0    | 0    | 1    | 31250.000000000 Hz | clkIO / 1 (No prescaling)
  //TCCR0B |= (0 <<CS02) | (1 << CS01) | (0 << CS00); // 0    | 1    | 0    |  3906.250000000 Hz | clkIO / 8 (From prescaler)
  //TCCR0B |= (0 <<CS02) | (1 << CS01) | (1 << CS00); // 0    | 1    | 1    |   488.281250000 Hz | clkIO / 64 (From prescaler)
  //TCCR0B |= (1 <<CS02) | (0 << CS01) | (0 << CS00); // 1    | 0    | 0    |   122.070312500 Hz | clkIO / 256 (From prescaler)
  //TCCR0B |= (1 <<CS02) | (0 << CS01) | (1 << CS00); // 1    | 0    | 1    |    30.517578125 Hz | clkIO / 1024 (From prescaler)

  // # TCNT0 - TC0 Counter Value Register
  // TCNT0[7:0]: TC0 Counter Value
  TCNT0 = 0;

  // # OCR0A - TC0 Output Compare Register A
  // OCR0A[7:0]: Output Compare 0 A
  OCR0A = 0;

  // # OCR0B - TC0 Output Compare Register B
  // OCR0B[7:0]: Output Compare 0 B
  OCR0B = 0;
  
  Serial.print("COMPLETE\n");
  
  // ---------- Interrupts ---------- //
  
  Serial.print("Setup: Interrupts...");

  // # EICRA - External Interrupt Control Register A
  // ISC1n: Interrupt Sense Control 1 [n = 1:0]
  // ISC0n: Interrupt Sense Control 0 [n = 1:0]
  //EICRA = (0 << ISC01) | (0 << ISC00); // The low level of INT0 generates an interrupt request.
  EICRA = (0 << ISC01) | (1 << ISC00); // Any logical change on INT0 generates an interrupt request.
  //EICRA = (1 << ISC01) | (0 << ISC00); // The falling edge of INT0 generates an interrupt request.
  //EICRA = (1 << ISC01) | (1 << ISC00); // The rising edge of INT0 generates an interrupt request.

  // # EIMSK - External Interrupt Mask Register
  // INT1: External Interrupt Request 1 Enable
  // INT0: External Interrupt Request 0 Enable
  EIMSK = (1 << INT0);

  // # TIMSK0 - TC0 Interrupt Mask Register
  // BIT2: OCIEB: Timer/Counter0, Output Compare B Match Interrupt Enable
  // BIT1: OCIEA: Timer/Counter0, Output Compare A Match Interrupt Enable
  // BIT0: TOIE: Timer/Counter0, Overflow Interrupt Enable
  TIMSK0 = (0 << 2) | (0 << 1) | (1 << 0);
  
  // # SREG - AVR Status Register
  // BIT7 (I): Global Interrupt Enable
  SREG = (1 << 7);

  // <Add more interrupt stuff here.>
  
  Serial.print("COMPLETE\n");

  // ---------- Power ---------- //

  Serial.print("Setup: Power...");

  // <Add power stuff here.>

  Serial.print("COMPLETE\n");
}

void loop()
{
  Serial.print("ping\n");
  delay(1000);
}

ISR(INT0_vect) // PD2
{
  Serial.print("Alert: Door Activity - ");
  // <Add door sensor interrupt stuff here.>
  // Read PINxn?
  
  if((PIND & 4) > 0)
  {
    // Do something
    Serial.print("PIND2 = ");
    Serial.println(PIND & 4);
    // activate timer, or just sound alarm, or whatever.
  }
  else
  {
    Serial.print("PIND2 = 0\n");
    // reset timer
  }
}

/*ISR(TOV0_vect)
{
  // <Add door timer interrupt stuff here.>
}*/
