/*
 * rtc.h
 *
 * Abstractions for The arduino System, RTC and PIT systems
 * Created: 03/03/2025 08:33:06
 *  Author: Simon
 */


#ifndef CLOCKS_H_
#define CLOCKS_H_

#include <arduino.h>
#include <regutils.h>

/*
void RTCSetup () {
    uint8_t temp;
    // Initialize 32.768kHz Oscillator:

    // Disable oscillator:
    temp = CLKCTRL.XOSC32KCTRLA & ~CLKCTRL_ENABLE_bm;

    // Enable writing to protected register
    CPU_CCP = CCP_IOREG_gc;
    CLKCTRL.XOSC32KCTRLA = temp;

    while (CLKCTRL.MCLKSTATUS & CLKCTRL_XOSC32KS_bm);   // Wait until XOSC32KS is 0

    temp = CLKCTRL.XOSC32KCTRLA & ~CLKCTRL_SEL_bm;      // Use External Crystal

    // Enable writing to protected register
    CPU_CCP = CCP_IOREG_gc;
    CLKCTRL.XOSC32KCTRLA = temp;

    temp = CLKCTRL.XOSC32KCTRLA | CLKCTRL_ENABLE_bm;    // Enable oscillator

    // Enable writing to protected register
    CPU_CCP = CCP_IOREG_gc;
    CLKCTRL.XOSC32KCTRLA = temp;

    // Initialize RTC
    while (RTC.STATUS > 0);                             // Wait until registers synchronized

    // 32.768kHz External Crystal Oscillator (XOSC32K)
    RTC.CLKSEL = RTC_CLKSEL_TOSC32K_gc;

    RTC.DBGCTRL = RTC_DBGRUN_bm;                        // Run in debug: enabled

    RTC.PITINTCTRL = RTC_PI_bm;                         // Periodic Interrupt: enabled

    // RTC Clock Cycles 32768, enabled ie 1Hz interrupt
    RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc | RTC_PITEN_bm;
}

*/
//==================================================================================
/**
 * Clock control class, entirely static
 */
class ClockControl
{
public:
    enum class XtlStartup
    {
        CY1K, CY16K, CY32K, CY64K
    };

    static void disableXtal()
    {
        PROTWRITE(CLKCTRL_XOSC32KCTRLA = 0);
        while(xtalStable()) ; // wait till stopped
    }

    static void enableXtal(
                        bool en,
                        bool runStdby,
                        XtlStartup startcyl = XtlStartup::CY1K,
                        bool extSel = false)
    {
        //Serial.printf("Xctrl1 %u\r\n", CLKCTRL_XOSC32KCTRLA);

        auto m = (uint8_t) en
                    | ((uint8_t) runStdby) << 1
                    | ((uint8_t) extSel) << 2
                    | ((uint8_t) startcyl) << 4
                    ;

        PROTWRITE(CLKCTRL_XOSC32KCTRLA = m);

        //Serial.printf("Xctrl2 %u\r\n", CLKCTRL_XOSC32KCTRLA);
    }

    static bool xtalEnabled()   { return CLKCTRL_XOSC32KCTRLA & CLKCTRL_ENABLE_bm; }
    static bool extStarted()    { return CLKCTRL_MCLKSTATUS & CLKCTRL_EXTS_bm; }
    static bool xtalStable()    { return CLKCTRL_MCLKSTATUS & CLKCTRL_XOSC32KS_bm; }
    static bool osclpStable()   { return CLKCTRL_MCLKSTATUS & CLKCTRL_OSC32KS_bm; }
    static bool osc20MStable()   { return CLKCTRL_MCLKSTATUS & CLKCTRL_OSC20MS_bm; }
    static bool mainOscChanging()   { return CLKCTRL_MCLKSTATUS & CLKCTRL_SOSC_bm; }
    static void waitForXtal()
    {
        //Serial.printf("MclkStatus1 %u\r\n", CLKCTRL_MCLKSTATUS);
        //while(!xtalStable());
        //Serial.printf("MclkStatus2 %u\r\n", CLKCTRL_MCLKSTATUS);
    }
};

//==================================================================================
/**
 * @brief RTC control class, entirely static.
 * Wraps common RTC & PIT functions in a less arcane wrapper.
 * Synchronization is used where required.

  Typical ISR implementations:

@verbatim
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ISR(RTC_CNT_vect)
{
    if ( RtcControl::gotOvfInterrupt() )
    {
        // handle OVF
    }

    if ( RtcControl::gotCmpInterrupt()  )
    {
        // handle CMP
    }
    RtcControl::clearInterruptFlags();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ISR(RTC_PIT_vect)
{
    if(RtcControl::gotPitInterrupt() )
    {
        // handle PIT
    }
    RtcControl::clearPitInterruptFlags();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@endverbatim
 */
class RtcControl
{
public:
    enum class Prescale {
        DIV1, DIV2, DIV4, DIV8, DIV16, DIV32,
        DIV64, DIV128, DIV256, DIV512, DIV1K,
        DIV2K, DIV4K, DIV8K, DIV16K, DIV32K
    };

    enum class Period {
        OFF, CYC4, CYC8, CYC16, CYC32,
        CYC64, CYC128, CYC256, CYC512, CYC1K,
        CYC2K, CYC4K, CYC8K, CYC16K, CYC32K
    };

    static bool cmpBusy() { return RTC_STATUS & RTC_CMPBUSY_bm; }
    static bool perBusy() { return RTC_STATUS & RTC_PERBUSY_bm; }
    static bool cntBusy() { return RTC_STATUS & RTC_CNTBUSY_bm; }
    static bool ctrlaBusy() { return RTC_STATUS & RTC_CTRLABUSY_bm; }
    static bool pitBusy() { return RTC_PITSTATUS & RTC_CTRLBUSY_bm; }

    static void cmpWait() { while(cmpBusy()); }
    static void perWait() { while(perBusy()); }
    static void cntWait() { while(cntBusy()); }
    static void ctrlaWait() { while(ctrlaBusy()); }
    static void pitWait() { while(pitBusy()); }

    static void clockLP32k()            {  RTC_CLKSEL = 0; }
    static void clockLP1k()             {  RTC_CLKSEL = 1; }
    /// Set crystal Osc source; @note it *also* has to be enabled; see ClockControl::enableXtal()
    static void clockXT32k()            {  RTC_CLKSEL = 2; }
    static void clockEXT()              {  RTC_CLKSEL = 3; }

    static bool is32kClocked()          { return RTC_CLKSEL==0 || RTC_CLKSEL==2; }
        
    static void runInSleep(bool st)     { ctrlaWait(); SETBIT(RTC_CTRLA,RTC_RUNSTDBY_bm); }
    static void prescale(Prescale p)    { ctrlaWait(); SETFIELD(RTC_CTRLA, RTC_PRESCALER, p); }
    static Prescale prescale()          { return (Prescale)GETFIELD(RTC_CTRLA, RTC_PRESCALER); }
        
    static void clearPrescaler()
    {
        auto old = prescale();
        prescale(Prescale::DIV1);
        prescale(old);
    }        

    static void setPrescaleAndClear(Prescale p)
    {
        prescale(Prescale::DIV1);
        prescale(p);
    }        
        
    static void enable(bool st)         { ctrlaWait(); SETBIT(RTC_CTRLA,RTC_RTCEN_bm); }
    static bool enabled()               { return RTC_CTRLA & RTC_RTCEN_bm; }


    static void enableCmpInterrupt(bool st) { SETBIT(RTC_INTCTRL,RTC_CMP_bm); }
    static void enableOvfInterrupt(bool st) { SETBIT(RTC_INTCTRL,RTC_OVF_bm); }

    static bool cmpInterruptFlag()  { return RTC_INTFLAGS & RTC_CMP_bm; }
    static bool ovfInterruptFlag()  { return RTC_INTFLAGS & RTC_OVF_bm; }
    static bool gotCmpInterrupt()   { return (RTC.INTCTRL & RTC_CMP_bm) && (RTC.INTFLAGS & RTC_CMP_bm); }
    static bool gotOvfInterrupt()   { return (RTC.INTCTRL & RTC_OVF_bm) && (RTC.INTFLAGS & RTC_OVF_bm); }

    static void clearInterruptFlags() { RTC_INTFLAGS = RTC_OVF_bm|RTC_CMP_bm; }

    static uint16_t count()         { return RTC_CNT; }
    static void count(uint16_t v)   { cntWait(); RTC_CNT = v; }

    static uint16_t period()        { return RTC_PER; }
    static void period(uint16_t v)  { perWait(); RTC_PER = v; }

    static uint16_t compare()       { return RTC_CMP; }
    static void compare(uint16_t v) { cmpWait(); RTC_CMP = v; }

    static void runInDebug(bool st) { SETBIT(RTC_DBGCTRL,RTC_DBGRUN_bm); }

    static void pitPeriod(Period p) { pitWait(); SETFIELD(RTC_PITCTRLA, RTC_PERIOD, p); }
    static Period pitPeriod() { return (Period)GETFIELD(RTC_PITCTRLA, RTC_PERIOD); }

    static void enablePit(bool st)  { pitWait(); SETBIT(RTC_PITCTRLA,RTC_PITEN_bm); }
    static bool pitEnabled() { return RTC_PITCTRLA & RTC_PITEN_bm; }
        
    static void enablePitInterrupt(bool st) { SETBIT(RTC_PITINTCTRL,RTC_PI_bm); }
    static bool pitInterruptFlag()          { return RTC_PITINTFLAGS & RTC_PI_bm; }
    static bool gotPitInterrupt()
    {
        return (RTC_PITINTCTRL & RTC_PI_bm)
            && (RTC_PITINTFLAGS & RTC_PI_bm);
    }

    static void clearPitInterruptFlags()    { RTC_PITINTFLAGS = RTC_PI_bm; }

    static void runPitInDebug(bool st)      { SETBIT(RTC_PITDBGCTRL,RTC_DBGRUN_bm); }

};
//==================================================================================


#endif /* RTC_H_ */