/** PreampTDA7419 Library
 *
 * @Author: Dan Cohen
 * Modified for RPI : Cedric Paille
 *
 */

#include "tda7419.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "wiringPiI2C.h"

// I2C address for TDA7419
#define TDA7419_ADDRESS   0x44

#define TDA_MAIN_SOURCE   0
#define TDA_MAIN_LOUD     1   | 0x40
#define TDA_SOFTMUTE      2   | 0x40
#define TDA_VOLUME        3   | 0x40
#define TDA_TREBLE        4   | 0x40
#define TDA_MIDDLE        5   | 0x40
#define TDA_BASS          6   | 0x40
#define TDA_SECOND_SOURCE 7   | 0x40
#define TDA_SUB_MID_BASS  8   | 0x40
#define TDA_MIX_GAIN      9   | 0x40
#define TDA_ATTEN_LF      10  | 0x40
#define TDA_ATTEN_RF      11  | 0x40
#define TDA_ATTEN_LR      12  | 0x40
#define TDA_ATTEN_RR      13  | 0x40
#define TDA_ATTEN_MIX     14  | 0x40
#define TDA_ATTEN_SUB     15  | 0x40
#define TDA_SPECTRUM      16  | 0x40

TDA7419_control::TDA7419_control()
{
	init();
}

void
TDA7419_control::init()
{
    _volume           = 6;
    _input            = 1;

    _mute             = 0;
    _mix              = 0;

    // for register 4 Treble Filter
    _referenceInE     = 0;
    _trebleCenterFreq = 0;
    _treble           = 0;

    // for middle frequecy filter
    _middleSoftStep   = 0;
    _middleQ          = 0;
    _middle           = 0;

    _atten_lf         =  9;
    _atten_rf         =  9;
    _atten_lr         =  9;
    _atten_rr         =  9;
    _atten_mix        =  9;
    _atten_sub        =  9;
}

void TDA7419_control::writeToTDA7419 (int command, int value)
{
//    _device.start();
//    transmissionSuccessful  = _device.write(TDA7419_ADDRESS<<1);
//    transmissionSuccessful |= _device.write(command);
//    transmissionSuccessful |= _device.write(value);
//    _device.stop();
	int fd  = wiringPiI2CSetup(TDA7419_ADDRESS);
	if (fd < 0)
		printf("TDA7419_control::writeToTDA7419 : cannot open I2C device.\n");
	wiringPiI2CWriteReg8 (fd, command, value);
	close(fd);
}

/////////////////////////////////
// set the speaker attenuators //
/////////////////////////////////
// attenuation can be set from 0 to 11 and this is mapped to the
// values that the TDA7419 uses for it's attenuation (0->h60)

//
//  (FL/FR/RL/RR/SWL/SWR) (13-18)
void TDA7419_control::setAttenuationReg(int regAddr, int attenuation)
{
    int regAtten;
    if (attenuation == 11) {
        regAtten = 13;
    } else if (attenuation == 10) {
        regAtten = 6;
    } else {
        regAtten = (99-(attenuation*9));
    }
    writeToTDA7419(regAddr, regAtten);
}

int TDA7419_control::calcToneAttenuationReg(int attenuation)
{
    int regAtten;
    if (attenuation > 0) {
        regAtten = 16 + (attenuation * 3);
    } else if (attenuation == 0) {
        regAtten = 0;
    } else if (attenuation  < 0) {
        regAtten = 0 - (attenuation * 3);
    }
    return (regAtten);
}

// update all of the registers in the TDA7419
void TDA7419_control::updateTDA7419Reg()
{
    // int s_test           = 17  | 0x40;

    //////////////////////////////////////////////////////////////////
    // Calculate actual register values from the variables that the //
    // buttons control                                              //
    //////////////////////////////////////////////////////////////////

    //////////////////////////
    // update the registers //
    //////////////////////////
    writeToTDA7419(TDA_MAIN_SOURCE,  ( (0x78) | (_input & 0x3) ) );
    writeToTDA7419(TDA_MAIN_LOUD,      (0xc0));
    writeToTDA7419(TDA_SOFTMUTE,       (0xa7));
    setAttenuationReg(TDA_VOLUME, _volume);


    // tone register attenuation isn't simple so moving that
    // calculation to a separate function
    // locking softwtep as '0' because that is on and I think we
    // want soft step!
    writeToTDA7419(TDA_TREBLE,
                   ( (0                                      &  0x1 ) << 7 ) |
                   ( (1                                      &  0x3 ) << 5 ) |
                   ( (calcToneAttenuationReg(_treble)        & 0x1f )      ) );

    writeToTDA7419(TDA_MIDDLE,
                   ( (0                                      &  0x1 ) << 7 ) |
                   ( (1                                      &  0x3 ) << 5 ) |
                   ( (calcToneAttenuationReg(_middle)        & 0x1f )      ) );

    writeToTDA7419(TDA_BASS,
                   ( (0                                      &  0x1 ) << 7 ) |
                   ( (1                                      &  0x3 ) << 5 ) |
                   ( (calcToneAttenuationReg(_bass)          & 0x1f )      ) );


    // this register allows the second source to be routed to the rear speakers
    // not useful in the context of this project
    writeToTDA7419(TDA_SECOND_SOURCE, (0x07));

    // this is the subwoofer cut-off frequency
    // 11 which is 160Khz)
    writeToTDA7419(TDA_SUB_MID_BASS,  (0x63));

    // mix to the front speakers,  enable the sub,  no gain
    if (_mix == 1) {
        writeToTDA7419(TDA_MIX_GAIN,    (0xf7));
    } else {
        writeToTDA7419(TDA_MIX_GAIN,    (0xf0));
    }

    setAttenuationReg(TDA_ATTEN_LF  , _atten_lf  );
    setAttenuationReg(TDA_ATTEN_RF  , _atten_rf  );
    setAttenuationReg(TDA_ATTEN_LR  , _atten_lr  );
    setAttenuationReg(TDA_ATTEN_RR  , _atten_rr  );

    setAttenuationReg(TDA_ATTEN_MIX , _atten_mix );
    setAttenuationReg(TDA_ATTEN_SUB , _atten_sub );

    writeToTDA7419       (TDA_SPECTRUM,      (0x09));

}

/* setVolume: This sets the volume within the valid range of 0->11
  return indicates it was successfully set */
void TDA7419_control::setVolume(int volume)
{
    if (volume > 11) {
        _volume = 11;
    } else if (volume < 0) {
        volume = 0;
    } else {
        _volume = volume;
    }
    updateTDA7419Reg();
}

/* readVolume:  return the volume level that is currently set */
int TDA7419_control::readVolume()
{
    return (_volume);
}
/* readVolume:  return the volume level that is currently set */
int TDA7419_control::increaseVolume()
{
    _volume++;
    setVolume(_volume);
    return (_volume);
}
/* readVolume:  return the volume level that is currently set */
int TDA7419_control::decreaseVolume()
{
    _volume--;
    setVolume(_volume);
    return (_volume);
}

void TDA7419_control::setInput(int input)
{
    if (input > 3) {
        _input = 3;
    } else if (input < 0) {
        input = 0;
    } else {
        _input = input;
    }
    updateTDA7419Reg();
}

int TDA7419_control::readInput()
{
    return (_input);
}

int  TDA7419_control::increaseTreble()
{
    if (_treble < 5) {
        _treble++;
    }
    updateTDA7419Reg();
    return(_treble);
}

int  TDA7419_control::decreaseTreble()
{
    if (_treble > -5) {
        _treble--;
    }
    updateTDA7419Reg();
    return(_treble);
}

int TDA7419_control::readTreble()
{
    return (_treble);
}

int  TDA7419_control::increaseMiddle()
{
    if (_middle < 5) {
        _middle++;
    }
    updateTDA7419Reg();
    return(_middle);
}

int  TDA7419_control::decreaseMiddle()
{
    if (_middle > -5) {
        _middle--;
    }
    updateTDA7419Reg();
    return(_middle);
}

int TDA7419_control::readMiddle()
{
    return (_middle);
}

int  TDA7419_control::increaseBass()
{
    if (_bass < 5) {
        _bass++;
    }
    updateTDA7419Reg();
    return(_bass);
}

int  TDA7419_control::decreaseBass()
{
    if (_bass > -5) {
        _bass--;
    }
    updateTDA7419Reg();
    return(_bass);
}

int TDA7419_control::readBass()
{
    return (_bass);
}

int  TDA7419_control::increaseSpeaker (int speakerNumber) {
  switch (speakerNumber) {
    case (1): if (_atten_lf  < 11) { _atten_lf++;  }; updateTDA7419Reg(); return( _atten_lf  );
    case (2): if (_atten_rf  < 11) { _atten_rf++;  }; updateTDA7419Reg(); return( _atten_rf  );
    case (3): if (_atten_lr  < 11) { _atten_lr++;  }; updateTDA7419Reg(); return( _atten_lr  );
    case (4): if (_atten_rr  < 11) { _atten_rr++;  }; updateTDA7419Reg(); return( _atten_rr  );
    case (5): if (_atten_sub < 11) { _atten_sub++; }; updateTDA7419Reg(); return( _atten_sub );
    case (6): if (_atten_mix < 11) { _atten_mix++; }; updateTDA7419Reg(); return( _atten_mix );
  }
  return (_atten_lf );
}

int  TDA7419_control::decreaseSpeaker (int speakerNumber) {
  switch (speakerNumber) {
    case (1): if (_atten_lf  >  0) { _atten_lf--;  }; updateTDA7419Reg(); return( _atten_lf  );
    case (2): if (_atten_rf  >  0) { _atten_rf--;  }; updateTDA7419Reg(); return( _atten_rf  );
    case (3): if (_atten_lr  >  0) { _atten_lr--;  }; updateTDA7419Reg(); return( _atten_lr  );
    case (4): if (_atten_rr  >  0) { _atten_rr--;  }; updateTDA7419Reg(); return( _atten_rr  );
    case (5): if (_atten_sub >  0) { _atten_sub--; }; updateTDA7419Reg(); return( _atten_sub );
    case (6): if (_atten_mix >  0) { _atten_mix--; }; updateTDA7419Reg(); return( _atten_mix );
  }
  return (_atten_lf );
}

int  TDA7419_control::readSpeaker (int speakerNumber) {
  switch (speakerNumber) {
    case (1):  return( _atten_lf  );
    case (2):  return( _atten_rf  );
    case (3):  return( _atten_lr  );
    case (4):  return( _atten_rr  );
    case (5):  return( _atten_sub );
    case (6):  return( _atten_mix );
  }
  return (_atten_lf );
}

int TDA7419_control::toggleMix() {
  _mix = !_mix;
  updateTDA7419Reg();
  return (_mix);
}

int TDA7419_control::readMix() {
  return (_mix);
}
