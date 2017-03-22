/** TDA7419 PreAmp library, I2C
 *
 * @Author: Dan Cohen
 */
#ifndef TDA_7419_H
#define TDA_7419_H

#include <inttypes.h>

 /** TDA7419 PreAmp library
 *
 * Includes the commands for volume, fader, subwoofer and tone controls
 *
 */
class TDA7419_control {
public:

    /** Create a new TDA7419 communication interface
     *
     * @param sda is the pin for I2C SDA
     * @param scl is the pin for I2C SCL
     */
    TDA7419_control();

    /** Directly set the TDA7419 Master volume (0->11)
    *
    */
    void setVolume  (int volume);
    /** Read the the TDA7419 Master volume (0->11)
    *
    */
    int  readVolume ();
    /** Increase the current volume (if below 11)
    *
    */
    int  increaseVolume();
    /** Decrease the current volume (if more than 0)
    *
    */
    int  decreaseVolume();
    /** Select the input (1->4)
    *   Stereo inputs are 1->3
    */
    void setInput   (int input);
    /** Read currently selected input
    *
    */
    int  readInput  ();

    /** Increase treble level (-5 -> 5)
    *
    */
    int  increaseTreble();
    /** Decrease treble level (-5 -> 5)
    *
    */
    int  decreaseTreble();
    /** Read currently set treble level
    *
    */
    int  readTreble    ();

    /** Increase middle level (-5 -> 5)
    *
    */
    int  increaseMiddle();
    /** Decrease middle level (-5 -> 5)
    *
    */
    int  decreaseMiddle();
    /** Read currently set middle level
    *
    */
    int  readMiddle    ();

    /** Increase Bass level (-5 -> 5)
    *
    */
    int  increaseBass();
    /** Decrease bass level (-5 -> 5)
    *
    */
    int  decreaseBass();
    /** Read currently set middle level
    *
    */
    int  readBass    ();

    /** Adjust the volume of each channel attenuator
    *
    *  @param speakerNumber
    *        1 - left front,
    *        2 - right front,
    *        3 - back left,
    *        4 - back right,
    *        5 - subwoofer,
    *        6 - volume of mix channel
    */
    int  increaseSpeaker (int speakerNumber);
    /** Adjust the volume of each channel attenuator
    *
    *  @param speakerNumber
    *        1 - left front,
    *        2 - right front,
    *        3 - back left,
    *        4 - back right,
    *        5 - subwoofer,
    *        6 - volume of mix channel
    */
    int  decreaseSpeaker (int speakerNumber);

    /** Read the value that is currently set for each attenuator
    *  @param speakerNumber  match the increase and decrease parameter values
    */
    int  readSpeaker     (int speakerNumber);

    /** Enable the mix input (by default this mixes to the front two channels
    * @return The return is the state of the mix (1 = on, 0 = 0ff) after this call
    */
    int toggleMix();

    /** Read if Mix is enabled or not
    * @return The return is the state of the mix (1 = on, 0 = 0ff) after this call
    */
    int readMix();

private:
    ////////////////////////////////////
    // register addresses for TDA7419 //
    ////////////////////////////////////
    int _volume;
    int _input;

    int _mute;
    int _mix;

    // for register 4 Treble Filter
    int  _referenceInE;
    int  _trebleCenterFreq;
    int  _treble;

    // for middle frequecy filter
    int  _middleSoftStep;
    int  _middleQ;
    int  _middle;

    // for bass frequecy filter
    int  _bassSoftStep;
    int  _bassQ;
    int  _bass;

    // for output attenuators
    int _atten_lf;
    int _atten_rf;
    int _atten_lr;
    int _atten_rr;
    int _atten_mix;
    int _atten_sub;

    void writeToTDA7419     (int command, int value);
    void setAttenuationReg  (int regAddr, int attenuation);
    int  calcToneAttenuationReg(int attenuation);
    void updateTDA7419Reg();
    void init();

};

#endif
