/*
 * regutils.h
 *
 * Created: 03/03/2025 13:27:05
 *  Author: Simon
 */


#ifndef REGUTILS_H_
#define REGUTILS_H_

/** Find first set bit in a byte */
#define FIRSTSETBIT8(_X) \
    (_X & 1 ? 0 : _X & 2 ? 1 : _X & 4 ? 2 : _X & 8 ? 3 : \
    _X & 0x10 ? 4 : _X & 0x20 ? 5 : _X & 0x40 ? 6 : _X & 0x80 ? 7 : \
    0)

#define PROTWRITE(_E)  CPU_CCP = CCP_IOREG_gc; _E

#define SETFIELD(_R, _M, _V) _R = ( ((uint8_t)(_V)) << _M##_gp ) | (_R & ~_M##_gm)
#define GETFIELD(_R, _M)   ((_R & _M##_gm)  >> _M##_gp )

#define SETFIELD2(_R, _M, _V ) _R = ( ((uint8_t)(_V)) << FIRSTSETBIT8(_M) ) | (_R & ~_M)
#define GETFIELD2(_R, _M )  ( (_R & _M) << FIRSTSETBIT8(_M) )

#define SETBIT(_R,_M) { auto tmp = st ? (_R | _M) : (_R & ~_M); CPU_CCP = CCP_IOREG_gc; _R = tmp; }


#endif /* REGUTILS_H_ */