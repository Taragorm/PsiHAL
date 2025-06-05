/*
 * tca.h
 *
 * Created: 03/03/2025 13:16:50
 *  Author: Simon
 */


#ifndef TCA_H_
#define TCA_H_

#include <regutils.h>

enum class EventAction {
    PosEdge,
    AnyEdge,
    HighLevel,
    UpDown
};


enum class TcaClock {
    Div1, Div2, Div4, Div8,
    Div16, Div64, Div256, Div1028,
};
//=======================================================================
/**
 * @brief TCA control class for TCA running in Normal (16-bit) mode
   @param N Timer ordinal
 */
template<unsigned N>
class TCAControl
{
    static const unsigned CTRLA_CLKSEL_gm = 3<<1;
    static const unsigned EVCTRL_EVACT_gm = 3<<1;
    static const unsigned EVCTRL_CNTEI_gm = 1;

    inline static bool _beenReset;
    
public:


    static constexpr TCA_t& tmr() { return (&TCA0)[N]; }

    static bool beenReset() { return _beenReset; }
        
    static void reset(bool force)
    {
        if(force || !_beenReset)
        {
            _beenReset = true;
            tmr().SPLIT.CTRLA=0;                                //disable TCA0 and set divider to 1
            tmr().SPLIT.CTRLESET=TCA_SPLIT_CMD_RESET_gc|0x03;   //set CMD to RESET to do a hard reset of TCA0.
        }            
    }

    static void clockSelect(TcaClock cl)    { SETFIELD2(tmr().SINGLE.CTRLA, CTRLA_CLKSEL_gm, cl); }
    static void enable(bool st) { SETBIT(tmr().SINGLE.CTRLA,1); }

    static uint16_t count() { return tmr().SINGLE.CNT; }
    static void count(uint16_t v) { tmr().SINGLE.CNT = v; }

    static void eventAction(EventAction act)    { SETFIELD2(tmr().SINGLE.EVCTRL, EVCTRL_EVACT_gm, act); }
    static void eventCountEnable(bool st)       { SETBIT(tmr().SINGLE.EVCTRL, EVCTRL_CNTEI_gm); }
    static uint16_t evctrl() { return tmr().SINGLE.EVCTRL; }
    static bool eventCountEnable() { return evctrl() & EVCTRL_CNTEI_gm; }
    static EventAction eventAction() { return (EventAction)(evctrl() & EVCTRL_EVACT_gm); }
};
//=======================================================================

using TCA0Control = TCAControl<0>;

#if defined(TCA1)
using TCA1Control = TCAControl<1>;
#endif

#if defined(TCA2)
using TCA2Control = TCAControl<2>;
#endif

#if defined(TCA3)
using TCA3Control = TCAControl<3>;
#endif

#endif /* TCA_H_ */