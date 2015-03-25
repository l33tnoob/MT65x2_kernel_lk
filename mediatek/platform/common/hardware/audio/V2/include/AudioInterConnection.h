#ifndef _AUDIO_INTER_CONNECTION_H
#define _AUDIO_INTER_CONNECTION_H

#include "AudioDef.h"
#include "AudioType.h"
#include "AudioAfeReg.h"
#include "AudioDigitalType.h"

namespace android
{

//!  A AudioInterConnection implementation class
/*!
this class is hold only the operation of digital connection registers
other complicated should move to higher layer.
*/

class AudioInterConnection
{
    public:

        /**
        * AudioInterConnection constructor.
        * setting a regiseter depend on input and output
        */
        AudioInterConnection();

        /**
        * AudioInterConnection destructor.
        */
        ~AudioInterConnection();

        /**
        * setting connection base on input and output
        * @param ConnectionState
        * @see AudioDigitalType::InterConnectionState
        * @param Input
        * @see AudioDigitalType::InterConnectionInput
        * @param Output
        * @see AudioDigitalType::InterConnectionOutput
        * @return status
        */
        status_t SetinputConnection(uint32 ConnectionState, uint32 Input , uint32 Output);


        /**
        * dump all connection state
        */
        void dump();

    private:
        /**
        * check bits can be connected
        * @param RegAddr
        * @param ConnectBits
        * @return status
        */
        bool CheckBitsandReg(short RegAddr, char ConnectBits);

        /**
        * AudioAfeReg instance , to set afe register
        */
        AudioAfeReg *mAduioAfeInstanse;
};

}

#endif
