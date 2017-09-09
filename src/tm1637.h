/*
 * TM1637 Library
 *
 * Created 24.08.2017 07:33
 * Author Kevin Luedemann
 */

#define sclport        PORTB
#define sclddr         DDRB

#define sdaport        PORTB
#define sdaddr         DDRB

#define sdapin         PINB
#define sclpin         PINB

#define scl            PB1
#define sda            PB2


#define bb_sda_hi()    sdaport|=((1<<sda))
#define bb_sda_lo()    sdaport&=(~(1<<sda))

#define bb_scl_hi()    sclport|=((1<<scl))
#define bb_scl_lo()    sclport&=(~(1<<scl))

#define puls_us        10
#define puls_len()     _delay_us(puls_us)


/* ----------------------------------------------------------
                     Globale Variable
   ---------------------------------------------------------- */

uint8_t    hellig    = 15;                // beinhaltet Wert fuer die Helligkeit (erlaubt: 0x00 .. 0x0f);
uint8_t    tm1637_dp = 0;                 // 0 : Doppelpunkt abgeschaltet
                                          // 1 : Doppelpunkt sichtbar
                                          //     tm1637_dp wird beim Setzen der Anzeigeposition 1 verwendet
                                          //     und hat erst mit setzen dieser Anzeige einen Effekt

uint8_t    led7sbmp[16] =                // Bitmapmuster fuer Ziffern von 0 .. F
                { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07,
                  0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71 };


/* ----------------------------------------------------------
                         PROTOTYPEN
   ---------------------------------------------------------- */

void tm1637_start(void);
void tm1637_stop(void);
void tm1637_write (uint8_t value);
void tm1637_init(void);
void tm1637_clear(void);
void tm1637_selectpos(char nr);
void tm1637_setbright(uint8_t value);
void tm1637_setbmp(uint8_t pos, uint8_t value);
void tm1637_setzif(uint8_t pos, uint8_t zif);
void tm1637_setseg(uint8_t pos, uint8_t seg);
void tm1637_setdez(int value);
void tm1637_sethex(uint16_t value);


/* ----------------------------------------------------------
                   TM1637 - Definitionen
   ---------------------------------------------------------- */

/*  ------------------- Kommunikation -----------------------

    Der Treiberbaustein TM1637 wird etwas "merkwuerdig
    angesprochen. Er verwendet zur Kommunikation ein I2C
    Protokoll, jedoch OHNE eine Adressvergabe. Der Chip ist
    somit IMMER angesprochen. Aus diesem Grund wird die
    Kommunikation mittels Bitbanging vorgenommen. Hierfuer
    kann jeder freie I/O Anschluss des Controllers verwendet
    werden (siehe defines am Anfang).
   ---------------------------------------------------------- */

void tm1637_start(void)              // I2C Bus-Start
{
  bb_scl_hi();
  bb_sda_hi();
  puls_len();
  bb_sda_lo();
}

void tm1637_stop(void)               // I2C Bus-Stop
{
  bb_scl_lo();
  puls_len();
  bb_sda_lo();
  puls_len();
  bb_scl_hi();
  puls_len();
  bb_sda_hi();
}

void tm1637_write (uint8_t value)    // I2C Bus-Datentransfer
{
  uint8_t i;

  for (i = 0; i <8; i++)
  {
    bb_scl_lo();
    if (value & 0x01) { bb_sda_hi(); }
                   else { bb_sda_lo(); }
    puls_len();
    value = value >> 1;
    bb_scl_hi();
    puls_len();
  }
  bb_scl_lo();
  puls_len();                        // der Einfachheit wegen wird ACK nicht abgefragt
  bb_scl_hi();
  puls_len();
  bb_scl_lo();

}

/*  ----------------------------------------------------------
                      Benutzerfunktionen
    ---------------------------------------------------------- */


 /*  ------------------- SELECTPOS ---------------------------

        waehlt die zu beschreibende Anzeigeposition aus
     --------------------------------------------------------- */
void tm1637_selectpos(char nr)
{
  tm1637_start();
  tm1637_write(0x40);                // Auswahl LED-Register
  tm1637_stop();

  tm1637_start();
  tm1637_write(0xc0 | nr);           // Auswahl der 7-Segmentanzeige
}

/*  ----------------------- SETBRIGHT ------------------------

       setzt die Helligkeit der Anzeige
       erlaubte Werte fuer Value sind 0 .. 15
    ---------------------------------------------------------- */
void tm1637_setbright(uint8_t value)
{
  tm1637_start();
  tm1637_write(0x80 | value);        // unteres Nibble beinhaltet Helligkeitswert
  tm1637_stop();
}

/*  ------------------------- CLEAR -------------------------

       loescht die Anzeige auf dem Modul
    --------------------------------------------------------- */
void tm1637_clear(void)
{
  uint8_t i;

  tm1637_selectpos(0);
  for(i=0;i<6;i++) { tm1637_write(0x00); }
  tm1637_stop();

  tm1637_setbright(hellig);

}

/*  ---------------------- SETBMP ---------------------------
       gibt ein Bitmapmuster an einer Position aus
    --------------------------------------------------------- */
void tm1637_setbmp(uint8_t pos, uint8_t value)
{
  tm1637_selectpos(pos);             // zu beschreibende Anzeige waehlen

  if (pos== 1)
  {
    if (tm1637_dp) { value |= 0x80; }
  }
  tm1637_write(value);               // Bitmuster value auf 7-Segmentanzeige ausgeben
  tm1637_stop();

}

/*  ---------------------- SETZIF ---------------------------
       gibt ein Ziffer an einer Position aus
       Anmerkung: das Bitmuster der Ziffern ist in
                  led7sbmp definiert
    --------------------------------------------------------- */
void tm1637_setzif(uint8_t pos, uint8_t zif)
{
  tm1637_selectpos(pos);             // zu beschreibende Anzeige waehlen

  zif= led7sbmp[zif];
  if (pos== 1)
  {
    if (tm1637_dp) { zif |= 0x80; }
  }
  tm1637_write(zif);               // Bitmuster value auf 7-Segmentanzeige ausgeben
  tm1637_stop();

}
/*  ----------------------- SETSEG --------------------------
       setzt ein einzelnes Segment einer Anzeige

       pos: Anzeigeposition (0..3)
       seg: das einzelne Segment (0..7 siehe oben)
    --------------------------------------------------------- */
void tm1637_setseg(uint8_t pos, uint8_t seg)
{

  tm1637_selectpos(pos);             // zu beschreibende Anzeige waehlen
  tm1637_write(1 << seg);
  tm1637_stop();

}

/*  ----------------------- SETDEZ --------------------------
       gibt einen 4-stelligen dezimalen Wert auf der
       Anzeige aus
    --------------------------------------------------------- */
void tm1637_setdez(int value)
{
  uint8_t i,v;

  for (i= 4; i> 0; i--)
  {
    v= value % 10;
    tm1637_setbmp(i-1, led7sbmp[v]);
    value= value / 10;
  }
}

/*  ----------------------- SETHEX --------------------------
       gibt einen 4-stelligen hexadezimalen Wert auf der
       Anzeige aus
    --------------------------------------------------------- */
void tm1637_sethex(uint16_t value)
{
  uint8_t i,v;

  for (i= 4; i> 0; i--)
  {
    v= value % 0x10;
    tm1637_setbmp(i-1, led7sbmp[v]);
    value= value / 0x10;
  }
}

/*  ----------------------- INIT ----------------------------
       initialisiert die Anschluesse des Controllers zur
       Kommunikation als Ausganege und loescht die Anzeige
    ---------------------------------------------------------- */
void tm1637_init(void)
{
  sdaddr|= (1 << sda);
  sclddr|= (1 << scl);
  tm1637_clear();
}
